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
#include <DebugUtils.h>

#include <V12/CPhysicsEventItem.h>
#include <V12/CRingStateChangeItem.h>
#include <V12/CRingScalerItem.h>
#include <V12/CRingTextItem.h>
#include <V12/CRingPhysicsEventCountItem.h>

#include "V12/CTransparentFilter.h"
#include "V12/CNullFilter.h"
#include "V12/CTestFilter.h"

#define private public
#define protected public
#include "V12/CCompositeFilter.h"
#undef private
#undef protected 

#include <ios>
#include <algorithm>
#include <fstream>
#include <iterator>
#include <string>
#include <fstream>
#include <vector>

using namespace DAQ::V12;

// A test suite 
class CCompositeFilterTest : public CppUnit::TestFixture
{
  private:
    // Define a test filter to return some testable results

  private:
    CFilterPtr m_pFilter;
    CCompositeFilterPtr m_pCompositeTest;
    CCompositeFilterPtr m_pCompositeTrans;

  public:
    CCompositeFilterTest();

    CPPUNIT_TEST_SUITE( CCompositeFilterTest );
    CPPUNIT_TEST ( testConstructor );

    CPPUNIT_TEST ( testRegisterFilter );
    CPPUNIT_TEST ( testProcessTransparentFilter );

    CPPUNIT_TEST ( testTransparentStateChangeItem );
    CPPUNIT_TEST ( testTestStateChangeItem );
    CPPUNIT_TEST ( testTransparentScalerItem );
    CPPUNIT_TEST ( testTestScalerItem );
    CPPUNIT_TEST ( testTransparentTextItem );
    CPPUNIT_TEST ( testTestTextItem );
    CPPUNIT_TEST ( testTransparentPhysicsEventItem );
    CPPUNIT_TEST ( testTestPhysicsEventItem );
    CPPUNIT_TEST ( testTransparentPhysicsEventCountItem );
    CPPUNIT_TEST ( testTestPhysicsEventCountItem );
    CPPUNIT_TEST ( testTransparentGenericItem );
    CPPUNIT_TEST ( testTestGenericItem );
    CPPUNIT_TEST ( testInitialize0 );
    CPPUNIT_TEST ( testFinalize0 );
    CPPUNIT_TEST ( testAbnormalEndItem);
    CPPUNIT_TEST ( testGlomParameters);
    CPPUNIT_TEST ( testDataFormatItem);
    CPPUNIT_TEST ( testExitsOnNullReturn );

    CPPUNIT_TEST_SUITE_END();

  public:
    void setUp();
    void tearDown();

    void testConstructor();
    void testRegisterFilter();

    void testProcessTransparentFilter();
    void testTransparentStateChangeItem();
    void testTestStateChangeItem();
    void testTestScalerItem();
    void testTransparentScalerItem();
    void testTestTextItem();
    void testTransparentTextItem();
    void testTransparentPhysicsEventItem();
    void testTestPhysicsEventItem();
    void testTransparentPhysicsEventCountItem();
    void testTestPhysicsEventCountItem();
    void testTransparentFragmentItem();
    void testTestFragmentItem();
    void testTransparentGenericItem();
    void testTestGenericItem();
    void testInitialize0();
    void testFinalize0();
    void testExitsOnNullReturn();
    void testAbnormalEndItem();
    void testGlomParameters();
    void testDataFormatItem();

//    void testTransparentMainLoop();

  private:
  template<class T>  CRingItem* setupAndRunFilter(CRingItem* item);
};


// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( CCompositeFilterTest );

CCompositeFilterTest::CCompositeFilterTest()
    : m_pFilter(), m_pCompositeTest(), m_pCompositeTrans()
{}

void CCompositeFilterTest::setUp()
{
  m_pFilter.reset(new CTestFilter);
  m_pCompositeTest.reset(new CCompositeFilter);
  CTestFilterPtr testFilt(new CTestFilter);
  m_pCompositeTest->registerFilter(testFilt);

  CTransparentFilterPtr transFilt(new CTransparentFilter);
  m_pCompositeTrans.reset(new CCompositeFilter);
  m_pCompositeTrans->registerFilter(transFilt);
  
}

void CCompositeFilterTest::tearDown()
{
    m_pFilter.reset();
    m_pCompositeTest.reset();
    m_pCompositeTrans.reset();
}


void CCompositeFilterTest::testConstructor()
{
  CCompositeFilter filter;
  CPPUNIT_ASSERT(filter.begin() == filter.end()); 
}



void CCompositeFilterTest::testRegisterFilter()
{
  CCompositeFilter filter;
  CPPUNIT_ASSERT(filter.begin() == filter.end()); 
  filter.registerFilter(m_pFilter);

  // Check that this is no longer empty
  CPPUNIT_ASSERT(filter.begin() != filter.end());
  // It should have only one filter
  CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(1) , filter.size() );
  
}

