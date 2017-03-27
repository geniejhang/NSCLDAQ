

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


#include <cppunit/Asserter.h>
#include <cppunit/extensions/HelperMacros.h>
#include "Asserts.h"

#include <V12/CRingScalerItem.h>
#include <V11/CRingScalerItem.h>

#include <V12/CRingStateChangeItem.h>
#include <V11/CRingStateChangeItem.h>

#include <V12/CPhysicsEventItem.h>
#include <V11/CPhysicsEventItem.h>

#include <V12/CRingPhysicsEventCountItem.h>
#include <V11/CRingPhysicsEventCountItem.h>

#include <V12/CRingTextItem.h>
#include <V11/CRingTextItem.h>

#include <V12/CDataFormatItem.h>
#include <V11/CDataFormatItem.h>

#include <V12/DataFormat.h>
#include <V11/DataFormat.h>

#include <V12/CAbnormalEndItem.h>
#include <V11/CAbnormalEndItem.h>

#include <V12/CGlomParameters.h>
#include <V11/CGlomParameters.h>

#include <V12/CCompositeRingItem.h>
#include <V11/CRingFragmentItem.h>
#include <V11/CUnknownFragment.h>

#define private public
#define protected public
#include <CTransform11p0to12p0.h>
#undef protected
#undef private

#include <chrono>
#include <iterator>
#include <algorithm>
#include <stdexcept>

using namespace std;
using namespace DAQ;
using namespace DAQ::Transform;

/*!
 * \brief The CTransform11p0to12p0Tests_Scaler class
 *
 * Tests the validity of the transform of scaler items
 */
class CTransform11p0to12p0Tests_Scaler : public CppUnit::TestFixture
{
private:
    CTransform11p0to12p0                    m_transform;
    V11::CRingScalerItem                    v11item;
    V12::CRingScalerItem                    v12item;
    std::time_t                             time_now;

public:
    CPPUNIT_TEST_SUITE(CTransform11p0to12p0Tests_Scaler);
    CPPUNIT_TEST(scaler_0);
    CPPUNIT_TEST(scaler_1);
    CPPUNIT_TEST(scaler_2);
    CPPUNIT_TEST(scaler_3);
    CPPUNIT_TEST(scaler_4);
    CPPUNIT_TEST(scaler_5);
    CPPUNIT_TEST(scaler_6);
    CPPUNIT_TEST(scaler_7);
    CPPUNIT_TEST(scaler_8);
    CPPUNIT_TEST(scaler_9);
    CPPUNIT_TEST_SUITE_END();

public:
    CTransform11p0to12p0Tests_Scaler()
        : m_transform(),
          v11item(1),
          v12item(1),
          time_now() {}

    void setUp() {
        using namespace std::chrono;

        // force this be different than now
        time_now = system_clock::to_time_t( system_clock::now() ) + 1;

        m_transform = CTransform11p0to12p0();
        v11item     = V11::CRingScalerItem(1234, // evt tstamp
                                                 56,   // source id
                                                 0, // barrier
                                                 14,    // start time
                                                 1,    // stop time
                                                 time_now,  // timestmp
                                                 {0, 1, 2, 3}, // scalers
                                                 2,    // time divisor
                                                 false); // incremental?

        v12item     = m_transform(v11item);
    }

  void tearDown() {
  }
protected:
void scaler_0() {
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "V11::PERIODIC_SCALERS becomes V12::PERIODIC_SCALERS",
                V12::PERIODIC_SCALERS, v12item.type());
}
void scaler_1()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Timestamp becomes event timestamp",
                                 uint64_t(1234), v12item.getEventTimestamp());
}
void scaler_2()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Start time offset remains unchanged",
                                 uint32_t(14), v12item.getStartTime());
}
void scaler_3()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("End time offset remains unchanged",
                                 uint32_t(1), v12item.getEndTime());
}
void scaler_4()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Timestamp should remain the same",
                                 time_now, v12item.getTimestamp());
}
void scaler_5()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Interval divisor remains the same",
                                 uint32_t(2), v12item.getTimeDivisor());
}
void scaler_6()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Scaler count remains the same",
                                 uint32_t(4), v12item.getScalerCount());
}
void scaler_7()
{
    CPPUNIT_ASSERT_MESSAGE(
                "Scalers remain the same",
                vector<uint32_t>({0, 1, 2, 3}) == v12item.getScalers());
}

