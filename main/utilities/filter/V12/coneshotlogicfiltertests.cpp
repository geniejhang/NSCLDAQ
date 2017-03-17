
/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
         NSCL
         Michigan State University
         East Lansing, MI 48824-1321
*/


static const char* Copyright = "(C) Copyright Michigan State University 2017, All rights reserved";


#include <cppunit/extensions/HelperMacros.h>
#include <DebugUtils.h>
#include <Asserts.h>

#include <V12/CRingStateChangeItem.h>
#include <V12/CRingScalerItem.h>

#include <V12/CFilterAbstraction.h>
#include "V12/COneShotLogicFilter.h"

#include "V12/CTestFilter.h"

#include <CFilterMediator.h>
#include <CFilterAbstraction.h>

#include <COneShotException.h>
#include <make_unique.h>

#include <ios>
#include <algorithm>
#include <fstream>
#include <iterator>
#include <string>
#include <fstream>
#include <vector>

using namespace DAQ;

// A test suite
class COneShotLogicFilterTest : public CppUnit::TestFixture
{
  private:
    // Define a test filter to return some testable results

  private:
    CFilterMediatorUPtr    m_pMediator;
    V12::CFilterAbstractionPtr  m_pAbstraction;
    V12::COneShotLogicFilterPtr m_pFilter;

  public:

    CPPUNIT_TEST_SUITE( COneShotLogicFilterTest );
    CPPUNIT_TEST ( testStateChangeItem_0 );
    CPPUNIT_TEST ( testStateChangeItem_1 );
    CPPUNIT_TEST ( testStateChangeItem_2 );
    CPPUNIT_TEST ( testStateChangeItem_3 );
    CPPUNIT_TEST ( testRingItem_0 );
    CPPUNIT_TEST ( testRingItem_1 );
    CPPUNIT_TEST_SUITE_END();

  public:
    void setUp();
    void tearDown();

    void testStateChangeItem_0();
    void testStateChangeItem_1();
    void testStateChangeItem_2();
    void testStateChangeItem_3();
    void testRingItem_0();
    void testRingItem_1();

};


// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( COneShotLogicFilterTest );

void COneShotLogicFilterTest::setUp()
{
    m_pMediator = DAQ::make_unique<CFilterMediator>();

    m_pAbstraction = std::make_shared<V12::CFilterAbstraction>();
    m_pAbstraction->setFilterMediator(*m_pMediator);

    m_pFilter = std::make_shared<V12::COneShotLogicFilter>(1, *m_pAbstraction);
}

void COneShotLogicFilterTest::tearDown()
{
}


void COneShotLogicFilterTest::testStateChangeItem_0()
{
  auto pItem = std::make_shared<V12::CRingStateChangeItem>(V12::BEGIN_RUN);

  auto pNewItem = m_pFilter->handleStateChangeItem(pItem);

  CPPUNIT_ASSERT( pNewItem == pItem );

  ASSERTMSG("after first begin, no longer waiting",
                 !m_pFilter->getOneShotLogic().waitingForBegin());

}

void COneShotLogicFilterTest::testStateChangeItem_1()
{
  auto pItem = std::make_shared<V12::CRingStateChangeItem>(V12::END_RUN);

  auto pNewItem = m_pFilter->handleStateChangeItem(pItem);

  CPPUNIT_ASSERT( pNewItem == nullptr );

  ASSERTMSG("before first begin, wait for it",
                 m_pFilter->getOneShotLogic().waitingForBegin());

}

void COneShotLogicFilterTest::testStateChangeItem_2()
{
  // a proper number of begin and end runs sets the state to complete,
  // aborts the mediator, and still passes on all of the data
  auto pBegin = std::make_shared<V12::CRingStateChangeItem>(V12::BEGIN_RUN);
  auto pEnd   = std::make_shared<V12::CRingStateChangeItem>(V12::END_RUN);

  auto pNewBeg = m_pFilter->handleStateChangeItem(pBegin);
  auto pNewEnd = m_pFilter->handleStateChangeItem(pEnd);

  CPPUNIT_ASSERT( pBegin == pNewBeg );
  CPPUNIT_ASSERT( pEnd == pNewEnd );

  EQMSG("mediator is set to abort",
        true,
        m_pMediator->getAbort());

}

void COneShotLogicFilterTest::testStateChangeItem_3()
{
  auto pBegin1 = std::make_shared<V12::CRingStateChangeItem>(V12::BEGIN_RUN);
  pBegin1->setRunNumber(120);
  auto pBegin2 = std::make_shared<V12::CRingStateChangeItem>(V12::BEGIN_RUN);
  pBegin2->setRunNumber(121);

  auto pNewItem = m_pFilter->handleStateChangeItem(pBegin1);

  CPPUNIT_ASSERT_THROW_MESSAGE("changing run number is an error",
                               m_pFilter->handleStateChangeItem(pBegin2),
                               CException);

}


void COneShotLogicFilterTest::testRingItem_0()
{
  auto item = std::make_shared<V12::CRingScalerItem>(300);
  auto new_item = m_pFilter->handleRingItem(item);

  ASSERTMSG( "before begin, nullptr returned",
             nullptr == new_item );

}

void COneShotLogicFilterTest::testRingItem_1()
{
    auto pBegin = std::make_shared<V12::CRingStateChangeItem>(V12::BEGIN_RUN);
    auto pItem = std::make_shared<V12::CRingScalerItem>(300);

    m_pFilter->handleStateChangeItem(pBegin);

    auto pNewItem = m_pFilter->handleRingItem(pItem);

  ASSERTMSG( "after begin, filter behaves as a transparent filter",
           pItem == pNewItem );

}

