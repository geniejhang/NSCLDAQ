
#include <cppunit/Asserter.h>
#include <cppunit/extensions/HelperMacros.h>
#include "Asserts.h"

#include <NSCLDAQ10/CRingStateChangeItem.h>
#include <NSCLDAQ10/CPhysicsEventItem.h>
#include <NSCLDAQ10/DataFormatV10.h>

#include <NSCLDAQ8/CRawBuffer.h>
#include <NSCLDAQ8/DataFormatV8.h>
#include <NSCLDAQ8/format_cast.h>

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
    NSCLDAQ10::CPhysicsEventItem item(NSCLDAQ10::PHYSICS_EVENT);
    item.fillBody(std::vector<std::uint16_t>({2,0}));

    auto pSource = dynamic_cast<CTestSourceSink*>(m_mediator.getDataSource());
    auto pSink = dynamic_cast<CTestSourceSink*>(m_mediator.getDataSink());

    *pSource << item;

    bool good = m_mediator.processOne();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("First physics event with no need to flush returns VOID",
                                 std::size_t(0), pSink->getBuffer().size());
  }

void physEventFlush_1() {

  NSCLDAQ10::CRingStateChangeItem begin;
  NSCLDAQ10::CPhysicsEventItem item(NSCLDAQ10::PHYSICS_EVENT);
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

    NSCLDAQ10::CRingStateChangeItem begin;
    NSCLDAQ10::CPhysicsEventItem event(NSCLDAQ10::PHYSICS_EVENT);
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

};

CPPUNIT_TEST_SUITE_REGISTRATION(C10p0to8p0MediatorTests_PhysEventFlush);