void setUpWithoutBodyHeader () {

    v11item     = V11::CRingScalerItem(14,    // start time
                                       1,    // stop time
                                       time_now,  // timestmp
                                       {0, 1, 2, 3}, // scalers
                                       false,    // time divisor
                                       2); // incremental?

    v12item     = m_transform(v11item);
}

void scaler_8 () {

    setUpWithoutBodyHeader();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default timestamp used when no body header",
                                 V12::NULL_TIMESTAMP, v12item.getEventTimestamp());


}

void scaler_9 () {

    setUpWithoutBodyHeader();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default source id used when no body header",
                                 uint32_t(0), v12item.getSourceId());

}


}; // end of non-incr scaler tests
CPPUNIT_TEST_SUITE_REGISTRATION(CTransform11p0to12p0Tests_Scaler);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/*!
 * \brief The CTransform11p0to12p0Tests_State class
 *
 * Checks that state change items transform according to the rules
 */
class CTransform11p0to12p0Tests_State : public CppUnit::TestFixture
{
private:
    CTransform11p0to12p0       m_transform;
    V11::CRingStateChangeItem  v11item;
    V12::CRingStateChangeItem  v12item;
    std::time_t                time_now;

public:
    CPPUNIT_TEST_SUITE(CTransform11p0to12p0Tests_State);
    CPPUNIT_TEST(state_0);
    CPPUNIT_TEST(state_1);
    CPPUNIT_TEST(state_2);
    CPPUNIT_TEST(state_3);
    CPPUNIT_TEST(state_4);
    CPPUNIT_TEST(state_5);
    CPPUNIT_TEST(state_6);
    CPPUNIT_TEST(state_7);
    CPPUNIT_TEST(state_8);
    CPPUNIT_TEST(state_9);
    CPPUNIT_TEST(state_10);
    CPPUNIT_TEST(state_11);
    CPPUNIT_TEST_SUITE_END();

public:
    CTransform11p0to12p0Tests_State()
        : m_transform(), v11item(1), v12item(1), time_now() {}

    void setUp() {
        using namespace std::chrono;

        // force this be different than now
        time_now = system_clock::to_time_t( system_clock::now() ) + 1;

        m_transform = CTransform11p0to12p0();
        v11item = V11::CRingStateChangeItem(987, // tstamp
                                           9, // source id
                                           V12::BEGIN_RUN, // barrier type
                                           V12::BEGIN_RUN,
                                           42, // run number
                                           1000, // time offset
                                           time_now, // timestamp
                                           "You would like to know", // title
                                           3); // time divisor
        v12item     = m_transform(v11item);
    }
  void tearDown() {
  }
protected:
void state_0() {
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "BEGIN_RUN -- > BEGIN_RUN",
                V12::BEGIN_RUN, v12item.type());
}
void state_1()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Run number remains unchanged",
                                 uint32_t(42), v12item.getRunNumber());
}
void state_2()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Time offset remains unchanged",
                                 uint32_t(1000), v12item.getElapsedTime());
}
void state_3()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Timestamp remains the same",
                                 time_now, v12item.getTimestamp());
}
void state_4()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Title remains the same",
                                 string("You would like to know"),
                                 v12item.getTitle());
}

