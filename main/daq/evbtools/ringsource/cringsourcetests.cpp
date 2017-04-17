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
#include <V12/Serialize.h>
#include <V12/DataFormat.h>

#include <CTestSourceSink.h>
#include <RingIOV12.h>

#include "CRingSource.h"

#include <fragment.h>

#include <array>
#include <vector>
#include <limits>
#include <vector>
#include <memory>
#include <iostream>
#include <iterator>

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
    CPPUNIT_TEST(getEvent_2);
    CPPUNIT_TEST(getEvent_3);
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
  void getEvent_2();
  void getEvent_3();
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

    begin.setSourceId(2);
    end.setSourceId(2);

    writeItem(*m_pRing, begin);
    writeItem(*m_pRing, begin);
    writeItem(*m_pRing, end);
    writeItem(*m_pRing, end);

    m_pSource->transformAvailableData();

    EQMSG("Observation of 2 end runs for 2 sources, oneshot -> complete",
          true, m_pSource->oneshotComplete());
}

// Simply test that we can wrap a physics event with a fragment header...
// This is representative of all other types besides state change types
void CRingSourceTest::getEvent_2() {

    V12::CPhysicsEventItem item(1234, 2, {0,1,2,3,4,5});

    writeItem(*m_pRing, item);

    m_pSource->transformAvailableData();

    auto& list = m_pSource->getFragmentList();
    ClientEventFragment frag = list.front();

    EQMSG("timestamp", uint64_t(1234), frag.s_timestamp);
    EQMSG("source id", uint32_t(2), frag.s_sourceId);
    EQMSG("payload size", uint32_t(26), frag.s_size);
    EQMSG("barrier", uint32_t(0), frag.s_barrierType);

    auto serialItem = V12::serializeItem(item);
    ASSERTMSG("payload", std::equal(serialItem.begin(), serialItem.end(), frag.s_payload));
}



// Simply test that we can wrap a physics event with a fragment header...
// This is representative of all other types besides state change types
void CRingSourceTest::getEvent_3() {

    V12::CRingStateChangeItem item(V12::BEGIN_RUN);
    item.setSourceId(2);

    writeItem(*m_pRing, item);

    m_pSource->transformAvailableData();

    auto& list = m_pSource->getFragmentList();
    ClientEventFragment frag = list.front();

    EQMSG("timestamp", V12::NULL_TIMESTAMP, frag.s_timestamp);
    EQMSG("source id", uint32_t(2), frag.s_sourceId);
    EQMSG("payload size", item.size(), frag.s_size);
    EQMSG("barrier", V12::BEGIN_RUN, frag.s_barrierType);

    auto serialItem = V12::serializeItem(item);

    std::copy(serialItem.begin(), serialItem.end(), std::ostream_iterator<char>(std::cout, " "));

    ASSERTMSG("payload", std::equal(serialItem.begin(), serialItem.end(), frag.s_payload));
}





