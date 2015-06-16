#include <cppunit/Asserter.h>
#include <cppunit/extensions/HelperMacros.h>
#include "Asserts.h"

#include <NSCLDAQ10/CRingScalerItem.h>
#include <NSCLDAQ11/CRingScalerItem.h>

#include <NSCLDAQ10/CRingStateChangeItem.h>
#include <NSCLDAQ11/CRingStateChangeItem.h>

#include <NSCLDAQ10/CPhysicsEventItem.h>
#include <NSCLDAQ11/CPhysicsEventItem.h>

#include <NSCLDAQ10/CRingPhysicsEventCountItem.h>
#include <NSCLDAQ11/CRingPhysicsEventCountItem.h>

#include <NSCLDAQ10/CRingTextItem.h>
#include <NSCLDAQ11/CRingTextItem.h>

#include <NSCLDAQ10/CRingTimestampedRunningScalerItem.h>

#include <NSCLDAQ10/CRingFragmentItem.h>
#include <NSCLDAQ11/CRingFragmentItem.h>
#include <NSCLDAQ10/CUnknownFragment.h>
#include <NSCLDAQ11/CUnknownFragment.h>

#include <NSCLDAQ11/CDataFormatItem.h>

#include <NSCLDAQ10/DataFormatV10.h>
#include <NSCLDAQ11/DataFormatV11.h>

#define private public
#define protected public
#include <CTransform11p0to10p0.h>
#undef protected
#undef private

#include <chrono>
#include <iterator>
#include <algorithm>
#include <stdexcept>

using namespace std;

class CTransform11p0to10p0Tests_NonIncrScaler : public CppUnit::TestFixture
{
private:
    CTransform11p0to10p0       m_transform;
    NSCLDAQ10::CRingTimestampedRunningScalerItem v10item;
    NSCLDAQ11::CRingScalerItem v11item;
    std::time_t time_now;

public:
    CPPUNIT_TEST_SUITE(CTransform11p0to10p0Tests_NonIncrScaler);
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
    CTransform11p0to10p0Tests_NonIncrScaler()
        : m_transform(),
          v10item(1, 2, 3, 4, time_now, {1}),
          v11item(1), time_now() {}

    void setUp() {
        using namespace std::chrono;

        // force this be different than now
        time_now = system_clock::to_time_t( system_clock::now() ) + 1;

        m_transform = CTransform11p0to10p0();
        v11item     = NSCLDAQ11::CRingScalerItem(1234, // evt tstamp
                                                 56,   // source id
                                                 78,   // barrier
                                                 14,    // start time
                                                 1,    // stop time
                                                 time_now,  // timestmp
                                                 {0, 1, 2, 3}, // scalers
                                                 2,    // time divisor
                                                 false);  // incremental?


        v10item     = m_transform(v11item);
    }
  void tearDown() {
  }
protected:
void scaler_0() {
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "Non-incr scaler becomes TIMESTAMPED_NONINCR_SCALER",
                NSCLDAQ10::TIMESTAMPED_NONINCR_SCALERS, v10item.type());
}
void scaler_1()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Timestamp becomes event timestamp",
                                 uint64_t(1234), v10item.getTimestamp());
}
void scaler_2()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Start time offset remains unchanged",
                                 uint32_t(14), v10item.getOffsetStart());
}
void scaler_3()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("End time offset remains unchanged",
                                 uint32_t(1), v10item.getOffsetEnd());
}
void scaler_4()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Timestamp should remain the same",
                                 time_now, v10item.getCalendarTime());
}
void scaler_5()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Interval divisor remains the same",
                                 uint32_t(2), v10item.getIntervalDivisor());
}
void scaler_6()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Scaler count remains the same",
                                 uint32_t(4), v10item.getScalerCount());
}
void scaler_7()
{
    CPPUNIT_ASSERT_MESSAGE(
                "Scalers remain the same",
                vector<uint32_t>({0, 1, 2, 3}) == v10item.getScalers());
}

}; // end of non-incr scaler tests
CPPUNIT_TEST_SUITE_REGISTRATION(CTransform11p0to10p0Tests_NonIncrScaler);



