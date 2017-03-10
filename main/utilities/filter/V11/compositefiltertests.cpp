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

#include <V11/CPhysicsEventItem.h>
#include <V11/CRingStateChangeItem.h>
#include <V11/CRingScalerItem.h>
#include <V11/CRingTextItem.h>
#include <V11/CRingPhysicsEventCountItem.h>
#include <V11/CRingFragmentItem.h>

#include "V11/CTransparentFilter.h"
#include "V11/CNullFilter.h"
#include "V11/CTestFilter.h"

#define private public
#define protected public
#include "V11/CCompositeFilter.h"
#undef private
#undef protected 

#include <ios>
#include <algorithm>
#include <fstream>
#include <iterator>
#include <string>
#include <fstream>
#include <vector>

using namespace DAQ::V11;

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
    CPPUNIT_TEST ( testTransparentFragmentItem );
    CPPUNIT_TEST ( testTestFragmentItem );
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
  CRingItem* item = new CRingItem(100,100); 
  // Setup composite with transparent filter and handle the 
  // ring item with it 
  CRingItem* new_item = m_pCompositeTrans->handleRingItem(item);
  // Verify that the composite didn't delete my item
  CPPUNIT_ASSERT( new_item == item );
  
  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}

void CCompositeFilterTest::testTransparentStateChangeItem()
{
  // CReate a generic new item to pass in
  CRingStateChangeItem* item = new CRingStateChangeItem(END_RUN); 
  // Setup composite with transparent filter and handle the 
  // ring item with it
  CRingItem* new_item = m_pCompositeTrans->handleStateChangeItem(item);
  // Verify that the composite didn't delete my item
  CPPUNIT_ASSERT( new_item == item );

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
} 

void CCompositeFilterTest::testTestStateChangeItem()
{
  // CReate a generic new item to pass in
  CRingStateChangeItem* item = new CRingStateChangeItem(END_RUN); 
  // Setup composite with transparent filter and handle the 
  // ring item with it
  CRingItem* new_item = m_pCompositeTest->handleStateChangeItem(item);
  // Verify that the composite didn't delete my item
  CPPUNIT_ASSERT( new_item != item );

  // Test filter should always return type BEGIN_RUN
  CPPUNIT_ASSERT( BEGIN_RUN == new_item->type() );

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;

} 

// Create scaler item
void CCompositeFilterTest::testTransparentScalerItem()
{
  CRingScalerItem* item = new CRingScalerItem (300);
  // Setup composite with transparent filter and handle the 
  // ring item with it
  CRingItem* new_item = m_pCompositeTrans->handleScalerItem(item);
  // Verify that the composite didn't delete my item
  CPPUNIT_ASSERT( new_item == item );

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}

// Create scaler item
void CCompositeFilterTest::testTestScalerItem()
{
  CRingScalerItem* item = new CRingScalerItem (300);
  // Setup composite with transparent filter and handle the 
  // ring item with it
  CRingItem* new_item = m_pCompositeTest->handleScalerItem(item);
  // Verify that the composite didn't delete my item
  CPPUNIT_ASSERT( new_item != item );

  CRingScalerItem* new_sclr = static_cast<CRingScalerItem*>(new_item);
  CPPUNIT_ASSERT( 200 == new_sclr->getScalerCount() );

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}
  
// Text item
void CCompositeFilterTest::testTransparentTextItem()
{
  std::vector<std::string> str_vec;
  str_vec.push_back("testing 123");
  CRingTextItem* item = new CRingTextItem(MONITORED_VARIABLES,str_vec);

  CRingItem* new_item = m_pCompositeTrans->handleTextItem(item);

  CPPUNIT_ASSERT (new_item == item);

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;

}

// Text item
void CCompositeFilterTest::testTestTextItem()
{
  std::vector<std::string> str_vec;
  str_vec.push_back("testing 123");
  CRingTextItem* item = new CRingTextItem(MONITORED_VARIABLES,str_vec);
  CRingItem* new_item = m_pCompositeTest->handleTextItem(item);
  // Test filter should always return 200 scalers
  CRingTextItem* new_text = dynamic_cast<CRingTextItem*>(new_item);

  CPPUNIT_ASSERT( 3 == new_text->getStrings().size() );
  CPPUNIT_ASSERT( "0000" == new_text->getStrings()[0] );
  CPPUNIT_ASSERT( "1111" == new_text->getStrings()[1] );
  CPPUNIT_ASSERT( "2222" == new_text->getStrings()[2] );

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}

// PhysicsEvent item
void CCompositeFilterTest::testTransparentPhysicsEventItem()
{
  CPhysicsEventItem* item = new CPhysicsEventItem(8192);

  CRingItem* new_item = m_pCompositeTrans->handlePhysicsEventItem(item);

  CPPUNIT_ASSERT( item == new_item );

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}    

// PhysicsEvent item
void CCompositeFilterTest::testTestPhysicsEventItem()
{
  CPhysicsEventItem* item = new CPhysicsEventItem(8192);

  CRingItem* new_item = m_pCompositeTest->handlePhysicsEventItem(item);
  CPPUNIT_ASSERT( item != new_item );

  CPhysicsEventItem* new_evt = dynamic_cast<CPhysicsEventItem*>(new_item);

  CPPUNIT_ASSERT( 4096 == new_evt->getStorageSize() );

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}    

// PhysicsEventCount item
void CCompositeFilterTest::testTransparentPhysicsEventCountItem()
{
  CRingPhysicsEventCountItem* item = 0;
  item =  new CRingPhysicsEventCountItem(static_cast<uint64_t>(100),
                                         static_cast<uint32_t>(100));

  CRingItem* new_item = m_pCompositeTrans->handlePhysicsEventCountItem(item);
  CRingPhysicsEventCountItem* new_cnt 
    = dynamic_cast<CRingPhysicsEventCountItem*>(new_item);

  CPPUNIT_ASSERT( item == new_item);

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}    