void state_5()
{
    v11item = V11::CRingStateChangeItem(V11::END_RUN);
    v12item = m_transform(v11item);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "END_RUN -- > END_RUN",
                V12::END_RUN, v12item.type());
}
void state_6()
{
    v11item = V11::CRingStateChangeItem(V11::PAUSE_RUN);
    v12item = m_transform(v11item);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "PAUSE_RUN -- > PAUSE_RUN",
                V12::PAUSE_RUN, v12item.type());
}
void state_7()
{
    v11item = V11::CRingStateChangeItem(V11::RESUME_RUN);
    v12item = m_transform(v11item);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "RESUME_RUN -- > RESUME_RUN",
                V12::RESUME_RUN, v12item.type());
}

void state_8()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Event timestmap remains same",
                                 uint64_t(987),
                                 v12item.getEventTimestamp());
}

void state_9()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Source id remains same",
                                 uint32_t(9),
                                 v12item.getSourceId());
}

void state_10()
{
  v11item = V11::CRingStateChangeItem(V11::BEGIN_RUN);

  v12item = m_transform(v11item);

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Event timestamp is NULL_TIMESTAMP when bh is messing",
                                V12::NULL_TIMESTAMP,
                                v12item.getEventTimestamp());
}

void state_11()
{
  v11item = V11::CRingStateChangeItem(V11::BEGIN_RUN);

  v12item = m_transform(v11item);

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Event timestamp is NULL_TIMESTAMP when bh is messing",
                                uint32_t(0),
                                v12item.getSourceId());
}


}; // end of state tests

CPPUNIT_TEST_SUITE_REGISTRATION(CTransform11p0to12p0Tests_State);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/*!
 * \brief The CTransform11p0to12p0Tests_Fragment class
 *
 *  Verifies that the rules are obeyed for physics event transforms
 */
class CTransform11p0to12p0Tests_Fragment : public CppUnit::TestFixture
{
private:
    CTransform11p0to12p0       m_transform;
    V11::CRingFragmentItem     v11item;
    V12::CPhysicsEventItem     v12item;
    vector<uint8_t>            data;

public:
    CPPUNIT_TEST_SUITE(CTransform11p0to12p0Tests_Fragment);
    CPPUNIT_TEST(fragment_0);
    CPPUNIT_TEST(fragment_1);
    CPPUNIT_TEST(fragment_2);
    CPPUNIT_TEST(fragment_3);
    CPPUNIT_TEST_SUITE_END();

public:
    CTransform11p0to12p0Tests_Fragment()
        : m_transform(), v11item(V11::EVB_FRAGMENT), v12item(),data() {}

    void setUp() {
        m_transform = CTransform11p0to12p0();

        data.resize(32);
        iota(data.begin(), data.end(), 0);

        v11item     = V11::CRingFragmentItem(12345, 6, data.size(), data.data(), 0);
        v12item     = m_transform(v11item);
    }
  void tearDown() {
  }
protected:
void fragment_0() {
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "EVB_FRAGMENT -- > PHYSICS_EVENT",
                V12::PHYSICS_EVENT, v12item.type());
}
void fragment_1()
{

  CPPUNIT_ASSERT_MESSAGE("Body data remains same",
                          data == v12item.getBody());
}

void fragment_2()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("event timestamp transforms unchanged",
                                uint64_t(12345), v12item.getEventTimestamp());
}

void fragment_3()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("source id timestamp transforms unchanged",
                                uint32_t(6), v12item.getSourceId());
}

}; // end of fragment tests

CPPUNIT_TEST_SUITE_REGISTRATION(CTransform11p0to12p0Tests_Fragment);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/*!
 * \brief The CTransform11p0to12p0Tests_UnknownFragment class
 *
 *  Verifies that the rules are obeyed for physics event transforms
 */