class CTransform11p0to10p0Tests_IncrScaler : public CppUnit::TestFixture
{
private:
    CTransform11p0to10p0       m_transform;
    NSCLDAQ10::CRingScalerItem v10item;
    NSCLDAQ11::CRingScalerItem v11item;
    std::time_t time_now;

public:
    CPPUNIT_TEST_SUITE(CTransform11p0to10p0Tests_IncrScaler);
    CPPUNIT_TEST(scaler_0);
    CPPUNIT_TEST(scaler_2);
    CPPUNIT_TEST(scaler_3);
    CPPUNIT_TEST(scaler_4);
    CPPUNIT_TEST(scaler_6);
    CPPUNIT_TEST(scaler_7);
    CPPUNIT_TEST_SUITE_END();

public:
    CTransform11p0to10p0Tests_IncrScaler()
        : m_transform(), v10item(1), v11item(1), time_now() {}

    void setUp() {
        using namespace std::chrono;

        // force this be different than now
        time_now = system_clock::to_time_t( system_clock::now() ) + 1;

        m_transform = CTransform11p0to10p0();
        v11item     = NSCLDAQ11::CRingScalerItem(1234, // evt tstamp
                                                 56,   // source id
                                                 78,   // barrier
                                                 14,    // start time
                                                 1,    // stop time
                                                 time_now,  // timestmp
                                                 {0, 1, 2, 3}, // scalers
                                                 2,    // time divisor
                                                 true);  // incremental?


        v10item     = m_transform(v11item);
    }
  void tearDown() {
  }
protected:
void scaler_0() {
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "Incremental scaler becomes INCREMENTAL_SCALER",
                NSCLDAQ10::INCREMENTAL_SCALERS, v10item.type());
}
void scaler_2()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Start time offset remains unchanged",
                                 uint32_t(14), v10item.getStartTime());
}
void scaler_3()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("End time offset remains unchanged",
                                 uint32_t(1), v10item.getEndTime());
}
void scaler_4()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Timestamp should remain the same",
                                 time_now, v10item.getTimestamp());
}
void scaler_6()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Scaler count remains the same",
                                 uint32_t(4), v10item.getScalerCount());
}
void scaler_7()
{
    CPPUNIT_ASSERT_MESSAGE(
                "Scalers remain the same",
                vector<uint32_t>({0, 1, 2, 3}) == v10item.getScalers());
}

}; // end of incremental scaler tests
CPPUNIT_TEST_SUITE_REGISTRATION(CTransform11p0to10p0Tests_IncrScaler);





class CTransform11p0to10p0Tests_State : public CppUnit::TestFixture
{
private:
    CTransform11p0to10p0       m_transform;
    NSCLDAQ10::CRingStateChangeItem v10item;
    NSCLDAQ11::CRingStateChangeItem v11item;
    std::time_t time_now;

public:
    CPPUNIT_TEST_SUITE(CTransform11p0to10p0Tests_State);
    CPPUNIT_TEST(state_0);
    CPPUNIT_TEST(state_1);
    CPPUNIT_TEST(state_2);
    CPPUNIT_TEST(state_3);
    CPPUNIT_TEST(state_4);
    CPPUNIT_TEST(state_5);
    CPPUNIT_TEST(state_6);
    CPPUNIT_TEST(state_7);
    CPPUNIT_TEST_SUITE_END();

public:
    CTransform11p0to10p0Tests_State()
        : m_transform(), v10item(1), v11item(1), time_now() {}

    void setUp() {
        using namespace std::chrono;

        // force this be different than now
        time_now = system_clock::to_time_t( system_clock::now() ) + 1;

        m_transform = CTransform11p0to10p0();
        v11item = NSCLDAQ11::CRingStateChangeItem(987, // tstamp
                                           9, // source id
                                           8, // barrier
                                           NSCLDAQ11::BEGIN_RUN,
                                           42, // run number
                                           1000, // time offset
                                           time_now, // timestamp
                                           "You would like to know"); // title
        v10item     = m_transform(v11item);
    }
  void tearDown() {
  }
protected:
void state_0() {
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "BEGIN_RUN -- > BEGIN_RUN",
                NSCLDAQ10::BEGIN_RUN, v10item.type());
}
void state_1()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Run number remains unchanged",
                                 uint32_t(42), v10item.getRunNumber());
}
void state_2()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Time offset remains unchanged",
                                 uint32_t(1000), v10item.getElapsedTime());
}
void state_3()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Timestamp remains the same",
                                 time_now, v10item.getTimestamp());
}
void state_4()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Title remains the same",
                                 string("You would like to know"),
                                 v10item.getTitle());
}

