
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

#define private public
#define protected public
#include <CTransform12p0to11p0.h>
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
 * \brief The CTransform12p0to11p0Tests_Scaler class
 *
 * Tests the validity of the transform of scaler items
 */
class CTransform12p0to11p0Tests_Scaler : public CppUnit::TestFixture
{
private:
    CTransform12p0to11p0                    m_transform;
    V11::CRingScalerItem                    v11item;
    V12::CRingScalerItem                    v12item;
    std::time_t                             time_now;

public:
    CPPUNIT_TEST_SUITE(CTransform12p0to11p0Tests_Scaler);
    CPPUNIT_TEST(scaler_0);
    CPPUNIT_TEST(scaler_1);
    CPPUNIT_TEST(scaler_2);
    CPPUNIT_TEST(scaler_3);
    CPPUNIT_TEST(scaler_4);
    CPPUNIT_TEST(scaler_5);
    CPPUNIT_TEST(scaler_6);
    CPPUNIT_TEST(scaler_7);
    CPPUNIT_TEST_SUITE_END();

public:
    CTransform12p0to11p0Tests_Scaler()
        : m_transform(),
          v11item(1),
          v12item(1, 2, 3, 4, time_now, {1}, 5, false, 123),
          time_now() {}

    void setUp() {
        using namespace std::chrono;

        // force this be different than now
        time_now = system_clock::to_time_t( system_clock::now() ) + 1;

        m_transform = CTransform12p0to11p0();
        v12item     = V12::CRingScalerItem(1234, // evt tstamp
                                                 56,   // source id
                                                 14,    // start time
                                                 1,    // stop time
                                                 time_now,  // timestmp
                                                 {0, 1, 2, 3}, // scalers
                                                 2,    // time divisor
                                                 false, // incremental?
                                                 17 ); // scaler width

        V12::CRawRingItem rawItem(v12item);
        v11item     = m_transform(rawItem);
    }

  void tearDown() {
  }
protected:
void scaler_0() {
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "V12::PERIODIC_SCALERS becomes V11::PERIODIC_SCALERS",
                V11::PERIODIC_SCALERS, v11item.type());
}
void scaler_1()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Timestamp becomes event timestamp",
                                 uint64_t(1234), v11item.getEventTimestamp());
}
void scaler_2()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Start time offset remains unchanged",
                                 uint32_t(14), v11item.getStartTime());
}
void scaler_3()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("End time offset remains unchanged",
                                 uint32_t(1), v11item.getEndTime());
}
void scaler_4()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Timestamp should remain the same",
                                 time_now, v11item.getTimestamp());
}
void scaler_5()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Interval divisor remains the same",
                                 uint32_t(2), v11item.getTimeDivisor());
}
void scaler_6()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Scaler count remains the same",
                                 uint32_t(4), v11item.getScalerCount());
}
void scaler_7()
{
    CPPUNIT_ASSERT_MESSAGE(
                "Scalers remain the same",
                vector<uint32_t>({0, 1, 2, 3}) == v11item.getScalers());
}

}; // end of non-incr scaler tests
CPPUNIT_TEST_SUITE_REGISTRATION(CTransform12p0to11p0Tests_Scaler);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/*!
 * \brief The CTransform12p0to11p0Tests_State class
 *
 * Checks that state change items transform according to the rules
 */
class CTransform12p0to11p0Tests_State : public CppUnit::TestFixture
{
private:
    CTransform12p0to11p0       m_transform;
    V11::CRingStateChangeItem  v11item;
    V12::CRingStateChangeItem  v12item;
    std::time_t                time_now;

public:
    CPPUNIT_TEST_SUITE(CTransform12p0to11p0Tests_State);
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
    CTransform12p0to11p0Tests_State()
        : m_transform(), v11item(1), v12item(1), time_now() {}

    void setUp() {
        using namespace std::chrono;

        // force this be different than now
        time_now = system_clock::to_time_t( system_clock::now() ) + 1;

        m_transform = CTransform12p0to11p0();
        v12item = V12::CRingStateChangeItem(987, // tstamp
                                           9, // source id
                                           V12::BEGIN_RUN,
                                           42, // run number
                                           1000, // time offset
                                           time_now, // timestamp
                                           "You would like to know", // title
                                           3); // time divisor
        v11item     = m_transform(v12item);
    }
  void tearDown() {
  }
protected:
void state_0() {
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "BEGIN_RUN -- > BEGIN_RUN",
                V11::BEGIN_RUN, v11item.type());
}
void state_1()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Run number remains unchanged",
                                 uint32_t(42), v11item.getRunNumber());
}
void state_2()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Time offset remains unchanged",
                                 uint32_t(1000), v11item.getElapsedTime());
}
void state_3()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Timestamp remains the same",
                                 time_now, v11item.getTimestamp());
}
void state_4()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Title remains the same",
                                 string("You would like to know"),
                                 v11item.getTitle());
}