// PhysicsEventCount item
void CCompositeFilterTest::testTestPhysicsEventCountItem()
{
  CRingPhysicsEventCountItem* item = 0;
  item =  new CRingPhysicsEventCountItem(static_cast<uint64_t>(100),
                                         static_cast<uint32_t>(100));

  CRingItem* new_item = m_pCompositeTest->handlePhysicsEventCountItem(item);
  CRingPhysicsEventCountItem* new_cnt 
    = dynamic_cast<CRingPhysicsEventCountItem*>(new_item);

  CPPUNIT_ASSERT( item != new_item);
  CPPUNIT_ASSERT( static_cast<uint64_t>(4) == new_cnt->getEventCount() );
  CPPUNIT_ASSERT( static_cast<uint32_t>(1001) == new_cnt->getTimeOffset() );

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}    

// RingFragmentItem
void CCompositeFilterTest::testTransparentFragmentItem()
{
  CRingFragmentItem* item = new CRingFragmentItem(static_cast<uint64_t>(0),
                                                        static_cast<uint32_t>(0),
                                                        static_cast<uint32_t>(0),
                                                        reinterpret_cast<void*>(0),
                                                        static_cast<uint32_t>(0));

  CRingItem* new_item = m_pCompositeTrans->handleFragmentItem(item);

  CPPUNIT_ASSERT( new_item == item);

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}

// RingFragmentItem
void CCompositeFilterTest::testTestFragmentItem()
{
  CRingFragmentItem* item = new CRingFragmentItem(static_cast<uint64_t>(0),
                                                        static_cast<uint32_t>(0),
                                                        static_cast<uint32_t>(0),
                                                        reinterpret_cast<void*>(0),
                                                        static_cast<uint32_t>(0));

  CRingItem* new_item = m_pCompositeTest->handleFragmentItem(item);

  CPPUNIT_ASSERT( new_item != item);

  CRingFragmentItem* new_frag = dynamic_cast<CRingFragmentItem*>(new_item);
  CPPUNIT_ASSERT( static_cast<uint64_t>(10101) == new_frag->timestamp());
  CPPUNIT_ASSERT( static_cast<uint32_t>(1) == new_frag->source());
  CPPUNIT_ASSERT( static_cast<uint32_t>(2) == const_cast<CRingFragmentItem*>(new_frag)->payloadSize());
  CPPUNIT_ASSERT( static_cast<uint32_t>(3) == new_frag->barrierType());

  if (item != new_item) {
    delete new_item;
  }
  
  delete item;
}

// Create some generic item type to force testing of handleRingItem()
void CCompositeFilterTest::testTransparentGenericItem()
{
  CRingItem* item = new CRingItem(1000);    
  CRingItem* new_item = m_pCompositeTrans->handleRingItem(item);
  CPPUNIT_ASSERT( item == new_item );

  if (item != new_item) {
    delete new_item;
  }

  delete item;
}

// Create some generic item type to force testing of handleRingItem()
void CCompositeFilterTest::testTestGenericItem()
{
  CRingItem* item = new CRingItem(1000);    
  CRingItem* new_item = m_pCompositeTest->handleRingItem(item);
  CPPUNIT_ASSERT( item != new_item );
  // Test filter should always return type 100
  CPPUNIT_ASSERT( 100 == new_item->type() );

  if (item != new_item) {
    delete new_item;
  }

  delete item;
}


void CCompositeFilterTest::testInitialize0()
{
  auto  f1 = std::make_shared<CTestFilter>();
  auto  f2 = std::make_shared<CTestFilter>();
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
  auto  f1 = std::make_shared<CTestFilter>();
  auto  f2 = std::make_shared<CTestFilter>();
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
  CRingItem *item = new CRingItem(100);
  // Register it
  CCompositeFilter composite;

  // Create a test filter 
  CNullFilterPtr m_filter(new CNullFilter);
  composite.registerFilter(m_filter);

  CTestFilterPtr pFilter(new CTestFilter);
  composite.registerFilter(pFilter);

  CRingItem* new_item = composite.handleRingItem(item);

  // Check that it does return 0
  CPPUNIT_ASSERT( 0 == new_item );

  // check that the test filter never got called
  CCompositeFilter::iterator it = composite.begin();
  ++it;
  CTestFilterPtr theTest = std::static_pointer_cast<CTestFilter>(*it);

  CPPUNIT_ASSERT( 0 == theTest->getNProcessed() );

  if (item != new_item) {
    delete new_item;
  }

  delete item;
}




// CAbnormalEndItem
void CCompositeFilterTest::testAbnormalEndItem()
{
  auto pItem = new CAbnormalEndItem();

  CRingItem* new_item = m_pCompositeTest->handleAbnormalEndItem(pItem);

  CPPUNIT_ASSERT( new_item != pItem);

  if (pItem != new_item) {
    delete new_item;
  }

  delete pItem;
}



// CAbnormalEndItem
void CCompositeFilterTest::testGlomParameters()
{
  auto pItem = new CGlomParameters(10, true, CGlomParameters::first);

  CRingItem* new_item = m_pCompositeTest->handleGlomParameters(pItem);

  CPPUNIT_ASSERT( new_item != pItem);

  if (pItem != new_item) {
    delete new_item;
  }

  delete pItem;
}


void CCompositeFilterTest::testDataFormatItem()
{
    auto pItem = new CDataFormatItem;

    CRingItem* new_item = m_pCompositeTest->handleDataFormatItem(pItem);

    CPPUNIT_ASSERT( new_item != pItem);

    if (pItem != new_item) {
      delete new_item;
    }

    delete pItem;
}