void state_5()
{
    v11item = NSCLDAQ11::CRingStateChangeItem(NSCLDAQ11::END_RUN);
    v10item = m_transform(v11item);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "END_RUN -- > END_RUN",
                NSCLDAQ10::END_RUN, v10item.type());
}
void state_6()
{
    v11item = NSCLDAQ11::CRingStateChangeItem(NSCLDAQ11::PAUSE_RUN);
    v10item = m_transform(v11item);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "PAUSE_RUN -- > PAUSE_RUN",
                NSCLDAQ10::PAUSE_RUN, v10item.type());
}
void state_7()
{
    v11item = NSCLDAQ11::CRingStateChangeItem(NSCLDAQ11::RESUME_RUN);
    v10item = m_transform(v11item);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "RESUME_RUN -- > RESUME_RUN",
                NSCLDAQ10::RESUME_RUN, v10item.type());
}

}; // end of state tests

CPPUNIT_TEST_SUITE_REGISTRATION(CTransform11p0to10p0Tests_State);

class CTransform11p0to10p0Tests_PhysicsEvent : public CppUnit::TestFixture
{
private:
    CTransform11p0to10p0       m_transform;
    NSCLDAQ10::CPhysicsEventItem v10item;
    NSCLDAQ11::CPhysicsEventItem v11item;
    vector<uint8_t>              data;

public:
    CPPUNIT_TEST_SUITE(CTransform11p0to10p0Tests_PhysicsEvent);
    CPPUNIT_TEST(physicsEvent_0);
    CPPUNIT_TEST(physicsEvent_1);
    CPPUNIT_TEST_SUITE_END();

public:
    CTransform11p0to10p0Tests_PhysicsEvent()
        : m_transform(), v10item(1), v11item(1),data() {}

    void setUp() {
        m_transform = CTransform11p0to10p0();

        data.resize(32);
        iota(data.begin(), data.end(), 0);

        v11item     = NSCLDAQ11::CPhysicsEventItem(12345, 6, 7);
        auto pCursor = reinterpret_cast<uint8_t*>(v11item.getBodyCursor());
        pCursor = std::copy(data.begin(), data.end(), pCursor); 
        v11item.setBodyCursor(pCursor);
        v11item.updateSize();

        v10item     = m_transform(v11item);
    }
  void tearDown() {
  }
protected:
void physicsEvent_0() {
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "PHYSICS_EVENT -- > PHYSICS_EVENT",
                NSCLDAQ10::PHYSICS_EVENT, v10item.type());
}
void physicsEvent_1()
{

  auto pBody = reinterpret_cast<uint8_t*>(v11item.getBodyPointer());
  CPPUNIT_ASSERT_MESSAGE("Body data remains same",
                          std::equal(data.begin(), data.end(), pBody));
}

}; // end of physicsEvent tests

CPPUNIT_TEST_SUITE_REGISTRATION(CTransform11p0to10p0Tests_PhysicsEvent);

class CTransform11p0to10p0Tests_EventCount : public CppUnit::TestFixture
{
private:
    CTransform11p0to10p0       m_transform;
    NSCLDAQ10::CRingPhysicsEventCountItem v10item;
    NSCLDAQ11::CRingPhysicsEventCountItem v11item;
    std::time_t time_now;

public:
    CPPUNIT_TEST_SUITE(CTransform11p0to10p0Tests_EventCount);
    CPPUNIT_TEST(eventCount_0);
    CPPUNIT_TEST(eventCount_1);
    CPPUNIT_TEST(eventCount_2);
    CPPUNIT_TEST(eventCount_3);
    CPPUNIT_TEST_SUITE_END();

public:
    CTransform11p0to10p0Tests_EventCount()
        : m_transform(), v10item(), v11item(), time_now() {}

