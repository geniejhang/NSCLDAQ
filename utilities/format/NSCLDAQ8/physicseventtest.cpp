


#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>



#include <DataFormatV8.h>
#include <CRawBuffer.h>
#include <ByteBuffer.h>
#include <DebugUtils.h>

#define private public
#define protected public
#include <CPhysicsEventBuffer.h>
#undef protected
#undef private

#include <iostream>
#include <iomanip>
#include <algorithm>

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
  CPPUNIT_TEST(rawBufferCtor_3);
  CPPUNIT_TEST(rawBufferCtor_4);
  CPPUNIT_TEST(toRawBuffer_0);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {

    m_header.nwds = 24;
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

void rawBufferCtor_3 () {
  CRawBuffer rawBuf(8192);
  m_header.type = SCALERBF;

  DAQ::Buffer::ByteBuffer buffer;
  buffer << m_header;
  buffer << m_bodyData;

  rawBuf.setBuffer(buffer);

  // because we are building our raw buffer from the same data that m_physicsBuffer was
  // constructed from, the contents better be the same.
  CPPUNIT_ASSERT_THROW_MESSAGE("RawBuffer ctor throws if not of type DATABF",
                               CPhysicsEventBuffer physBuf(rawBuf),
                                std::runtime_error);
}


// construct from a byte swapped buffer produces the correct result
void rawBufferCtor_4() {
  bheader header;
  header.nwds = 0x1800;
  header.type = 0x0100;
  header.nevt = 0x0200;
  header.buffmt = 0x0500;
  header.ssignature = 0x0201;
  header.lsignature = 0x04030201;
  header.cks = 0;
  header.cpu = 0;
  header.nbit = 0;
  header.nlam = 0;
  header.run = 0x0100;
  header.seq = 0x02000000;
  header.unused[0] = 0;
  header.unused[1] = 0;

  std::vector<std::uint16_t> data({0x0300, 0x0000, 0x0100,
                                   0x0200, 0x0300,
                                   0x0300, 0x0400, 0x0500});


  CRawBuffer rawBuf(8192);
  DAQ::Buffer::ByteBuffer buffer;
  buffer << header;
  buffer << data;
  rawBuf.setBuffer(buffer);

  CPhysicsEventBuffer physBuf(rawBuf);

  CPPUNIT_ASSERT_EQUAL_MESSAGE("raw ctor swaps header properly",
                         m_header,  physBuf.getHeader());
  CPPUNIT_ASSERT_EQUAL_MESSAGE("raw ctor specifies properly whether data needs a swap",
                               true, physBuf.at(0)->dataNeedsSwap());
  CPPUNIT_ASSERT_EQUAL_MESSAGE("raw ctor specifies properly whether data needs a swap",
                               true, physBuf.at(1)->dataNeedsSwap());
  }

  // construct from a byte swapped buffer produces the correct result
  void toRawBuffer_0() {
    bheader expectedHeader;
    expectedHeader.nwds = 0x1800;
    expectedHeader.type = 0x0100;
    expectedHeader.nevt = 0x0200;
    expectedHeader.buffmt = 0x0500;
    expectedHeader.ssignature = 0x0201;
    expectedHeader.lsignature = 0x04030201;
    expectedHeader.cks = 0;
    expectedHeader.cpu = 0;
    expectedHeader.nbit = 0;
    expectedHeader.nlam = 0;
    expectedHeader.run = 0x0100;
    expectedHeader.seq = 0x02000000;
    expectedHeader.unused[0] = 0;
    expectedHeader.unused[1] = 0;

    std::vector<std::uint16_t> data({0x0300, 0x0000, 0x0100,
                                     0x0200, 0x0300});


    CRawBuffer rawBuf(8192);
    DAQ::Buffer::ByteBuffer origBuffer;
    origBuffer << expectedHeader;
    origBuffer << data;
    rawBuf.setBuffer(origBuffer);

    CPhysicsEventBuffer physBuf(rawBuf);

    CRawBuffer newBuf(8192);
    physBuf.toRawBuffer(newBuf);

    origBuffer.resize(8192); // make sure that the origBuffer is a full buffer size, b/c
                             // CPhysicsEventBuffer::toRawBuffer will make sure that
                             // the raw buffer is the appropriate size.

    CPPUNIT_ASSERT_EQUAL_MESSAGE("physics buffer is returned to raw buffer unswapped",
                                 origBuffer, newBuf.getBuffer());
    }
};


CPPUNIT_TEST_SUITE_REGISTRATION(physicseventtest);