void state_5()
{
    v12item = V12::CRingStateChangeItem(V12::END_RUN);
    v11item = m_transform(v12item);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "END_RUN -- > END_RUN",
                V11::END_RUN, v11item.type());
}
void state_6()
{
    v12item = V12::CRingStateChangeItem(V12::PAUSE_RUN);
    v11item = m_transform(v12item);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "PAUSE_RUN -- > PAUSE_RUN",
                V11::PAUSE_RUN, v11item.type());
}
void state_7()
{
    v12item = V12::CRingStateChangeItem(V12::RESUME_RUN);
    v11item = m_transform(v12item);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "RESUME_RUN -- > RESUME_RUN",
                V11::RESUME_RUN, v11item.type());
}

void state_8()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Event timestmap remains same",
                                 uint64_t(987),
                                 v11item.getEventTimestamp());
}

void state_9()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Source id remains same",
                                 uint32_t(9),
                                 v11item.getSourceId());
}

void state_10()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Barrier type is same as type",
                                 V11::BEGIN_RUN,
                                 v11item.getBarrierType());
}

void state_11()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Body header is present",
                                 true,
                                 v11item.hasBodyHeader());
}


}; // end of state tests

CPPUNIT_TEST_SUITE_REGISTRATION(CTransform12p0to11p0Tests_State);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/*!
 * \brief The CTransform12p0to11p0Tests_PhysicsEvent class
 *
 *  Verifies that the rules are obeyed for physics event transforms
 */
class CTransform12p0to11p0Tests_PhysicsEvent : public CppUnit::TestFixture
{
private:
    CTransform12p0to11p0       m_transform;
    V11::CPhysicsEventItem v11item;
    V12::CPhysicsEventItem v12item;
    vector<uint8_t>              data;

public:
    CPPUNIT_TEST_SUITE(CTransform12p0to11p0Tests_PhysicsEvent);
    CPPUNIT_TEST(physicsEvent_0);
    CPPUNIT_TEST(physicsEvent_1);
    CPPUNIT_TEST(physicsEvent_2);
    CPPUNIT_TEST(physicsEvent_3);
    CPPUNIT_TEST(physicsEvent_4);
    CPPUNIT_TEST_SUITE_END();

public:
    CTransform12p0to11p0Tests_PhysicsEvent()
        : m_transform(), v11item(1), v12item(),data() {}

    void setUp() {
        m_transform = CTransform12p0to11p0();

        data.resize(32);
        iota(data.begin(), data.end(), 0);

        v12item     = V12::CPhysicsEventItem(12345, 6);
        v12item.setBody(data);

        v11item     = m_transform(v12item);
    }
  void tearDown() {
  }
protected:
void physicsEvent_0() {
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "PHYSICS_EVENT -- > PHYSICS_EVENT",
                V11::PHYSICS_EVENT, v11item.type());
}
void physicsEvent_1()
{

  auto pBody = reinterpret_cast<uint8_t*>(v11item.getBodyPointer());
  CPPUNIT_ASSERT_MESSAGE("Body data remains same",
                          std::equal(data.begin(), data.end(), pBody));
}

void physicsEvent_2()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("event timestamp transforms unchanged",
                                uint64_t(12345), v11item.getEventTimestamp());
}

void physicsEvent_3()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("source id timestamp transforms unchanged",
                                uint32_t(6), v11item.getSourceId());
}

void physicsEvent_4()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("PHYSICS_EVENT --> PHYSICS_EVENT",
                                V11::PHYSICS_EVENT, v11item.type());
}

}; // end of physicsEvent tests

CPPUNIT_TEST_SUITE_REGISTRATION(CTransform12p0to11p0Tests_PhysicsEvent);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/*!
 * \brief The CTransform12p0to11p0Tests_EventCount class
 *
 *  Verifies that the rules of transformaton are obeyed for PHYSICS_EVENT_COUNT
 */
class CTransform12p0to11p0Tests_EventCount : public CppUnit::TestFixture
{
private:
    CTransform12p0to11p0       m_transform;
    V11::CRingPhysicsEventCountItem v11item;
    V12::CRingPhysicsEventCountItem v12item;
    std::time_t time_now;

public:
    CPPUNIT_TEST_SUITE(CTransform12p0to11p0Tests_EventCount);
    CPPUNIT_TEST(eventCount_0);
    CPPUNIT_TEST(eventCount_1);
    CPPUNIT_TEST(eventCount_2);
    CPPUNIT_TEST(eventCount_3);
    CPPUNIT_TEST(eventCount_4);
    CPPUNIT_TEST(eventCount_5);
    CPPUNIT_TEST(eventCount_6);
    CPPUNIT_TEST(eventCount_7);
    CPPUNIT_TEST_SUITE_END();

public:
    CTransform12p0to11p0Tests_EventCount()
        : m_transform(), v11item(), v12item(), time_now() {}