    void setUp() {

        using namespace std::chrono;

        // force this be different than now
        time_now = system_clock::to_time_t( system_clock::now() ) + 1;

        m_transform = CTransform11p0to10p0();

        v11item     = NSCLDAQ11::CRingPhysicsEventCountItem(12345, 6, 7, 8, 9, time_now, 11);

        v10item     = m_transform(v11item);
    }
  void tearDown() {
  }
protected:
void eventCount_0() {
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "PHYSICS_EVENT_COUNT -- > PHYSICS_EVENT_COUNT",
                NSCLDAQ10::PHYSICS_EVENT_COUNT, v10item.type());
}
void eventCount_1()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Elapsed time remains same",
                                uint32_t(9), v10item.getTimeOffset());
}
void eventCount_2()
{

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Event count remains same",
                                uint64_t(8), v10item.getEventCount());
}
void eventCount_3()
{

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Unix timestamp remains same",
                                time_now, v10item.getTimestamp());
}

}; // end of eventCount tests


class CTransform11p0to10p0Tests_Fragment : public CppUnit::TestFixture
{
private:
    CTransform11p0to10p0       m_transform;
    NSCLDAQ10::CRingFragmentItem v10item;
    NSCLDAQ11::CRingFragmentItem v11item;
    vector<uint8_t>              data;

public:
    CPPUNIT_TEST_SUITE(CTransform11p0to10p0Tests_Fragment);
    CPPUNIT_TEST(Fragment_0);
    CPPUNIT_TEST(Fragment_1);
    CPPUNIT_TEST(Fragment_2);
    CPPUNIT_TEST(Fragment_3);
    CPPUNIT_TEST(Fragment_4);
    CPPUNIT_TEST_SUITE_END();

public:
    CTransform11p0to10p0Tests_Fragment()
        : m_transform(), v10item(1, 2, 0, nullptr, 4),
          v11item(1, 2, 0, nullptr, 4), data() {}

    void setUp() {

        data.resize(128);
        std::iota(data.begin(), data.end(), 0);
        m_transform = CTransform11p0to10p0();

        v11item     = NSCLDAQ11::CRingFragmentItem(12345, 6, data.size(),
                                                  data.data(), 23);

        v10item     = m_transform(v11item);
    }
  void tearDown() {
  }
protected:
void Fragment_0() {
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "EVB_FRAGMENT -- > EVB_FRAGMENT",
                NSCLDAQ10::EVB_FRAGMENT, v10item.type());
}
void Fragment_1()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Timestamp in body header is retained",
                                uint64_t(12345), v10item.timestamp());
}
void Fragment_2()
{

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Source id in body header is retained",
                                uint32_t(6), v10item.source());
}
void Fragment_3()
{

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Barrier type is retained",
                               uint32_t(23), v10item.barrierType());
}
void Fragment_4()
{
  auto pBody10 = reinterpret_cast<const uint8_t*>(v10item.payloadPointer());

  CPPUNIT_ASSERT_MESSAGE("Payload remains the same",
                         std::equal(data.begin(), data.end(), pBody10));
}


}; // end of Fragment tests

CPPUNIT_TEST_SUITE_REGISTRATION(CTransform11p0to10p0Tests_Fragment);

class CTransform11p0to10p0Tests_UnknownFragment : public CppUnit::TestFixture
{
private:
    CTransform11p0to10p0       m_transform;
    NSCLDAQ10::CUnknownFragment v10item;
    NSCLDAQ11::CUnknownFragment v11item;
    vector<uint8_t>              data;

public:
    CPPUNIT_TEST_SUITE(CTransform11p0to10p0Tests_UnknownFragment);
    CPPUNIT_TEST(UFragment_0);
    CPPUNIT_TEST(UFragment_1);
    CPPUNIT_TEST(UFragment_2);
    CPPUNIT_TEST(UFragment_3);
    CPPUNIT_TEST(UFragment_4);
    CPPUNIT_TEST_SUITE_END();

public:
    CTransform11p0to10p0Tests_UnknownFragment()
        : m_transform(), v10item(1, 2, 4, 0, nullptr),
          v11item(1, 2, 4, 0, nullptr), data() {}