void CCompositeFilterTest::testProcessTransparentFilter()
{
  // CReate a generic new item to pass in
  CRingItemPtr pItem(new CRawRingItem);
  pItem->setType(100);

  // Setup composite with transparent filter and handle the 
  // ring item with it 
  CRingItemPtr pNewItem = m_pCompositeTrans->handleRingItem(pItem);
  // Verify that the composite didn't delete my item
  CPPUNIT_ASSERT( pNewItem == pItem );
}

void CCompositeFilterTest::testTransparentStateChangeItem()
{
  // CReate a generic new item to pass in
  CRingStateChangeItemPtr pItem(new CRingStateChangeItem(END_RUN));
  // Setup composite with transparent filter and handle the 
  // ring item with it
  auto pNewItem = m_pCompositeTrans->handleStateChangeItem(pItem);
  // Verify that the composite didn't delete my item
  CPPUNIT_ASSERT( pNewItem == pItem );

} 

void CCompositeFilterTest::testTestStateChangeItem()
{
  // CReate a generic new item to pass in
  CRingStateChangeItemPtr pItem(new CRingStateChangeItem(END_RUN));
  // Setup composite with transparent filter and handle the 
  // ring item with it
  CRingStateChangeItemPtr pNewItem = m_pCompositeTest->handleStateChangeItem(pItem);
  // Verify that the composite didn't delete my item
  CPPUNIT_ASSERT( pNewItem != pItem );

  // Test filter should always return type BEGIN_RUN
  CPPUNIT_ASSERT( BEGIN_RUN == pNewItem->type() );

} 

// Create scaler item
void CCompositeFilterTest::testTransparentScalerItem()
{
  CRingScalerItemPtr pItem(new CRingScalerItem (300));
  // Setup composite with transparent filter and handle the 
  // ring item with it
  CRingScalerItemPtr pNewItem = m_pCompositeTrans->handleScalerItem(pItem);
  // Verify that the composite didn't delete my item
  CPPUNIT_ASSERT( pNewItem == pItem );
}

// Create scaler item
void CCompositeFilterTest::testTestScalerItem()
{
  CRingScalerItemPtr pItem(new CRingScalerItem (300));
  // Setup composite with transparent filter and handle the 
  // ring item with it
  CRingScalerItemPtr pNewItem = m_pCompositeTest->handleScalerItem(pItem);
  // Verify that the composite didn't delete my item
  CPPUNIT_ASSERT( pNewItem != pItem );

  CPPUNIT_ASSERT( 200 == pNewItem->getScalerCount() );

}
  
// Text item
void CCompositeFilterTest::testTransparentTextItem()
{
  std::vector<std::string> str_vec;
  str_vec.push_back("testing 123");
  CRingTextItemPtr pItem(new CRingTextItem(MONITORED_VARIABLES,str_vec));

  CRingTextItemPtr pNewItem = m_pCompositeTrans->handleTextItem(pItem);

  CPPUNIT_ASSERT (pNewItem == pItem);

}

// Text item
void CCompositeFilterTest::testTestTextItem()
{
  std::vector<std::string> str_vec;
  str_vec.push_back("testing 123");
  CRingTextItemPtr pItem(new CRingTextItem(MONITORED_VARIABLES,str_vec));
  CRingTextItemPtr pNewItem = m_pCompositeTest->handleTextItem(pItem);

  CPPUNIT_ASSERT( 3 == pNewItem->getStrings().size() );
  CPPUNIT_ASSERT( "0000" == pNewItem->getStrings()[0] );
  CPPUNIT_ASSERT( "1111" == pNewItem->getStrings()[1] );
  CPPUNIT_ASSERT( "2222" == pNewItem->getStrings()[2] );
}

// PhysicsEvent item
void CCompositeFilterTest::testTransparentPhysicsEventItem()
{
  CPhysicsEventItemPtr pItem(new CPhysicsEventItem);

  CPhysicsEventItemPtr pNewItem = m_pCompositeTrans->handlePhysicsEventItem(pItem);

  CPPUNIT_ASSERT( pItem == pNewItem );

}    

// PhysicsEvent item
void CCompositeFilterTest::testTestPhysicsEventItem()
{
  CPhysicsEventItemPtr pItem(new CPhysicsEventItem);
  pItem->getBody().resize(4096);

  CPhysicsEventItemPtr pNewItem = m_pCompositeTest->handlePhysicsEventItem(pItem);
  CPPUNIT_ASSERT( pItem != pNewItem );

}    

// PhysicsEventCount item
void CCompositeFilterTest::testTransparentPhysicsEventCountItem()
{
    CRingPhysicsEventCountItemPtr pItem (
                new CRingPhysicsEventCountItem(static_cast<uint64_t>(100),
                                               static_cast<uint32_t>(100))
                );

  CRingPhysicsEventCountItemPtr pNewItem = m_pCompositeTrans->handlePhysicsEventCountItem(pItem);

  CPPUNIT_ASSERT( pItem == pNewItem);
}    

