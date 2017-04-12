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


#include <Asserts.h>

#include <cppunit/extensions/HelperMacros.h>

#include <V12/CPhysicsEventItem.h>
#include <V12/CRingStateChangeItem.h>
#include <V12/DataFormat.h>

#include <CTestSourceSink.h>
#include <RingIOV12.h>

#include "CRingSource.h"

#include <array>
#include <vector>
#include <limits>
#include <vector>
#include <memory>
#include <iostream>

using namespace std;
using namespace DAQ;

// A test suite 
class CRingSourceTest : public CppUnit::TestFixture
{

  private:
    CRingSource* m_pSource;
    CTestSourceSinkPtr m_pRing;
    bool m_ownRing;

  public:

    CPPUNIT_TEST_SUITE( CRingSourceTest );
    CPPUNIT_TEST(getEvent_0);
    CPPUNIT_TEST(getEvent_1);
    CPPUNIT_TEST_SUITE_END();

  public:
    void setUp() {

      std::vector<uint32_t> okids;
      okids.push_back(2);
      m_pRing= make_shared<CTestSourceSink>();
      m_pSource = new CRingSource(m_pRing, okids);
    }

    void tearDown() {
      delete m_pSource;
    }
protected:
  void getEvent_0();
  void getEvent_1();
};
// Register it with the test factory

CPPUNIT_TEST_SUITE_REGISTRATION( CRingSourceTest );

    
void CRingSourceTest::getEvent_0() {
    V12::CPhysicsEventItem item(1, 2);

    writeItem(*m_pRing, item);

    m_pSource->transformAvailableData();

    ASSERT( m_pSource->getFragmentList().size() == 1);
}

void CRingSourceTest::getEvent_1() {

    m_pSource->setOneshot(true);
    m_pSource->setNumberOfSources(2);
    V12::CRingStateChangeItem begin(V12::BEGIN_RUN);
    V12::CRingStateChangeItem end(V12::END_RUN);
    writeItem(*m_pRing, begin);
    writeItem(*m_pRing, end);

    m_pSource->transformAvailableData();

    EQMSG("Observation of 2 end runs for 2 sources, oneshot -> complete",
          true, m_pSource->oneshotComplete());
}