class CTransform11p0to12p0Tests_UnknownFragment : public CppUnit::TestFixture
{
private:
    CTransform11p0to12p0       m_transform;
    V11::CUnknownFragment      v11item;
    V12::CPhysicsEventItem     v12item;
    vector<uint8_t>            data;

public:
    CPPUNIT_TEST_SUITE(CTransform11p0to12p0Tests_UnknownFragment);
    CPPUNIT_TEST(fragment_0);
    CPPUNIT_TEST(fragment_1);
    CPPUNIT_TEST(fragment_2);
    CPPUNIT_TEST(fragment_3);
    CPPUNIT_TEST_SUITE_END();

public:
    CTransform11p0to12p0Tests_UnknownFragment()
        : m_transform(), v11item(V11::EVB_UNKNOWN_PAYLOAD), v12item(),data() {}

    void setUp() {
        m_transform = CTransform11p0to12p0();

        data.resize(32);
        iota(data.begin(), data.end(), 0);

        v11item     = V11::CUnknownFragment(12345, 6, 0, data.size(), data.data());
        v12item     = m_transform(v11item);
    }
  void tearDown() {
  }
protected:
void fragment_0() {
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "EVB_UNKNOWN_PAYLOAD -- > PHYSICS_EVENT",
                V12::PHYSICS_EVENT, v12item.type());
}
void fragment_1()
{
  CPPUNIT_ASSERT_MESSAGE("Body data remains same",
                          data == v12item.getBody());
}

void fragment_2()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("event timestamp transforms unchanged",
                                uint64_t(12345), v12item.getEventTimestamp());
}

void fragment_3()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("source id timestamp transforms unchanged",
                                uint32_t(6), v12item.getSourceId());
}

}; // end of fragment tests

CPPUNIT_TEST_SUITE_REGISTRATION(CTransform11p0to12p0Tests_UnknownFragment);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/*!
 * \brief The CTransform11p0to12p0Tests_PhysicsEvent class
 *
 *  Verifies that the rules are obeyed for physics event transforms
 */
class CTransform11p0to12p0Tests_PhysicsEvent : public CppUnit::TestFixture
{
private:
    CTransform11p0to12p0       m_transform;
    V11::CPhysicsEventItem     v11item;
    V12::CPhysicsEventItem     v12item;
    vector<uint8_t>            data;

public:
    CPPUNIT_TEST_SUITE(CTransform11p0to12p0Tests_PhysicsEvent);
    CPPUNIT_TEST(physicsEvent_0);
    CPPUNIT_TEST(physicsEvent_1);
    CPPUNIT_TEST(physicsEvent_2);
    CPPUNIT_TEST(physicsEvent_3);
    CPPUNIT_TEST(physicsEvent_4);
    CPPUNIT_TEST(physicsEvent_5);
    CPPUNIT_TEST(physicsEvent_6);
    CPPUNIT_TEST_SUITE_END();

public:
    CTransform11p0to12p0Tests_PhysicsEvent()
        : m_transform(), v11item(1), v12item(),data() {}

    void setUp() {
        m_transform = CTransform11p0to12p0();

        data.resize(32);
        iota(data.begin(), data.end(), 0);

        v11item     = V11::CPhysicsEventItem();
        auto pBody = reinterpret_cast<uint8_t*>(v11item.getBodyPointer());
        pBody = std::copy(data.begin(), data.end(), pBody);
        v11item.setBodyCursor(pBody);
        v11item.updateSize();

        v11item.setBodyHeader(12345, 6, 0);

        v12item     = m_transform(v11item);
    }
  void tearDown() {
  }
protected:
void physicsEvent_0() {
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "PHYSICS_EVENT -- > PHYSICS_EVENT",
                V12::PHYSICS_EVENT, v12item.type());
}
void physicsEvent_1()
{

  CPPUNIT_ASSERT_MESSAGE("Body data remains same",
                          data == v12item.getBody());
}

void physicsEvent_2()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("event timestamp transforms unchanged",
                                uint64_t(12345), v12item.getEventTimestamp());
}

void physicsEvent_3()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("source id timestamp transforms unchanged",
                                uint32_t(6), v12item.getSourceId());
}