    void setUp() {

        using namespace std::chrono;

        // force this be different than now
        time_now = system_clock::to_time_t( system_clock::now() ) + 1;

        m_transform = CTransform12p0to11p0();

        v12item     = V12::CRingPhysicsEventCountItem(12345, 6, 8, 9, time_now, 11);

        v11item     = m_transform(v12item);
    }
  void tearDown() {
  }
protected:
void eventCount_0() {
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "PHYSICS_EVENT_COUNT -- > PHYSICS_EVENT_COUNT",
                V11::PHYSICS_EVENT_COUNT, v11item.type());
}

void eventCount_1()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Elapsed time remains same",
                                uint32_t(9), v11item.getTimeOffset());
}

void eventCount_2()
{

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Event count remains same",
                                uint64_t(8), v11item.getEventCount());
}

void eventCount_3()
{

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Unix timestamp remains same",
                                time_now, v11item.getTimestamp());
}

void eventCount_4()
{

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Event timestamp remains the same",
                                uint64_t(12345), v11item.getEventTimestamp());
}

void eventCount_5()
{

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Source id remains the same",
                                uint32_t(6), v11item.getSourceId());
}
void eventCount_6()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("v11 has body header",
                                true, v11item.hasBodyHeader());
}
void eventCount_7()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Time divisor transforms unchanged",
                                uint32_t(11), v11item.getTimeDivisor());
}


}; // end of eventCount tests

CPPUNIT_TEST_SUITE_REGISTRATION(CTransform12p0to11p0Tests_EventCount);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class CTransform12p0to11p0Tests_Text : public CppUnit::TestFixture
{
private:
    CTransform12p0to11p0       m_transform;
    V11::CRingTextItem   v11item;
    V12::CRingTextItem   v12item;
    std::time_t                time_now;
    std::vector<std::string>   strings;

public:
    CPPUNIT_TEST_SUITE(CTransform12p0to11p0Tests_Text);
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
    CTransform12p0to11p0Tests_Text()
        : m_transform(),
          v11item(V11::PACKET_TYPES, {}),
          v12item(V12::PACKET_TYPES, {}),
          time_now(),
          strings() {}

    void setUp() {

        using namespace std::chrono;
        time_now = system_clock::to_time_t(system_clock::now())+1;

        strings = {"the", "test", "strings"};

        m_transform = CTransform12p0to11p0();

        v12item     = V12::CRingTextItem(V12::PACKET_TYPES,
                                               12345, 6,
                                               strings, 56, time_now,
                                               3);

        v11item     = m_transform(v12item);
    }
  void tearDown() {
  }
protected:
void Text_0() {
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "PACKET_TYPES --> PACKET_TYPES",
                V11::PACKET_TYPES, v11item.type());
}
void Text_1()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Time offset remains the same",
                                uint32_t(56), v11item.getTimeOffset());
}
void Text_2()
{

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Unix timestamp remains the same",
                                time_now, v11item.getTimestamp());
}
void Text_3()
{

  CPPUNIT_ASSERT_EQUAL_MESSAGE("String count remains unchanged",
                               uint32_t(strings.size()),
                               v11item.getStringCount());
}
void Text_4()
{
  CPPUNIT_ASSERT_MESSAGE("Strings transform unchanged",
                         strings == v11item.getStrings());

}

void Text_5()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Event timestamp transforms unchanged",
                                uint64_t(12345), v11item.getEventTimestamp());
}

void Text_6()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Source id transforms unchanged",
                                uint32_t(6), v11item.getSourceId());
}

void Text_7()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("barrier becomes 0",
                                uint32_t(0), v11item.getBarrierType());
}

void Text_8()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("body header exists in v11 item",
                                true, v11item.hasBodyHeader());
}


}; // end of text tests