    void setUp() {

        data.resize(128);
        std::iota(data.begin(), data.end(), 0);
        m_transform = CTransform11p0to10p0();

        v11item     = NSCLDAQ11::CUnknownFragment(12345, 6, 23, data.size(),
                                                  data.data());

        v10item     = m_transform(v11item);
    }
  void tearDown() {
  }
protected:
void UFragment_0() {
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "EVB_UNKNOWN_PAYLOAD -- > EVB_UNKNOWN_PAYLOAD",
                NSCLDAQ10::EVB_UNKNOWN_PAYLOAD, v10item.type());
}
void UFragment_1()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Timestamp in body header is retained",
                                uint64_t(12345), v10item.timestamp());
}
void UFragment_2()
{

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Source id in body header is retained",
                                uint32_t(6), v10item.source());
}
void UFragment_3()
{

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Barrier type is retained",
                               uint32_t(23), v10item.barrierType());
}
void UFragment_4()
{
  auto pBody10 = reinterpret_cast<const uint8_t*>(v10item.payloadPointer());

  CPPUNIT_ASSERT_MESSAGE("Payload remains the same",
                         std::equal(data.begin(), data.end(), pBody10));
}


}; // end of Fragment tests

CPPUNIT_TEST_SUITE_REGISTRATION(CTransform11p0to10p0Tests_UnknownFragment);

class CTransform11p0to10p0Tests_Text : public CppUnit::TestFixture
{
private:
    CTransform11p0to10p0       m_transform;
    NSCLDAQ10::CRingTextItem   v10item;
    NSCLDAQ11::CRingTextItem   v11item;
    std::time_t                time_now;
    std::vector<std::string>   strings;

public:
    CPPUNIT_TEST_SUITE(CTransform11p0to10p0Tests_Text);
    CPPUNIT_TEST(Text_0);
    CPPUNIT_TEST(Text_1);
    CPPUNIT_TEST(Text_2);
    CPPUNIT_TEST(Text_3);
    CPPUNIT_TEST(Text_4);
    CPPUNIT_TEST_SUITE_END();

public:
    CTransform11p0to10p0Tests_Text()
        : m_transform(),
          v10item(NSCLDAQ10::PACKET_TYPES, {}),
          v11item(NSCLDAQ11::PACKET_TYPES, {}),
          time_now(),
          strings() {}

    void setUp() {

        using namespace std::chrono;
        time_now = system_clock::to_time_t(system_clock::now())+1;

        strings = {"the", "test", "strings"};

        m_transform = CTransform11p0to10p0();

        v11item     = NSCLDAQ11::CRingTextItem(NSCLDAQ11::PACKET_TYPES,
                                               12345, 6, 23,
                                               strings, 56, time_now,
                                               3);

        v10item     = m_transform(v11item);
    }
  void tearDown() {
  }
protected:
void Text_0() {
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "PACKET_TYPES --> PACKET_TYPES",
                NSCLDAQ10::PACKET_TYPES, v10item.type());
}
void Text_1()
{
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Time offset remains the same",
                                uint32_t(56), v10item.getTimeOffset());
}
void Text_2()
{

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Unix timestamp remains the same",
                                time_now, v10item.getTimestamp());
}
void Text_3()
{

  CPPUNIT_ASSERT_EQUAL_MESSAGE("String count remains unchanged",
                               uint32_t(strings.size()),
                               v10item.getStringCount());
}
void Text_4()
{
  CPPUNIT_ASSERT_MESSAGE("Strings transform unchanged",
                         strings == v10item.getStrings());

}


}; // end of Fragment tests

CPPUNIT_TEST_SUITE_REGISTRATION(CTransform11p0to10p0Tests_Text);


class CTransform11p0to10p0Tests_General : public CppUnit::TestFixture
{
private:
    CTransform11p0to10p0       m_transform;

public:
    CPPUNIT_TEST_SUITE(CTransform11p0to10p0Tests_General);
    CPPUNIT_TEST(Test_0);
    CPPUNIT_TEST(Test_1);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {
        m_transform = CTransform11p0to10p0();
    }
  void tearDown() {}
protected:
void Test_0() {
    NSCLDAQ11::CDataFormatItem item;
    CPPUNIT_ASSERT_THROW_MESSAGE( "Data format item do not convert",
                                  m_transform(item), std::runtime_error );
}
void Test_1() {
    NSCLDAQ11::CRingItem item(NSCLDAQ11::EVB_GLOM_INFO);
    CPPUNIT_ASSERT_THROW_MESSAGE( "Glom info items do not convert",
                                  m_transform(item), std::runtime_error );
}

}; // end of Fragment tests

CPPUNIT_TEST_SUITE_REGISTRATION(CTransform11p0to10p0Tests_General);

