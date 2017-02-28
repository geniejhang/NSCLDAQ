

/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
       Jeromy Tompkins
       NSCL
       Michigan State University
       East Lansing, MI 48824-1321
*/

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "DebugUtils.h"

#include "CFileDataSource.h"
#include "ByteBuffer.h"

#include <fstream>
#include <string>
#include <algorithm>

#include <unistd.h>

using namespace std;
using namespace DAQ;


pid_t childpid;

// A test suite
class CFileDataSourceTest : public CppUnit::TestFixture
{
    private:
    std::string m_testPath;

    public:
    CPPUNIT_TEST_SUITE( CFileDataSourceTest );
    CPPUNIT_TEST(peek_0);
    CPPUNIT_TEST(peek_1);
    CPPUNIT_TEST(peekRead_0);
    CPPUNIT_TEST(availableData_0);
    CPPUNIT_TEST(availableData_1);
    CPPUNIT_TEST(availableData_2);
    CPPUNIT_TEST(ignore_0);
    CPPUNIT_TEST(ignore_1);
    CPPUNIT_TEST_SUITE_END();

private:
    Buffer::ByteBuffer m_testData;

    public:
    void setUp() {

        m_testData << uint32_t(24);
        m_testData << uint32_t(30);
        m_testData << uint64_t(0x123456776);
        m_testData << uint32_t(1);
        m_testData << uint32_t(123);

        m_testPath = "___---test---___.evt";
        std::ofstream tempfile(m_testPath.c_str());
        tempfile.write(reinterpret_cast<char*>(m_testData.data()), m_testData.size());
    }

    void tearDown() {
        std::remove(m_testPath.c_str());
    }

    int spawn() {

        int pipes[2];
        if (pipe(pipes) == -1) {
            perror("Could not open pipes");
            throw "could not open pipes";
        }

        // I can never keep these straight so:

        int readPipe = pipes[0];
        int writePipe= pipes[1];

        if ((childpid = fork()) != 0) {
            // parent...will read from the pipe:

            sleep(1);			// Let child start.
            close(writePipe);
            return readPipe;

        }
        else {
            // child...
            setsid();

            close(readPipe);


            // Set the write pipe as our stdout/stderr:

            dup2(writePipe, STDOUT_FILENO);
            close(writePipe);

            write(STDOUT_FILENO, m_testData.data(), m_testData.size());

            close(STDOUT_FILENO);

            exit(0);

        }
    }


    void peek_0() {
        char data[4];

        CFileDataSource file(m_testPath);
        size_t nRead = file.peek(data, sizeof(data));

        size_t pos = file.tell();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("peek returns expected number of bytes",
                               size_t(4), nRead);

        char expected[] = {24, 0, 0, 0};
        CPPUNIT_ASSERT_MESSAGE("peek returns expected data",
                               equal(expected, expected+sizeof(expected), data));

        CPPUNIT_ASSERT_EQUAL_MESSAGE("peek does not move the file position",
                                     size_t(0), pos);

    }

    void peek_1() {
        std::array<char,4> data, data1;

        CFileDataSource file(spawn());

        size_t nRead = file.peek(data.data(), data.size());
        size_t nRead1 = file.peek(data1.data(), data1.size());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("first peek returns expected number of bytes",
                               size_t(4), nRead);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("second peek returns expected number of bytes",
                               size_t(4), nRead1);

        char expected[] = {24, 0, 0, 0};
        CPPUNIT_ASSERT_MESSAGE("first peek returns expected data",
                               equal(expected, expected+sizeof(expected), data.begin()));
        CPPUNIT_ASSERT_MESSAGE("second peek returns expected data",
                               equal(expected, expected+sizeof(expected), data1.begin()));
    }


    // test that if we request more data than previously requested in a peek,
    // then we get all that was in the peek buffer and more
    void peekRead_0() {
        std::array<char,20> data;
        std::array<char,24> item;

        CFileDataSource file(spawn());

        size_t nRead = file.peek(data.data(), data.size());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("first peek returns expected number of bytes",
                               size_t(20), nRead);

        file.read(item.data(), item.size());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("no error flags were set during last read",
                                     false, file.eof());

        CPPUNIT_ASSERT_MESSAGE("first peek returns expected data",
                               equal(m_testData.begin(), m_testData.begin()+20, data.begin()));
        CPPUNIT_ASSERT_MESSAGE("Read after peek returns all data",
                               equal(m_testData.begin(), m_testData.end(), item.begin()));

    }

    // test that if we request less data than previously requested in a peek,
    // then we get all that was in the peek buffer and then get the remainder in subsequent
    // peeks
    void peekRead_1() {
        std::array<char,20> data;
        std::array<char,16> item;

        CFileDataSource file(spawn());

        size_t nRead = file.peek(data.data(), data.size());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("first peek returns expected number of bytes",
                                     data.size(), nRead);

        file.read(item.data(), item.size());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("read after peek returns expected number of bytes",
                                     item.size(), nRead);

        CPPUNIT_ASSERT_MESSAGE("first peek returns expected data",
                               equal(m_testData.begin(), m_testData.begin()+20, data.begin()) );
        CPPUNIT_ASSERT_MESSAGE("Read after peek returns all data",
                               equal(m_testData.begin(), m_testData.end(), item.begin()) );

        nRead = file.peek(data.begin(), 4);
        CPPUNIT_ASSERT_MESSAGE("peek after read gets rest of peek buffer",
                               equal( item.begin()+16, item.begin()+20, data.begin()) );

    }


    void availableData_0() {
        char data[4];

        CFileDataSource file(m_testPath);
        size_t nBytes = file.availableData();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("available data should be entire file prior to any reads",
                                     size_t(24), nBytes);

    }

    void availableData_1() {
        char data[4];

        CFileDataSource file(m_testPath);
        file.read(data, sizeof(data));
        size_t nBytes = file.availableData();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("available data should be entire file prior to any reads",
                                     size_t(20), nBytes);

    }

    void availableData_2() {
        char data[4];

        CFileDataSource file(STDIN_FILENO);
        size_t nBytes = file.availableData();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("available data for stdin should be essentially infinite",
                                     std::numeric_limits<size_t>::max(), nBytes);

    }

    void ignore_0() {

        CFileDataSource file(m_testPath);
        file.ignore(10);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("offset should change after ignore",
                                     size_t(10), file.tell());

    }

    void ignore_1() {
        union {
            uint32_t value;
            char     bytes[sizeof(uint32_t)];
        } hybrid;

        CFileDataSource file(m_testPath);
        file.ignore(20);

        file.read(hybrid.bytes, sizeof(uint32_t));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("offset should change after ignore",
                                     uint32_t(123), hybrid.value);

    }


};



// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( CFileDataSourceTest );