void physicsEvent_4()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("PHYSICS_EVENT --> PHYSICS_EVENT",
                                V12::PHYSICS_EVENT, v12item.type());
}


void physicsEvent_5()
{
  v11item = V11::CPhysicsEventItem();
  v12item = m_transform(v11item);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("event timestamp becomes V12::NULL_TIMESTAMP if no body header",
                              V12::NULL_TIMESTAMP,
                              v12item.getEventTimestamp());
}

void physicsEvent_6()
{
  v11item = V11::CPhysicsEventItem();
  v12item = m_transform(v11item);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("source id becomes 0 if no body header",
                              uint32_t(0),
                              v12item.getSourceId());
}

}; // end of physicsEvent tests

CPPUNIT_TEST_SUITE_REGISTRATION(CTransform11p0to12p0Tests_PhysicsEvent);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/*!
 * \brief The CTransform11p0to12p0Tests_EventCount class
 *
 *  Verifies that the rules of transformaton are obeyed for PHYSICS_EVENT_COUNT
 */
class CTransform11p0to12p0Tests_EventCount : public CppUnit::TestFixture
{
private:
    CTransform11p0to12p0       m_transform;
    V11::CRingPhysicsEventCountItem v11item;
    V12::CRingPhysicsEventCountItem v12item;
    std::time_t time_now;

public:
    CPPUNIT_TEST_SUITE(CTransform11p0to12p0Tests_EventCount);
    CPPUNIT_TEST(eventCount_0);
    CPPUNIT_TEST(eventCount_1);
    CPPUNIT_TEST(eventCount_2);
    CPPUNIT_TEST(eventCount_3);
    CPPUNIT_TEST(eventCount_4);
    CPPUNIT_TEST(eventCount_5);
    CPPUNIT_TEST(eventCount_6);
    CPPUNIT_TEST(eventCount_7);
    CPPUNIT_TEST(eventCount_8);
    CPPUNIT_TEST_SUITE_END();

public:
    CTransform11p0to12p0Tests_EventCount()
        : m_transform(), v11item(), v12item(), time_now() {}

    void setUp() {

        using namespace std::chrono;

        // force this be different than now
        time_now = system_clock::to_time_t( system_clock::now() ) + 1;

        m_transform = CTransform11p0to12p0();

        v11item     = V11::CRingPhysicsEventCountItem(12345, 6, 0, 8, 9, time_now, 11);

        v12item     = m_transform(v11item);
    }
  void tearDown() {
  }
protected:
void eventCount_0() {
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "PHYSICS_EVENT_COUNT -- > PHYSICS_EVENT_COUNT",
                V12::PHYSICS_EVENT_COUNT, v12item.type());
}

void eventCount_1()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Elapsed time remains same",
                                uint32_t(9), v12item.getTimeOffset());
}

void eventCount_2()
{

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Event count remains same",
                                uint64_t(8), v12item.getEventCount());
}

void eventCount_3()
{

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Unix timestamp remains same",
                                time_now, v12item.getTimestamp());
}

void eventCount_4()
{

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Event timestamp remains the same",
                                uint64_t(12345), v12item.getEventTimestamp());
}

void eventCount_5()
{

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Source id remains the same",
                                uint32_t(6), v12item.getSourceId());
}
void eventCount_6()
{
  v11item = V11::CRingPhysicsEventCountItem();
  v12item = m_transform(v11item);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("event timestamp becomes V12::NULL_TIMESTAMP if no body header",
                                V12::NULL_TIMESTAMP, 
                                v12item.getEventTimestamp());
}
void eventCount_7()
{
  v11item = V11::CRingPhysicsEventCountItem();
  v12item = m_transform(v11item);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("source id becomes 0 if no body header",
                                uint32_t(0), v12item.getSourceId());
}

void eventCount_8()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("time divisor remains the same",
                                uint32_t(11), v12item.getTimeDivisor());
}


}; // end of eventCount tests

