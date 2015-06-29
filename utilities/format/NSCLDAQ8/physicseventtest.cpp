


#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>

#include <iostream>
#include <iomanip>
#include <algorithm>

#include <DataFormatV8.h>
#include <CRawBuffer.h>
#include <ByteBuffer.h>
#include <DebugUtils.h>

#define private public
#define protected public
#include <CPhysicsEventBuffer.h>
#undef protected
#undef private

using namespace std;

using namespace DAQ::V8;
using namespace DAQ::Buffer;

class physicseventtest : public CppUnit::TestFixture {
private:
  bheader m_header;
  std::vector<uint16_t> m_bodyData;
  CPhysicsEventBuffer m_physicsBuffer;

public:
  CPPUNIT_TEST_SUITE(physicseventtest);
  CPPUNIT_TEST(totalShorts_0);
  CPPUNIT_TEST(ctor_0);
  CPPUNIT_TEST(copyCtor_0);
  CPPUNIT_TEST(copyCtor_1);
  CPPUNIT_TEST(copyCtor_2);
  CPPUNIT_TEST(unsupportedVersion_0);
  CPPUNIT_TEST(rawBufferCtor_0);
  CPPUNIT_TEST(rawBufferCtor_1);
  CPPUNIT_TEST(rawBufferCtor_2);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {

    m_header.nwds = 100;
    m_header.type = DAQ::V8::DATABF;
    m_header.cks  = 0;
    m_header.run  = 1;
    m_header.seq  = 2;
    m_header.nevt = 2;
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

void totalShorts_0() {
  std::vector<uint16_t> data = {2, 3};
  ByteBuffer buffer;
  buffer << data;

  const CPhysicsEvent& event = CPhysicsEvent(buffer, false);

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Total size for standard buffer makes sense",
                               std::size_t(2), event.getNTotalShorts());

}

void ctor_0() {

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Buffer should be parsed nevt number of times",
                               std::size_t(2), m_physicsBuffer.size());

}

void copyCtor_0 () {
  CPhysicsEventBuffer buffer(m_physicsBuffer);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of events in body is preserved during copy",
                               buffer.size(), m_physicsBuffer.size());
}
void copyCtor_1 () {
  CPhysicsEventBuffer buffer(m_physicsBuffer);
  CPPUNIT_ASSERT_MESSAGE("Data of first event is preserved during copy",
                               std::equal(buffer.at(0)->begin(), buffer.at(0)->end(),
                                          m_physicsBuffer.at(0)->begin()));
}
void copyCtor_2 () {
  CPhysicsEventBuffer buffer(m_physicsBuffer);
  CPPUNIT_ASSERT_MESSAGE("Data of second event is preserved during copy",
                               std::equal(buffer.at(1)->begin(), buffer.at(1)->end(),
                                          m_physicsBuffer.at(1)->begin()));
}

void unsupportedVersion_0() {
  m_header.buffmt = DAQ::V8::JumboVsn;

  CPPUNIT_ASSERT_THROW_MESSAGE("Parsing jumbo buffers is a failure",
                               m_physicsBuffer = CPhysicsEventBuffer(m_header, m_bodyData),
                               std::runtime_error);

}

void rawBufferCtor_0() {

  CRawBuffer rawBuf(8192);

  DAQ::Buffer::ByteBuffer buffer;
  buffer << m_header;
  buffer << m_bodyData;

  rawBuf.setBuffer(buffer);

  CPhysicsEventBuffer physBuf(rawBuf);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Construct from a raw buffer",
                               std::size_t(2), physBuf.size());
}


void rawBufferCtor_1() {

  CRawBuffer rawBuf(8192);

  DAQ::Buffer::ByteBuffer buffer;
  buffer << m_header;
  buffer << m_bodyData;

  rawBuf.setBuffer(buffer);


  // because we are building our raw buffer from the same data that m_physicsBuffer was
  // constructed from, the contents better be the same.
  CPhysicsEventBuffer physBuf(rawBuf);

  CPPUNIT_ASSERT_MESSAGE("RawBuffer ctor has proper 1st event",
                         std::equal(physBuf.at(0)->begin(), physBuf.at(0)->end(),
                                    m_physicsBuffer.at(0)->begin()));
}

void rawBufferCtor_2() {

  CRawBuffer rawBuf(8192);

  DAQ::Buffer::ByteBuffer buffer;
  buffer << m_header;
  buffer << m_bodyData;

  rawBuf.setBuffer(buffer);

  // because we are building our raw buffer from the same data that m_physicsBuffer was
  // constructed from, the contents better be the same.
  CPhysicsEventBuffer physBuf(rawBuf);
  CPPUNIT_ASSERT_MESSAGE("RawBuffer ctor has proper 2nd event",
                         std::equal(physBuf.at(1)->begin(), physBuf.at(1)->end(),
                                    m_physicsBuffer.at(1)->begin()));
}

};

CPPUNIT_TEST_SUITE_REGISTRATION(physicseventtest);


