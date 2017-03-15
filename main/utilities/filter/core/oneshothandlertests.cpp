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
#include <algorithm>
#include <COneShotException.h>
#include <stdint.h>
#include <limits>

#define private public
#define protected public
#include "COneShotHandler.h"
#undef private
#undef protected 

using namespace DAQ;

// A test suite 
class COneShotHandlerTest : public CppUnit::TestFixture
{
  public:
    COneShotHandlerTest();

    CPPUNIT_TEST_SUITE( COneShotHandlerTest );
    CPPUNIT_TEST ( testConstructor );
    CPPUNIT_TEST ( testWaitForBegin );
    CPPUNIT_TEST ( testCount );
    CPPUNIT_TEST ( testSkipUntilBegin );
    CPPUNIT_TEST ( testThrowOnExtraStateChange );
    CPPUNIT_TEST ( testThrowOnRunNoChange );
    CPPUNIT_TEST ( testBecomesComplete );
    CPPUNIT_TEST ( testTooManyBegins );
    CPPUNIT_TEST ( testWaitForBegin_1 );
    CPPUNIT_TEST_SUITE_END();

  public:
    void setUp();
    void tearDown();

    void testConstructor();
    void testWaitForBegin();
    void testCount();
    void testComplete();
    void testSkipUntilBegin();
    void testThrowOnExtraStateChange();
    void testThrowOnRunNoChange();
    void testBecomesComplete();
    void testTooManyBegins();
    void testWaitForBegin_1();
    void testUpdateNull();
};


// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( COneShotHandlerTest );

COneShotHandlerTest::COneShotHandlerTest()
{}

void COneShotHandlerTest::setUp()
{
}

void COneShotHandlerTest::tearDown()
{
}


void COneShotHandlerTest::testConstructor()
{
  int nsources= 1;
  COneShotHandler handler(nsources, 1, 2, {1,2,3,4});

  CPPUNIT_ASSERT_EQUAL(nsources, handler.m_nExpectedSources);
  CPPUNIT_ASSERT_EQUAL(4, (int)handler.m_stateCounts.size());
  CPPUNIT_ASSERT_EQUAL(false, handler.m_complete);
  CPPUNIT_ASSERT_EQUAL(std::numeric_limits<uint32_t>::max(), handler.m_cachedRunNo);

  std::map<uint32_t,uint32_t>::iterator it = handler.m_stateCounts.begin();
  std::map<uint32_t,uint32_t>::iterator itend = handler.m_stateCounts.end();
  
  int i=1;
  while (it != itend) {
    CPPUNIT_ASSERT_EQUAL(i, (int) it->first);
    CPPUNIT_ASSERT_EQUAL(0, (int) it->second);

    ++it;
    ++i;
  } 
  
}

void COneShotHandlerTest::testWaitForBegin()
{
  uint32_t nsources= 2;
  COneShotHandler handler(nsources, 1, 2, {1,2,3,4});
    
  // We should be waiting
  CPPUNIT_ASSERT_EQUAL(true, handler.waitingForBegin());
  
  // Handle an event
  handler.update(1, 40);

  // we should not longer be waiting
  CPPUNIT_ASSERT_EQUAL(false, handler.waitingForBegin());
  

}


void COneShotHandlerTest::testCount()
{
    COneShotHandler handler(0, 1, 2, {1,2,3,4});
  handler.m_stateCounts[1]=1;
  handler.m_stateCounts[2]=2;
  handler.m_stateCounts[3]=3;
  handler.m_stateCounts[4]=4;

  CPPUNIT_ASSERT_EQUAL(1, (int) handler.getCount(1));
  CPPUNIT_ASSERT_EQUAL(2, (int) handler.getCount(2));
  CPPUNIT_ASSERT_EQUAL(3, (int) handler.getCount(3));
  CPPUNIT_ASSERT_EQUAL(4, (int) handler.getCount(4));
}


void COneShotHandlerTest::testComplete()
{
    COneShotHandler handler(1, 1, 2, {1,2,3,4});
  
  handler.update(1, 0);

  CPPUNIT_ASSERT_EQUAL(false,handler.complete());
}

void COneShotHandlerTest::testSkipUntilBegin()
{
    COneShotHandler handler(1, 1, 2, {1,2,3,4});
  
  handler.update(3, 0);

  CPPUNIT_ASSERT_EQUAL(0, (int)handler.getCount(3));
}


void COneShotHandlerTest::testThrowOnExtraStateChange()
{
    COneShotHandler handler(1, 1, 2, {1,2,3,4});
    handler.m_complete = true;
  
  CPPUNIT_ASSERT_THROW(handler.update(3, 0), COneShotException);
  
}

void COneShotHandlerTest::testThrowOnRunNoChange()
{
    COneShotHandler handler(1, 1, 2, {1,2,3,4});
  handler.m_cachedRunNo = 3;
  CPPUNIT_ASSERT_THROW(handler.update(2, 30), COneShotException);

  CPPUNIT_ASSERT_THROW(handler.update(1, 30), COneShotException);

  CPPUNIT_ASSERT_THROW(handler.update(3, 30), COneShotException);

  CPPUNIT_ASSERT_THROW(handler.update(4, 30), COneShotException);
}


void COneShotHandlerTest::testBecomesComplete()
{
    COneShotHandler handler(1, 1, 2, {1,2,3,4});
  // make sure that we are not waitingForBegin
  handler.m_stateCounts[1] = 1;

  CPPUNIT_ASSERT_EQUAL(false, handler.complete());

  handler.update(2, 0);
  CPPUNIT_ASSERT_EQUAL(true, handler.complete());

}

void COneShotHandlerTest::testTooManyBegins()
{
    COneShotHandler handler(1, 1, 2, {1,2,3,4});
  handler.m_stateCounts[1] = 1;
  
  CPPUNIT_ASSERT_THROW(handler.update(1, 0),COneShotException);
}

void COneShotHandlerTest::testWaitForBegin_1()
{
  uint32_t nsources= 2;
  COneShotHandler handler(nsources, 1, 2, {1,2,3,4});
    
  // We should be waiting
  CPPUNIT_ASSERT_EQUAL(true, handler.waitingForBegin());
  
  // Handle an event
  handler.update(12, 40);

  // we should still be waiting
  CPPUNIT_ASSERT_EQUAL(true, handler.waitingForBegin());

  handler.update(1, 40);

  // we should not longer be waiting
  CPPUNIT_ASSERT_EQUAL(false, handler.waitingForBegin());
  

}