CPPUNIT_TEST_SUITE_REGISTRATION(CTransform11p0to12p0Tests_EventCount);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class CTransform11p0to12p0Tests_Text : public CppUnit::TestFixture
{
private:
    CTransform11p0to12p0       m_transform;
    V11::CRingTextItem   v11item;
    V12::CRingTextItem   v12item;
    std::time_t                time_now;
    std::vector<std::string>   strings;

public:
    CPPUNIT_TEST_SUITE(CTransform11p0to12p0Tests_Text);
    CPPUNIT_TEST(Text_0);
    CPPUNIT_TEST(Text_1);
    CPPUNIT_TEST(Text_2);
    CPPUNIT_TEST(Text_3);
    CPPUNIT_TEST(Text_4);
    CPPUNIT_TEST(Text_5);
    CPPUNIT_TEST(Text_6);
    CPPUNIT_TEST(Text_7);
    CPPUNIT_TEST(Text_8);
    CPPUNIT_TEST_SUITE_END();

public:
    CTransform11p0to12p0Tests_Text()
        : m_transform(),
          v11item(V11::PACKET_TYPES, {}),
          v12item(V12::PACKET_TYPES, {}),
          time_now(),
          strings() {}

    void setUp() {

        using namespace std::chrono;
        time_now = system_clock::to_time_t(system_clock::now())+1;

        strings = {"the", "test", "strings"};

        m_transform = CTransform11p0to12p0();

        v11item     = V11::CRingTextItem(V11::PACKET_TYPES,
                                               12345, 6, 0,
                                               strings, 56, time_now,
                                               3);

        v12item     = m_transform(v11item);
    }
  void tearDown() {
  }
protected:
void Text_0() {
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "PACKET_TYPES --> PACKET_TYPES",
                V12::PACKET_TYPES, v12item.type());
}
void Text_1()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Time offset remains the same",
                                uint32_t(56), v12item.getTimeOffset());
}
void Text_2()
{

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Unix timestamp remains the same",
                                time_now, v12item.getTimestamp());
}
void Text_3()
{

  CPPUNIT_ASSERT_EQUAL_MESSAGE("String count remains unchanged",
                               uint32_t(strings.size()),
                               v12item.getStringCount());
}
void Text_4()
{
  CPPUNIT_ASSERT_MESSAGE("Strings transform unchanged",
                         strings == v12item.getStrings());

}

void Text_5()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Event timestamp transforms unchanged",
                                uint64_t(12345), v12item.getEventTimestamp());
}

void Text_6()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Source id transforms unchanged",
                                uint32_t(6), v12item.getSourceId());
}

void Text_7()
{
  v11item = V11::CRingTextItem(V11::PACKET_TYPES, {"asdf"});
  v12item = m_transform(v11item);

  CPPUNIT_ASSERT_EQUAL_MESSAGE("event timestamp becoms V12::NULL_TIMESTAMP if no body header",
                                V12::NULL_TIMESTAMP, v12item.getEventTimestamp());
}

void Text_8()
{
  v11item = V11::CRingTextItem(V11::PACKET_TYPES, {"asdf"});
  v12item = m_transform(v11item);

  CPPUNIT_ASSERT_EQUAL_MESSAGE("source id becomes 0 if no body header",
                                uint32_t(0), v12item.getSourceId());
}


}; // end of text tests