// PhysicsEventCount item
void CCompositeFilterTest::testTestPhysicsEventCountItem()
{
  CRingPhysicsEventCountItemPtr pItem(
              new CRingPhysicsEventCountItem(static_cast<uint64_t>(100),
                                         static_cast<uint32_t>(100))
              );

  auto pNewItem = m_pCompositeTest->handlePhysicsEventCountItem(pItem);

  CPPUNIT_ASSERT( pItem != pNewItem);
  CPPUNIT_ASSERT( static_cast<uint64_t>(4) == pNewItem->getEventCount() );
  CPPUNIT_ASSERT( static_cast<uint32_t>(1001) == pNewItem->getTimeOffset() );

}    

// Create some generic item type to force testing of handleRingItem()
void CCompositeFilterTest::testTransparentGenericItem()
{
  CRingItemPtr pItem(new CRawRingItem);
  pItem->setType(1000);
  CRingItemPtr pNewItem = m_pCompositeTrans->handleRingItem(pItem);
  CPPUNIT_ASSERT( pItem == pNewItem );

}

// Create some generic item type to force testing of handleRingItem()
void CCompositeFilterTest::testTestGenericItem()
{
  CRingItemPtr pItem(new CRawRingItem);
  pItem->setType(1000);

  CRingItemPtr pNewItem = m_pCompositeTest->handleRingItem(pItem);
  CPPUNIT_ASSERT( pItem != pNewItem );
  // Test filter should always return type 100
  CPPUNIT_ASSERT( 100 == pNewItem->type() );

}


void CCompositeFilterTest::testInitialize0()
{
  CTestFilterPtr f1(new CTestFilter);
  CTestFilterPtr f2(new CTestFilter);
  m_pCompositeTest->registerFilter(f1);
  m_pCompositeTest->registerFilter(f2);

  m_pCompositeTest->initialize();
  CCompositeFilter::iterator it = m_pCompositeTest->begin();
  CCompositeFilter::iterator itend = m_pCompositeTest->end();
  while (it!=itend) {
    CTestFilterPtr tfilt = std::static_pointer_cast<CTestFilter>(*it);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>({"initialize"}), tfilt->getHistory());
    ++it;
  }

}

void CCompositeFilterTest::testFinalize0()
{
  CTestFilterPtr f1(new CTestFilter);
  CTestFilterPtr f2(new CTestFilter);
  m_pCompositeTest->registerFilter(f1);
  m_pCompositeTest->registerFilter(f2);

  m_pCompositeTest->finalize();
  CCompositeFilter::iterator it = m_pCompositeTest->begin();
  CCompositeFilter::iterator itend = m_pCompositeTest->end();
  while (it!=itend) {
    CTestFilterPtr tfilt = std::static_pointer_cast<CTestFilter>(*it);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>({"finalize"}), tfilt->getHistory());
    ++it;
  }

}

// Create some generic item type to force testing of handleRingItem()
void CCompositeFilterTest::testExitsOnNullReturn()
{
  CRingItemPtr pItem(new CRawRingItem);
  pItem->setType(100);

  // Register it
  CCompositeFilter composite;

  // Create a test filter 
  CNullFilterPtr m_filter(new CNullFilter);
  composite.registerFilter(m_filter);

  CTestFilterPtr pFilter(new CTestFilter);
  composite.registerFilter(pFilter);

  CRingItemPtr pNewItem = composite.handleRingItem(pItem);

  // Check that it does return 0
  CPPUNIT_ASSERT( pNewItem == nullptr );

  // check that the test filter never got called
  CCompositeFilter::iterator it = composite.begin();
  ++it;
  CTestFilterPtr theTest = std::static_pointer_cast<CTestFilter>(*it);

  CPPUNIT_ASSERT( 0 == theTest->getNProcessed() );

}




// CAbnormalEndItem
void CCompositeFilterTest::testAbnormalEndItem()
{
  CAbnormalEndItemPtr pItem(new CAbnormalEndItem);

  auto pNewItem = m_pCompositeTest->handleAbnormalEndItem(pItem);

  CPPUNIT_ASSERT( pNewItem != pItem);
}



// CAbnormalEndItem
void CCompositeFilterTest::testGlomParameters()
{
  auto pItem = std::make_shared<CGlomParameters>(10, true, CGlomParameters::first);

  auto pNewItem = m_pCompositeTest->handleGlomParameters(pItem);

  CPPUNIT_ASSERT( pNewItem != pItem);

}


void CCompositeFilterTest::testDataFormatItem()
{
    auto pItem = std::make_shared<CDataFormatItem>();

    auto pNewItem = m_pCompositeTest->handleDataFormatItem(pItem);

    CPPUNIT_ASSERT( pNewItem != pItem);
}
