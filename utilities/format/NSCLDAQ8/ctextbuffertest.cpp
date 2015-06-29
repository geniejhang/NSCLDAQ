


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
  CPPUNIT_TEST(toRawBuffer_0);
  CPPUNIT_TEST(totalBytes_0);
  CPPUNIT_TEST_SUITE_END();

public:
  ctextbuffertest() : m_header(), m_strings(), m_rawBuf(8192), m_buffer() {}
  void setUp() {

    m_header.nwds = 100;
    m_header.type = DAQ::V8::DATABF;
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

    m_strings = {"the first const char*",
                 "the second const char* ..              and some more",
                 "the first const char* but what about this?",
                 ""};

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
    const char* pText = reinterpret_cast<const char*>(m_rawBuf.getBuffer().data());
    pText += 16*sizeof(std::uint16_t) + sizeof(std::uint16_t);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("First string makes sense",
                                 m_strings.at(0), m_buffer.getStrings().at(0));

  }

  void rawCtor_1 () {
    const char* pText = reinterpret_cast<const char*>(m_rawBuf.getBuffer().data());
    pText += 16*sizeof(std::uint16_t) + sizeof(std::uint16_t);
    pText += m_strings.at(0).size() + 1;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("second string makes sense",
                                 m_strings.at(1), m_buffer.getStrings().at(1));

  }

  void rawCtor_2 () {
    const char* pText = reinterpret_cast<const char*>(m_rawBuf.getBuffer().data());
    pText += 16*sizeof(std::uint16_t) + sizeof(std::uint16_t);
    pText += m_strings.at(0).size() + 1;
    pText += m_strings.at(1).size() + 1;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("third string makes sense",
                                 m_strings.at(2), m_buffer.getStrings().at(2));

  }

  void rawCtor_3 () {
    const char* pText = reinterpret_cast<const char*>(m_rawBuf.getBuffer().data());
    pText += 16*sizeof(std::uint16_t) + sizeof(std::uint16_t);
    pText += m_strings.at(0).size() + 1;
    pText += m_strings.at(1).size() + 1;
    pText += m_strings.at(2).size() + 1;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("fourth string makes sense",
                                 m_strings.at(3), m_buffer.getStrings().at(3));

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


