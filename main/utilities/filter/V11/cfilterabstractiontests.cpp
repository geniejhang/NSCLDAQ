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


static const char* Copyright = "(C) Copyright Michigan State University 2014, All rights reserved";


#include <cppunit/extensions/HelperMacros.h>
#include <Asserts.h>
#include <DebugUtils.h>

#include <V11/CPhysicsEventItem.h>
#include <V11/CRingStateChangeItem.h>
#include <V11/CRingScalerItem.h>
#include <V11/CRingTextItem.h>
#include <V11/CRingPhysicsEventCountItem.h>
#include <V11/CRingFragmentItem.h>
#include <V11/CDataFormatItem.h>
#include <V11/CAbnormalEndItem.h>
#include <V11/CAbnormalEndItem.h>
#include <V11/CGlomParameters.h>
#include <V11/CDataFormatItem.h>
#include <V11/DataFormatV11.h>

#include <V11/CTestFilter.h>

#define private public
#include <V11/CFilterAbstraction.h>
#undef private

#include <ios>
#include <algorithm>
#include <fstream>
#include <iterator>
#include <string>
#include <fstream>
#include <vector>

using namespace std;
using namespace DAQ::V11;


class CFilterAbstractionTests : public CppUnit::TestFixture
{

    CPPUNIT_TEST_SUITE( CFilterAbstractionTests );
    CPPUNIT_TEST(dispatch_0);
    CPPUNIT_TEST(dispatch_1);
    CPPUNIT_TEST(dispatch_2);
    CPPUNIT_TEST(dispatch_3);
    CPPUNIT_TEST(dispatch_4);
    CPPUNIT_TEST(dispatch_5);
    CPPUNIT_TEST(dispatch_6);
    CPPUNIT_TEST(dispatch_7);
    CPPUNIT_TEST(dispatch_8);
    CPPUNIT_TEST(dispatch_9);
    CPPUNIT_TEST(cleanup_0);
    CPPUNIT_TEST_SUITE_END();

private:
    CTestFilterPtr     m_pFilter;
    CFilterAbstraction m_abstraction;

public:

    void setUp() {
        m_abstraction = CFilterAbstraction();
        m_pFilter.reset(new CTestFilter);
        m_abstraction.registerFilter(m_pFilter);
    }

    void tearDown() {}

    void dispatch_0() {
        auto pInput = new CRingStateChangeItem(BEGIN_RUN);
        auto pOutput = m_abstraction.dispatch(*pInput);

        auto history = m_pFilter->getHistory();
        EQMSG("dispatch state change", vector<string>({"handleStateChangeItem"}), history);
    }


    void dispatch_1() {
        auto pInput = new CRingScalerItem(0);
        auto pOutput = m_abstraction.dispatch(*pInput);

        auto history = m_pFilter->getHistory();
        EQMSG("dispatch scaler", vector<string>({"handleScalerItem"}), history);
    }

    void dispatch_2() {
        auto pInput = new CRingTextItem(MONITORED_VARIABLES, {});
        auto pOutput = m_abstraction.dispatch(*pInput);

        auto history = m_pFilter->getHistory();
        EQMSG("dispatch text", vector<string>({"handleTextItem"}), history);
    }

    void dispatch_3() {
        auto pInput = new CPhysicsEventItem();
        auto pOutput = m_abstraction.dispatch(*pInput);

        auto history = m_pFilter->getHistory();
        EQMSG("dispatch physics event", vector<string>({"handlePhysicsEventItem"}), history);
    }

    void dispatch_4() {
        auto pInput = new CRingPhysicsEventCountItem();
        auto pOutput = m_abstraction.dispatch(*pInput);

        auto history = m_pFilter->getHistory();
        EQMSG("dispatch physics event count", vector<string>({"handlePhysicsEventCountItem"}), history);
    }

    void dispatch_5() {
        auto pInput = new CRingFragmentItem(1234567890, 1, 0, nullptr);
        auto pOutput = m_abstraction.dispatch(*pInput);

        auto history = m_pFilter->getHistory();
        EQMSG("dispatch fragment", vector<string>({"handleFragmentItem"}), history);
    }

    void dispatch_6() {
        auto pInput = new CRingItem(49);
        auto pOutput = m_abstraction.dispatch(*pInput);

        auto history = m_pFilter->getHistory();
        EQMSG("dispatch generic ring item", vector<string>({"handleRingItem"}), history);
    }

    void dispatch_7() {
        auto pInput = new CDataFormatItem();
        auto pOutput = m_abstraction.dispatch(*pInput);

        auto history = m_pFilter->getHistory();
        EQMSG("dispatch data format item",
              vector<string>({"handleDataFormatItem"}), history);
    }

    void dispatch_8() {
        auto pInput = new CAbnormalEndItem();
        auto pOutput = m_abstraction.dispatch(*pInput);

        auto history = m_pFilter->getHistory();
        EQMSG("dispatch abnormal end run item",
              vector<string>({"handleAbnormalEndItem"}), history);
    }


    void dispatch_9() {
        auto pInput = new CGlomParameters(10, true, CGlomParameters::first);
        auto pOutput = m_abstraction.dispatch(*pInput);

        auto history = m_pFilter->getHistory();
        EQMSG("dispatch glom parameters",
              vector<string>({"handleGlomParameters"}), history);
    }

    void cleanup_0() {
        // make sure that if the input and output items are the same, we don't
        // blow up with a double free
        auto pInput = new CRingItem(23);
        m_abstraction.m_pInputItem = pInput;
        m_abstraction.m_pOutputItem = pInput;

        m_abstraction.cleanUp();

        EQMSG("input item should be nullptr after cleanup",
              (CRingItem*)(nullptr), m_abstraction.m_pInputItem);

        EQMSG("output item should be nullptr after cleanup",
              (CRingItem*)(nullptr), m_abstraction.m_pOutputItem);
    }

};


CPPUNIT_TEST_SUITE_REGISTRATION( CFilterAbstractionTests );