CPPUNIT_TEST_SUITE_REGISTRATION(CTransform11p0to12p0Tests_Text);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class CTransform11p0to12p0Tests_AbnormalEnd : public CppUnit::TestFixture
{
private:
    CTransform11p0to12p0       m_transform;
    V11::CAbnormalEndItem      v11item;
    V12::CAbnormalEndItem      v12item;

public:
    CPPUNIT_TEST_SUITE(CTransform11p0to12p0Tests_AbnormalEnd);
    CPPUNIT_TEST(AbnormalEnd_0);
    CPPUNIT_TEST(AbnormalEnd_5);
    CPPUNIT_TEST(AbnormalEnd_6);
    CPPUNIT_TEST_SUITE_END();

public:
    CTransform11p0to12p0Tests_AbnormalEnd()
        : m_transform(),
          v11item(),
          v12item() {}

    void setUp() {

        m_transform = CTransform11p0to12p0();

        v11item     = V11::CAbnormalEndItem();

        v12item     = m_transform(v11item);
    }
  void tearDown() {
  }
protected:
void AbnormalEnd_0() {
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "ABNORMAL_ENDRUN --> ABNORMAL_ENDRUN",
                V12::ABNORMAL_ENDRUN, v12item.type());
}

void AbnormalEnd_5()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Event timestamp becomes V12::NULL_TIMESTAMP",
                                V12::NULL_TIMESTAMP, v12item.getEventTimestamp());
}

void AbnormalEnd_6()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Source id becomes 0",
                                uint32_t(0), v12item.getSourceId());
}

}; // end of text tests

CPPUNIT_TEST_SUITE_REGISTRATION(CTransform11p0to12p0Tests_AbnormalEnd);


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

class CTransform11p0to12p0Tests_GlomParameters : public CppUnit::TestFixture
{
private:
    CTransform11p0to12p0       m_transform;
    V11::CGlomParameters       v11item;
    V12::CGlomParameters       v12item;

public:
    CPPUNIT_TEST_SUITE(CTransform11p0to12p0Tests_GlomParameters);
    CPPUNIT_TEST(GlomParameters_0);
    CPPUNIT_TEST(GlomParameters_1);
    CPPUNIT_TEST(GlomParameters_2);
    CPPUNIT_TEST(GlomParameters_3);
    CPPUNIT_TEST_SUITE_END();

public:
    CTransform11p0to12p0Tests_GlomParameters()
        : m_transform(),
          v11item(0, false, V11::CGlomParameters::first),
          v12item(0, false, V12::CGlomParameters::first)
    {}

    void setUp() {

        using namespace std::chrono;

        m_transform = CTransform11p0to12p0();

        v11item     = V11::CGlomParameters(123, false, V11::CGlomParameters::last);
        v12item     = m_transform(v11item);
    }
  void tearDown() {
  }

protected:
void GlomParameters_0() {
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "EVB_GLOM_INFO --> EVB_GLOM_INFO",
                V12::EVB_GLOM_INFO, v12item.type());
}


void GlomParameters_1()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Interval transforms unchanged",
                                uint64_t(123), v12item.coincidenceTicks());
}

void GlomParameters_2()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Is building transforms unchanged",
                                false, v12item.isBuilding());
}

void GlomParameters_3()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Timestamp policy stays the same",
                                V12::CGlomParameters::last, v12item.timestampPolicy());
}

}; // end of text tests

CPPUNIT_TEST_SUITE_REGISTRATION(CTransform11p0to12p0Tests_GlomParameters);





//class CTransform11p0to12p0Tests_General : public CppUnit::TestFixture
//{
//private:
//    CTransform11p0to12p0       m_transform;

//public:
//    CPPUNIT_TEST_SUITE(CTransform11p0to12p0Tests_General);
//    CPPUNIT_TEST(Test_0);
////    CPPUNIT_TEST(Test_1);
//    CPPUNIT_TEST_SUITE_END();

//public:
//    void setUp() {
//        m_transform = CTransform11p0to12p0();
//    }
//  void tearDown() {}
//protected:
//void Test_0() {
//    V12::CCompositeRingItem item(V12::COMP_PHYSICS_EVENT, 0, 1, {});
//    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Composite ring items do not convert",
//                                  V11::UNDEFINED, m_transform(item).type() );
//}

//}; // end of Fragment tests

//CPPUNIT_TEST_SUITE_REGISTRATION(CTransform11p0to12p0Tests_General);

