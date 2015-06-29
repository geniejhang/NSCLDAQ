


#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>

#include <iostream>
#include <iomanip>
#include <algorithm>

#include <DataFormatV8.h>
#include <CRawBuffer.h>
#include <CScalerBuffer.h>
#include <CPhysicsEventBuffer.h>
#include <DebugUtils.h>

#define private public
#define protected public
#include <format_cast.h>
#undef protected
#undef private

using namespace std;

using namespace DAQ::V8;
using namespace DAQ::Buffer;

class format_casttest : public CppUnit::TestFixture {
private:
  bheader m_header;
  std::vector<uint16_t> m_bodyData;
  CPhysicsEventBuffer m_physicsBuffer;

public:
  CPPUNIT_TEST_SUITE(format_casttest);
  CPPUNIT_TEST( castRawToPhysics_0 );
  CPPUNIT_TEST( castRawToScaler_0 );
  CPPUNIT_TEST_SUITE_END();

public:
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

    m_bodyData = std::vector<uint16_t>({3, 0, 1,
                                       2, 3,
                                       3, 4, 5});
    m_physicsBuffer = CPhysicsEventBuffer(m_header, m_bodyData);

  }

  void tearDown()
  {
  }


void castRawToPhysics_0 () {

  CRawBuffer rawBuf(8192);

  DAQ::Buffer::ByteBuffer buffer;
  buffer << m_header;
  buffer << m_bodyData;
  rawBuf.setBuffer(buffer);

  // because we are building our raw buffer from the same data that m_physicsBuffer was
  // constructed from, the contents better be the same.
  auto  physBuf = format_cast<CPhysicsEventBuffer>(rawBuf);
  CPPUNIT_ASSERT_MESSAGE("RawBuffer ctor has proper 2nd event",
                         std::equal(physBuf.at(1)->begin(), physBuf.at(1)->end(),
                                    m_physicsBuffer.at(1)->begin()));
}

void castRawToScaler_0 () {
  std::vector<std::uint32_t> sclrs = {0, 1, 2, 3};
  m_header.nevt = sclrs.size();

  CRawBuffer rawBuf(8192);
  DAQ::Buffer::ByteBuffer buffer;
  buffer << m_header;
  buffer << uint32_t(1234);
  buffer << uint16_t(0);
  buffer << uint16_t(0);
  buffer << uint16_t(0);

  buffer << uint32_t(5678);
  buffer << uint16_t(0);
  buffer << uint16_t(0);
  buffer << uint16_t(0);

  buffer << sclrs;

  rawBuf.setBuffer(buffer);

  // because we are building our raw buffer from the same data that m_physicsBuffer was
  // constructed from, the contents better be the same.
  auto  sclrBuf = format_cast<CScalerBuffer>(rawBuf);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("RawBuffer ctor has proper 2nd event",
                         uint32_t(1234), sclrBuf.getOffsetEnd());
  CPPUNIT_ASSERT_EQUAL_MESSAGE("RawBuffer ctor has proper 2nd event",
                         uint32_t(5678), sclrBuf.getOffsetBegin());
  CPPUNIT_ASSERT_EQUAL_MESSAGE("RawBuffer ctor has proper 2nd event",
                         sclrs, sclrBuf.getScalers());
}

};

CPPUNIT_TEST_SUITE_REGISTRATION(format_casttest);

