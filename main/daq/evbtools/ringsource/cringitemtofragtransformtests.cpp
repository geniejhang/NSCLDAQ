/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


static const char* Copyright = "(C) Copyright Michigan State University 2014, All rights reserved";


#include <vector>

#include <cppunit/extensions/HelperMacros.h>

#include <V12/CRawRingItem.h>
#include <V12/Serialize.h>
#include <ByteBuffer.h>

#define private public
#define protected public
#include "CRingItemToFragmentTransform.h"
#undef private
#undef protected

#include <array>
#include <vector>
#include <limits>

using namespace DAQ;


// A test suite 
class CRingItemToFragmentTransformTest : public CppUnit::TestFixture
{

  private:
    CRingItemToFragmentTransform *m_pTransform;

  public:

    CPPUNIT_TEST_SUITE( CRingItemToFragmentTransformTest );
    CPPUNIT_TEST ( validateIds_0 );
    CPPUNIT_TEST ( validateIds_1 );
    CPPUNIT_TEST ( transform_0 );
    CPPUNIT_TEST ( transform_1 );
    CPPUNIT_TEST ( transform_2 );
    CPPUNIT_TEST_SUITE_END();

  public:
    void setUp();
    void tearDown();

    void validateIds_0();
    void validateIds_1();
    void transform_0();
    void transform_1();
    void transform_2();

  private:

    std::vector<uint8_t> body();
};


// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( CRingItemToFragmentTransformTest );

void CRingItemToFragmentTransformTest::setUp()
{
  int argc=1;
  const char* argv[] = {"unittests"};
  m_pTransform = new CRingItemToFragmentTransform();
  m_pTransform->setAllowedSourceIds({0,1,2});
}

void CRingItemToFragmentTransformTest::tearDown()
{
  delete m_pTransform;
}


// Test that ring item with body header and source id that 
// is unaccepted causes failure
void CRingItemToFragmentTransformTest::validateIds_0 () {
   
  // create ring item with tstamp 0x123456 and source id = 3
  V12::CRawRingItem item(V12::PHYSICS_EVENT, 0x123456, 3);

  CPPUNIT_ASSERT_THROW_MESSAGE(
      "Bad source ids should cause a thrown exception",
      (*m_pTransform)(item),
      std::runtime_error);
}


// Test that ring item with body header and accepted source id 
// succeeds
void CRingItemToFragmentTransformTest::validateIds_1 () {
   
  // create ring item with tstamp 0x123456 and source id = 3
  V12::CRawRingItem item(V12::PHYSICS_EVENT, 0x123456, 0);

  CPPUNIT_ASSERT_NO_THROW_MESSAGE(
      "Valid source ids should NOT cause a thrown exception",
      (*m_pTransform)(item));
}


// Test that a ring item with body header generates a fragment
// with all of the data taken from the body header
void CRingItemToFragmentTransformTest::transform_0() 
{
  uint64_t timestamp = 0x123456;
  uint32_t sourceid  = 0;
  V12::CRawRingItem item(V12::PHYSICS_EVENT, timestamp, sourceid, body());

  ClientEventFragment frag = (*m_pTransform)(item);

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Timestamp is correct",
      item.getEventTimestamp(),
      frag.s_timestamp);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Source id is correct",
      item.getSourceId(),
      frag.s_sourceId);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Size is correct",
      item.size(),
      frag.s_size);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Barrier is correct",
      uint32_t(0),
      frag.s_barrierType);

  Buffer::ByteBuffer rawItem = V12::serializeItem(item);
  CPPUNIT_ASSERT_MESSAGE("Payload is correct",
      std::equal(rawItem.begin(), rawItem.end(), reinterpret_cast<uint8_t*>(frag.s_payload)));
}

// begin run gets a nonzero barrier type and all the normal functionality
void CRingItemToFragmentTransformTest::transform_1()
{
  uint64_t timestamp = 0x123456;
  uint32_t sourceid  = 0;
  V12::CRawRingItem item(V12::BEGIN_RUN, timestamp, sourceid);

  ClientEventFragment frag = (*m_pTransform)(item);

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Timestamp is correct",
      item.getEventTimestamp(),
      frag.s_timestamp);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Source id is correct",
      item.getSourceId(),
      frag.s_sourceId);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Size is correct",
      item.size(),
      frag.s_size);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Barrier is correct",
      V12::BEGIN_RUN,
      frag.s_barrierType);

  std::vector<uint8_t> rawItem = V12::serializeItem(item);
  CPPUNIT_ASSERT_MESSAGE("Payload is correct",
      std::equal(rawItem.begin(), rawItem.end(),
                 reinterpret_cast<uint8_t*>(frag.s_payload)));

}

// end run gets a nonzero barrier type
void CRingItemToFragmentTransformTest::transform_2()
{
  uint64_t timestamp = 0x123456;
  uint32_t sourceid  = 0;
  V12::CRawRingItem item(V12::END_RUN, timestamp, sourceid);

  ClientEventFragment frag = (*m_pTransform)(item);

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Barrier is correct",
      V12::END_RUN,
      frag.s_barrierType);

}

std::vector<uint8_t> CRingItemToFragmentTransformTest::body()
{

  std::vector<uint8_t> data = {0, 1, 2, 3, 4, 5, 6, 7};
  return data;
}

