
#include <cppunit/Asserter.h>
#include <cppunit/extensions/HelperMacros.h>
#include "Asserts.h"

#include <V10/CRingStateChangeItem.h>
#include <V10/CPhysicsEventItem.h>
#include <V10/CRingTextItem.h>
#include <V10/DataFormatV10.h>

#include <V8/CRawBuffer.h>
#include <V8/DataFormatV8.h>
#include <V8/format_cast.h>
#include <V8/ChangeBufferSize.h>

#include <BufferIOV8.h>

#include <DebugUtils.h>
#include <CTestSourceSink.h>

#define private public
#define protected public
#include <C10p0to8p0Mediator.h>
#undef protected
#undef private

#include <iterator>
#include <algorithm>
#include <ctime>
#include <chrono>

using namespace std;

using namespace DAQ;



class C10p0to8p0MediatorTests_PhysEventFlush : public CppUnit::TestFixture
{
private:
  Transform::C10p0to8p0Mediator m_mediator;

public:
    CPPUNIT_TEST_SUITE(C10p0to8p0MediatorTests_PhysEventFlush);
    CPPUNIT_TEST(physEventFlush_0);
    CPPUNIT_TEST(physEventFlush_1);
    CPPUNIT_TEST(physEventFlush_2);
    CPPUNIT_TEST(physEventFlush_3);
    CPPUNIT_TEST(physEventFlush_4);
    CPPUNIT_TEST_SUITE_END();

public:

    void setUp() {

      V8::gBufferSize = 8192;

      std::unique_ptr<CDataSource> pSource(new CTestSourceSink);
      m_mediator.setDataSource(pSource);

      std::unique_ptr<CDataSink> pSink(new CTestSourceSink);
      m_mediator.setDataSink(pSink);

    }

    void tearDown() {
    }

public:
  void physEventFlush_0() {
    V10::CPhysicsEventItem item(V10::PHYSICS_EVENT);
    item.fillBody(std::vector<std::uint16_t>({2,0}));

    auto pSource = dynamic_cast<CTestSourceSink*>(m_mediator.getDataSource());
    auto pSink = dynamic_cast<CTestSourceSink*>(m_mediator.getDataSink());

    *pSource << item;

    bool good = m_mediator.processOne();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("First physics event with no need to flush returns VOID",
                                 std::size_t(0), pSink->getBuffer().size());
  }

void physEventFlush_1() {

  V10::CRingStateChangeItem begin;
  V10::CPhysicsEventItem item(V10::PHYSICS_EVENT);
  item.fillBody(std::vector<std::uint16_t>({2,0}));

  auto pSource = dynamic_cast<CTestSourceSink*>(m_mediator.getDataSource());
  auto pSink = dynamic_cast<CTestSourceSink*>(m_mediator.getDataSink());

  *pSource << begin;

  bool good = m_mediator.processOne();

  V8::CRawBuffer returnedBuffer;
  *pSink >> returnedBuffer;

  CPPUNIT_ASSERT_EQUAL_MESSAGE("First begin run gets emitted without a flush",
                               std::uint16_t(V8::BEGRUNBF),
                               returnedBuffer.getHeader().type);
}

