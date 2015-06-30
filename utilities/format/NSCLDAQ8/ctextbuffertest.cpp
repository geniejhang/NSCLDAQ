


#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>

#include <DataFormatV8.h>
#include <CRawBuffer.h>
#include <DebugUtils.h>

#define private public
#define protected public
#include <CTextBuffer.h>
#undef protected
#undef private

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <iostream>
#include <string>
#include <stdexcept>

using namespace std;

using namespace DAQ::V8;
using namespace DAQ::Buffer;

class ctextbuffertest : public CppUnit::TestFixture {
private:
  bheader m_header;
  std::vector<std::string> m_strings;
  CRawBuffer m_rawBuf;
  CTextBuffer m_buffer;

public:
  CPPUNIT_TEST_SUITE(ctextbuffertest);
  CPPUNIT_TEST(rawCtor_0);
  CPPUNIT_TEST(rawCtor_1);
  CPPUNIT_TEST(rawCtor_2);
  CPPUNIT_TEST(rawCtor_3);
  CPPUNIT_TEST(rawCtor_4);
  CPPUNIT_TEST(toRawBuffer_0);
  CPPUNIT_TEST(totalBytes_0);
  CPPUNIT_TEST_SUITE_END();

public:
  ctextbuffertest() : m_header(), m_strings(), m_rawBuf(8192), m_buffer() {}
  void setUp() {

    m_header.nwds = 100;
    m_header.type = DAQ::V8::STATEVARBF;
    m_header.cks  = 0;
    m_header.run  = 1;
    m_header.seq  = 2;
    m_header.nevt = 3;
    m_header.nlam = 0;
    m_header.cpu  = 0;
    m_header.nbit = 0;
    m_header.buffmt = DAQ::V8::StandardVsn;
    m_header.ssignature = DAQ::V8::BOM16;
    m_header.lsignature = DAQ::V8::BOM32;

    // create some strings
    m_strings = {"the first const char*",
                 "the second const char* ..              and some more",
                 "the first const char* but what about this?",
                 ""};

    // compute the total number of bytes composed by strings
    std::uint16_t totalBytes = sizeof(std::uint16_t);
    for (auto& element : m_strings) {
      totalBytes += element.size() + 1;
    }

    ByteBuffer buffer;
    buffer << m_header;
    buffer << totalBytes;
    buffer << m_strings.at(0).c_str();
    buffer << m_strings.at(1).c_str();
    buffer << m_strings.at(2).c_str();
    buffer << m_strings.at(3).c_str();

    m_rawBuf.setBuffer(buffer);

    m_buffer = CTextBuffer(m_rawBuf);
  }

  void tearDown()
  {
  }

  void rawCtor_0 () {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("First string makes sense",
                                 m_strings.at(0), m_buffer.getStrings().at(0));
  }

  void rawCtor_1 () {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("second string makes sense",
                                 m_strings.at(1), m_buffer.getStrings().at(1));
  }

  void rawCtor_2 () {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("third string makes sense",
                                 m_strings.at(2), m_buffer.getStrings().at(2));

  }

  void rawCtor_3 () {

    CPPUNIT_ASSERT_EQUAL_MESSAGE("fourth string makes sense",
                                 m_strings.at(3), m_buffer.getStrings().at(3));

  }

  void rawCtor_4 () {
    CRawBuffer buffer;

    // create a buffer and fill it with a buffer header
    DAQ::Buffer::ByteBuffer buf;
    bheader headr; headr.type = DAQ::V8::DATABF; // this is an unacceptable type...
    buf << headr;

    // it does not matter that we finish this b/c it should throw before looking at any
    // of the data past the header

    buffer.setBuffer(buf);

    CPPUNIT_ASSERT_THROW_MESSAGE(
          "Raw ctor with non text type causes a thrown exception",
          CTextBuffer text(buffer),
          std::runtime_error
          );
  }

  void toRawBuffer_0 () {
    CRawBuffer rawBuf(8192);
    m_buffer.toRawBuffer(rawBuf);

    // Because we have create m_buffer using the m_rawBuf, we sure better find that
    // we get an identical buffer back using toRawBuffer
    CPPUNIT_ASSERT_EQUAL_MESSAGE("toRawBuffer should fill the right stuff",
                                 m_rawBuf.getBuffer(),
                                 rawBuf.getBuffer());
  }

  void totalBytes_0() {
    CTextBuffer text(bheader(), {"a", "b", "c"});
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Computing total bytes in body is correct",
                                 std::uint16_t(8), text.totalBytes());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(ctextbuffertest);


