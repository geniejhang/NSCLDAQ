


#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>

#include <iostream>
#include <iomanip>
#include <algorithm>

#include <DataFormatV8.h>
#include <CRawBuffer.h>
#include <DebugUtils.h>
#include <ByteBuffer.h>

#define private public
#define protected public
#include <CScalerBuffer.h>
#undef protected
#undef private

using namespace std;

using namespace DAQ::V8;
using namespace DAQ::Buffer;

class scalerbuffertest : public CppUnit::TestFixture {
private:
  bheader m_header;
  std::vector<std::uint32_t> m_scalers;
  CScalerBuffer m_scalerBuffer;
  ByteBuffer m_bytes;
  std::uint32_t m_offsetEnd;
  std::uint32_t m_offsetBegin;


public:
  CPPUNIT_TEST_SUITE(scalerbuffertest);
  CPPUNIT_TEST(rawBufferCtor_0);
  CPPUNIT_TEST(rawBufferCtor_1);
  CPPUNIT_TEST(rawBufferCtor_2);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {

    m_scalers = std::vector<std::uint32_t>({3, 0, 1,
                                           2, 3,
                                           3, 4, 5});
  m_header.nwds = 100;
    m_header.type = DAQ::V8::SCALERBF;
    m_header.cks  = 0;
    m_header.run  = 1;
    m_header.seq  = 2;
    m_header.nevt = m_scalers.size();
    m_header.nlam = 0;
    m_header.cpu  = 0;
    m_header.nbit = 0;
    m_header.buffmt = DAQ::V8::StandardVsn;
    m_header.ssignature = DAQ::V8::BOM16;
    m_header.lsignature = DAQ::V8::BOM32;


    // create the various pieces in the body
    m_offsetEnd = 123;
    m_offsetBegin = 456;
    std::vector<std::uint8_t>  empty(6); // empty space

    // fill a byte buffer with data
    m_bytes << m_offsetEnd;
    m_bytes << empty;
    m_bytes << m_offsetBegin;
    m_bytes << empty;
    m_bytes << m_scalers;

    m_scalerBuffer = CScalerBuffer(m_header, 123, 456, m_scalers);

  }

  void tearDown()
  {
  }

void rawBufferCtor_0() {

  CRawBuffer rawBuf(8192);

  DAQ::Buffer::ByteBuffer buffer;
  buffer << m_header;
  buffer << m_bytes;
  rawBuf.setBuffer(buffer);

  CScalerBuffer sclrBuf(rawBuf);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Scalers are correct when constructed from raw buffer",
                               m_scalers, sclrBuf.getScalers());
}


void rawBufferCtor_1() {

  CRawBuffer rawBuf(8192);

  DAQ::Buffer::ByteBuffer buffer;
  buffer << m_header;
  buffer << m_bytes;
  rawBuf.setBuffer(buffer);

  CScalerBuffer sclrBuf(rawBuf);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Offset end is correct when constructed from raw buffer",
                               m_offsetEnd, sclrBuf.getOffsetEnd());
}

void rawBufferCtor_2() {

  CRawBuffer rawBuf(8192);

  DAQ::Buffer::ByteBuffer buffer;
  buffer << m_header;
  buffer << m_bytes;
  rawBuf.setBuffer(buffer);

  CScalerBuffer sclrBuf(rawBuf);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Offset begin is correct when constructed from raw buffer",
                               m_offsetBegin, sclrBuf.getOffsetBegin());
}

};

CPPUNIT_TEST_SUITE_REGISTRATION(scalerbuffertest);