  void physEventFlush_2() {

    V10::CRingStateChangeItem begin;
    V10::CPhysicsEventItem event(V10::PHYSICS_EVENT);
    event.fillBody(std::vector<std::uint16_t>({2,0}));

    auto pSource = dynamic_cast<CTestSourceSink*>(m_mediator.getDataSource());
    auto pSink = dynamic_cast<CTestSourceSink*>(m_mediator.getDataSink());

    *pSource << event;
    bool good = m_mediator.processOne();

    *pSource << begin;
    good = m_mediator.processOne();

    V8::CRawBuffer returnedBuffer;
    *pSink >> returnedBuffer;

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Event buffer should flush before state change if data is present",
        std::uint16_t(V8::DATABF),
        returnedBuffer.getHeader().type);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Flushed event buffer should contain only data present since last flush",
        std::uint16_t(1),
        returnedBuffer.getHeader().nevt);

  }

      void physEventFlush_3() {

        // Ensure that the sequence numbers make sense.

        V8::Test::ChangeBufferSize forScope(132);

        V10::CRingStateChangeItem begin;
        V10::CPhysicsEventItem event(V10::PHYSICS_EVENT);
        std::vector<std::uint16_t> bodyData(50);
        std::iota(bodyData.begin(), bodyData.end(), 0);
        bodyData.at(0) = 50;
        event.fillBody(bodyData);

        auto pSource = dynamic_cast<CTestSourceSink*>(m_mediator.getDataSource());
        auto pSink = dynamic_cast<CTestSourceSink*>(m_mediator.getDataSink());

        *pSource << begin;
        *pSource << event;
        *pSource << event;
        bool good = m_mediator.processOne();
        good = m_mediator.processOne();
        good = m_mediator.processOne();

        V8::CRawBuffer returnedBuffer;
        *pSink >> returnedBuffer;

        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "First control buffer should have sequence 0",
            std::uint32_t(0),
            returnedBuffer.getHeader().seq);

        *pSink >> returnedBuffer;

        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "First physics event buffer should have sequence 0",
            std::uint32_t(0),
            returnedBuffer.getHeader().seq);

        *pSink >> returnedBuffer;

        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "Second physics event buffer should have sequence 1",
            std::uint32_t(1),
            returnedBuffer.getHeader().seq);

      }

      void physEventFlush_4() {

        // Ensure that the sequence numbers make sense when multiple events are in the queue.

        V8::Test::ChangeBufferSize forScope(232);

        V10::CRingStateChangeItem begin;
        V10::CRingStateChangeItem end(V10::END_RUN);
        V10::CPhysicsEventItem event(V10::PHYSICS_EVENT);
        std::vector<std::uint16_t> bodyData(50);
        std::iota(bodyData.begin(), bodyData.end(), 0);
        bodyData.at(0) = 50;
        event.fillBody(bodyData);

        auto pSource = dynamic_cast<CTestSourceSink*>(m_mediator.getDataSource());
        auto pSink = dynamic_cast<CTestSourceSink*>(m_mediator.getDataSink());

        // Load the source with data
        *pSource << begin;
        *pSource << event;
        *pSource << event;
        *pSource << event;
        *pSource << end;

        // process the data with the transform
        bool good = m_mediator.processOne();
        good = m_mediator.processOne();
        good = m_mediator.processOne();
        good = m_mediator.processOne();
        good = m_mediator.processOne();

        // read the output data from the sink
        V8::CRawBuffer returnedBuffer, lastEvtBuffer;
        *pSink >> returnedBuffer; // begin
        *pSink >> returnedBuffer; // event
        *pSink >> lastEvtBuffer; // event - flushed by begin
        *pSink >> returnedBuffer; // begin

        // make sure that the sequence numbers are what we expect
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Second physics buffer should be sequence = 2",
            std::uint32_t(2),
            lastEvtBuffer.getHeader().seq);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("control buffer sequence should match number of physics events",
            std::uint32_t(3),
            returnedBuffer.getHeader().seq);

      }

};

CPPUNIT_TEST_SUITE_REGISTRATION(C10p0to8p0MediatorTests_PhysEventFlush);



class C10p0to8p0MediatorTests_TextFlush : public CppUnit::TestFixture
{
private:
  Transform::C10p0to8p0Mediator m_mediator;

public:
    CPPUNIT_TEST_SUITE(C10p0to8p0MediatorTests_TextFlush);
    CPPUNIT_TEST(TextFlush_0);
    CPPUNIT_TEST_SUITE_END();

public:

    void setUp() {

      V8::gBufferSize = 43; // just big enough to fit two 3-letter words each buffer
      std::unique_ptr<CDataSource> pSource(new CTestSourceSink);
      m_mediator.setDataSource(pSource);

      std::unique_ptr<CDataSink> pSink(new CTestSourceSink);
      m_mediator.setDataSink(pSink);

    }

    void tearDown() {
      V8::gBufferSize = 8192;
    }

public:
  void TextFlush_0() {
    std::vector<std::string> m_strings = {"why", "did", "the","cat", "nap"};

    std::time_t tstamp = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() );

    auto v10item = V10::CRingTextItem(V10::MONITORED_VARIABLES,
                                            m_strings, 0x12345678, tstamp);

    auto pSource = dynamic_cast<CTestSourceSink*>(m_mediator.getDataSource());
    auto pSink = dynamic_cast<CTestSourceSink*>(m_mediator.getDataSink());

    *pSource << v10item;

    bool good = m_mediator.processOne();

    V8::CRawBuffer buffer0, buffer1, buffer2;

    // if there is insufficient data to read from the mediator, then an exception will be thrown
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("First buffer is present with data",
                                    *pSink >> buffer0);
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("Second buffer is present with data",
                                    *pSink >> buffer1);
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("Third buffer is present with data",
                                    *pSink >> buffer2);

  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(C10p0to8p0MediatorTests_TextFlush);



