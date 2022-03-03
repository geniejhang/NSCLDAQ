/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  stdintoringtests.cpp
 *  @brief:  Testsuite for stdintoring.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CRingBuffer.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>

#include <time.h>
#include <string.h>

#include <stdint.h>
#include <io.h>

static const char* ringname = "stdintoringtest";
static const char* command[] = {
    "./stdintoring",  ringname, nullptr
};

typedef struct _Header {
    uint32_t s_size;
    uint32_t s_type;
} Header, *pHeader;

class stdin2ringtest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(stdin2ringtest);
    CPPUNIT_TEST(test_1);
    CPPUNIT_TEST_SUITE_END();
    
private:
    pid_t  m_stdintoring;
    int m_pipe[2];                // stdinto ring read from 0 we write to 1.
    CRingBuffer*  m_pSource;      // Where the data gets dropped by stdintoring.
public:
    void setUp() {
        CRingBuffer::create(ringname);
        m_pSource  = new CRingBuffer(ringname);
        startStdinToRing();
                
    }
    void tearDown() {
        int status;
        close(m_pipe[1]);
        waitpid(m_stdintoring, &status, 0);
        delete m_pSource;               // Disconnect from the ring bufer.
        CRingBuffer::remove(ringname);
    }
protected:
    void test_1();
private:
    void startStdinToRing();
};

void
stdin2ringtest::startStdinToRing()
{
    EQ(0, pipe2(m_pipe, O_DIRECT ));
    m_stdintoring = fork();
    if (m_stdintoring > 0) {
        close(m_pipe[0]);     // will be stdin for stdintoring.
    } else if (m_stdintoring == 0) {
        // Jiggle file descriptor around so m_pipe[0] is stdin:
        
        close(STDIN_FILENO);
        if (dup2(m_pipe[0], STDIN_FILENO)) {
            perror("Child could not reset stdin");
            exit(EXIT_FAILURE);
        }
        // Exec glom:
        
        execv(command[0], const_cast<char**>(command));
        perror("Could not start stdintoring");
        
        
    } else {
        // failed
        
        ASSERT(0);
    }
    
}

CPPUNIT_TEST_SUITE_REGISTRATION(stdin2ringtest);

void stdin2ringtest::test_1()
{
#pragma pack(push, 1)
    struct Item {
        Header s_header;
        uint32_t data[65536/sizeof(uint32_t)];
    } item;
#pragma pack(pop)
    for (int i =0; i < 65536/sizeof(uint32_t); i++) {
        item.data[i] = i;
    }
    item.s_header.s_type = 1;
    item.s_header.s_size = sizeof(item);
    
    io::writeData(m_pipe[1], &item, sizeof(item));
    io::writeData(m_pipe[1], &item, sizeof(item));
    
}