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


#include <cppunit/extensions/HelperMacros.h>

#include <V12/CRingItem.h>
#include <V12/CRawRingItem.h>
#include <V12/CPhysicsEventItem.h>
#include <V12/CAbnormalEndItem.h>

#include "V12/CAbnormalEndRunFilterHandler.h"

#include <CFileDataSink.h>

#include <stdexcept>

using namespace DAQ;

// A test suite 
class CAbnormalEndRunFilterHandlerTest : public CppUnit::TestFixture
{
  private:
    V12::CFilter* m_filter;
    CDataSink* m_sink;

  public:
    CAbnormalEndRunFilterHandlerTest();

    CPPUNIT_TEST_SUITE( CAbnormalEndRunFilterHandlerTest );
    CPPUNIT_TEST ( testGenericItem );
    CPPUNIT_TEST ( testOtherItem );
    CPPUNIT_TEST_SUITE_END();

  public:
    void setUp();
    void tearDown();

    void testGenericItem();
    void testOtherItem();

};


// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( CAbnormalEndRunFilterHandlerTest );

CAbnormalEndRunFilterHandlerTest::CAbnormalEndRunFilterHandlerTest()
    : m_filter(0)
{}

void CAbnormalEndRunFilterHandlerTest::setUp()
{
  m_sink = new CFileDataSink("test.txt");
  m_filter = new V12::CAbnormalEndRunFilterHandler(*m_sink);
}

void CAbnormalEndRunFilterHandlerTest::tearDown()
{
  // Call the destructor to free
  // owned memory
  delete m_filter;
  m_filter=0;

  delete m_sink; 
  m_sink = 0;
  remove(".test.txt");
}

// Create some generic item type to force testing of handleRingItem()
void CAbnormalEndRunFilterHandlerTest::testGenericItem()
{
  V12::CRingItemPtr item(new V12::CRawRingItem());
  item->setType(1000);

  V12::CRingItemPtr new_item;

  // make sure this works fine for all other items
  CPPUNIT_ASSERT_NO_THROW( new_item = m_filter->handleRingItem(item));
  CPPUNIT_ASSERT( item == new_item );

  // make sure this throws when the item gets sent downstream
  V12::CAbnormalEndItemPtr abnEnd(new V12::CAbnormalEndItem());

  CPPUNIT_ASSERT_THROW( new_item = m_filter->handleAbnormalEndItem(abnEnd),
                        std::runtime_error );
}

void CAbnormalEndRunFilterHandlerTest::testOtherItem()
{
  V12::CRingItemPtr item(new V12::CRawRingItem);
  item->setType(1000);

  V12::CRingItemPtr new_item;

  // make sure this works fine for all other items
  CPPUNIT_ASSERT_NO_THROW( new_item = m_filter->handleRingItem(item));
  CPPUNIT_ASSERT( item == new_item );
}