CPPUNIT_TEST_SUITE_REGISTRATION(CTransform12p0to11p0Tests_Text);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class CTransform12p0to11p0Tests_AbnormalEnd : public CppUnit::TestFixture
{
private:
    CTransform12p0to11p0       m_transform;
    V11::CAbnormalEndItem      v11item;
    V12::CAbnormalEndItem      v12item;
    std::time_t                time_now;
    std::vector<std::string>   strings;

public:
    CPPUNIT_TEST_SUITE(CTransform12p0to11p0Tests_AbnormalEnd);
    CPPUNIT_TEST(AbnormalEnd_0);
    CPPUNIT_TEST(AbnormalEnd_5);
    CPPUNIT_TEST(AbnormalEnd_6);
    CPPUNIT_TEST(AbnormalEnd_7);
    CPPUNIT_TEST(AbnormalEnd_8);
    CPPUNIT_TEST_SUITE_END();

public:
    CTransform12p0to11p0Tests_AbnormalEnd()
        : m_transform(),
          v11item(),
          v12item(),
          time_now(),
          strings() {}

    void setUp() {

        using namespace std::chrono;
        time_now = system_clock::to_time_t(system_clock::now())+1;

        strings = {"the", "test", "strings"};

        m_transform = CTransform12p0to11p0();

        v12item     = V12::CAbnormalEndItem();
        v12item.setSourceId(23);
        v12item.setEventTimestamp(V12::NULL_TIMESTAMP);

        v11item     = m_transform(v12item);
    }
  void tearDown() {
  }
protected:
void AbnormalEnd_0() {
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "ABNORMAL_ENDRUN --> ABNORMAL_ENDRUN",
                V11::ABNORMAL_ENDRUN, v11item.type());
}

void AbnormalEnd_5()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Event timestamp transforms unchanged",
                                uint64_t(0xffffffffffffffff), v11item.getEventTimestamp());
}

void AbnormalEnd_6()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Source id transforms unchanged",
                                uint32_t(23), v11item.getSourceId());
}

void AbnormalEnd_7()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("barrier becomes 0",
                                uint32_t(0), v11item.getBarrierType());
}

void AbnormalEnd_8()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("body header exists in v11 item",
                                true, v11item.hasBodyHeader());
}


}; // end of text tests

CPPUNIT_TEST_SUITE_REGISTRATION(CTransform12p0to11p0Tests_AbnormalEnd);


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

class CTransform12p0to11p0Tests_GlomParameters : public CppUnit::TestFixture
{
private:
    CTransform12p0to11p0       m_transform;
    V11::CGlomParameters       v11item;
    V12::CGlomParameters       v12item;

public:
    CPPUNIT_TEST_SUITE(CTransform12p0to11p0Tests_GlomParameters);
    CPPUNIT_TEST(GlomParameters_0);
    CPPUNIT_TEST(GlomParameters_1);
    CPPUNIT_TEST(GlomParameters_2);
    CPPUNIT_TEST(GlomParameters_3);
    CPPUNIT_TEST(GlomParameters_8);
    CPPUNIT_TEST_SUITE_END();

public:
    CTransform12p0to11p0Tests_GlomParameters()
        : m_transform(),
          v11item(0, false, V11::CGlomParameters::first),
          v12item(0, false, V12::CGlomParameters::first)
    {}

    void setUp() {

        using namespace std::chrono;

        m_transform = CTransform12p0to11p0();

        v12item     = V12::CGlomParameters(123, false, V12::CGlomParameters::last);
        v12item.setSourceId(23);
        v12item.setEventTimestamp(V12::NULL_TIMESTAMP);

        v11item     = m_transform(v12item);
    }
  void tearDown() {
  }

protected:
void GlomParameters_0() {
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "EVB_GLOM_INFO --> EVB_GLOM_INFO",
                V11::EVB_GLOM_INFO, v11item.type());
}


void GlomParameters_1()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Interval transforms unchanged",
                                uint64_t(123), v11item.coincidenceTicks());
}

void GlomParameters_2()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Is building transforms unchanged",
                                false, v11item.isBuilding());
}

void GlomParameters_3()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Timestamp policy stays the same",
                                V11::CGlomParameters::last, v11item.timestampPolicy());
}


void GlomParameters_8()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("body header exists in v11 item",
                                false, v11item.hasBodyHeader());
}


}; // end of text tests

CPPUNIT_TEST_SUITE_REGISTRATION(CTransform12p0to11p0Tests_GlomParameters);





class CTransform12p0to11p0Tests_General : public CppUnit::TestFixture
{
private:
    CTransform12p0to11p0       m_transform;

public:
    CPPUNIT_TEST_SUITE(CTransform12p0to11p0Tests_General);
    CPPUNIT_TEST(Test_0);
//    CPPUNIT_TEST(Test_1);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {
        m_transform = CTransform12p0to11p0();
    }
  void tearDown() {}
protected:
void Test_0() {
    V12::CCompositeRingItem item(V12::COMP_PHYSICS_EVENT, 0, 1, {});
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Composite ring items do not convert",
                                  V11::UNDEFINED, m_transform(item).type() );
}

}; // end of Fragment tests

CPPUNIT_TEST_SUITE_REGISTRATION(CTransform12p0to11p0Tests_General);

