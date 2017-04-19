


#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>

#include <Asserts.h>

#include <CGlom.h>
#include <V12/CRingStateChangeItem.h>
#include <V12/CPhysicsEventItem.h>
#include <V12/CCompositeRingItem.h>
#include <V12/CRingScalerItem.h>

#include <RingIOV12.h>

#include <CTestSourceSink.h>

#include <CTimeout.h>

using namespace DAQ;

class CGlomTests : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CGlomTests);
    CPPUNIT_TEST(firstBarrier_0);
    CPPUNIT_TEST(firstBarrier_1);
    CPPUNIT_TEST(firstBarrier_2);
    CPPUNIT_TEST(accumulate_0);
    CPPUNIT_TEST(accumulate_1);
    CPPUNIT_TEST(accumulate_2);
    CPPUNIT_TEST_SUITE_END();

    CTestSourceSinkPtr m_pSink;

public:
    void setUp() {
        m_pSink = std::make_shared<CTestSourceSink>();
    }

    void tearDown() {}

    void firstBarrier_0()
    {
        auto pBegin = std::make_shared<V12::CRingStateChangeItem>(V12::BEGIN_RUN);

        {
            CGlom glommer(m_pSink);

            glommer.handleItem(pBegin);
        }

        V12::CRawRingItem item;
        readItem(*m_pSink, item);

        EQMSG("first barrier causes glom parameter emission",
              V12::EVB_GLOM_INFO, item.type());

        readItem(*m_pSink, item);
        EQMSG("begin still gets outputted after glom parameters",
              V12::COMP_BEGIN_RUN, item.type());


    }


    void firstBarrier_1() {
        auto pBegin = std::make_shared<V12::CRingStateChangeItem>(V12::BEGIN_RUN);

        {
            CGlom glommer(m_pSink);
            glommer.setFirstBarrier(false);

            glommer.handleItem(pBegin);
        }

        V12::CRawRingItem item;
        readItem(*m_pSink, item);
        EQMSG("begin outputted without glom info if not waiting for first barrier",
              V12::COMP_BEGIN_RUN, item.type());

    }

    void firstBarrier_2() {
        auto pItem = std::make_shared<V12::CPhysicsEventItem>();

        {
            CGlom glommer(m_pSink);

            glommer.handleItem(pItem);
        }

        V12::CRawRingItem item;
        readItem(*m_pSink, item);
        EQMSG("non-barriers outputted without glom info when waiting for first barrier",
              V12::COMP_PHYSICS_EVENT, item.type());

    }


    void accumulate_0() {

        auto pItem0 = std::make_shared<V12::CPhysicsEventItem>(1, 2);
        auto pItem1 = std::make_shared<V12::CPhysicsEventItem>(3, 2);
        auto pItem2 = std::make_shared<V12::CPhysicsEventItem>(4, 2);

        {
            CGlom glommer(m_pSink);
            glommer.disableBuilding(false);
            glommer.setCorrelationTime(2);

            glommer.handleItem(pItem0);
            glommer.handleItem(pItem1);
            glommer.handleItem(pItem2);
        }

        V12::CRawRingItem item;
        readItem(*m_pSink, item);

        V12::CCompositeRingItem comp(item);
        EQMSG("type of first outputted item", V12::COMP_PHYSICS_EVENT, item.type());
        EQMSG("number of children", size_t(2), comp.count());
        EQMSG("Timestamp", uint64_t(1), comp.getEventTimestamp());

        readItem(*m_pSink, item, CTimeout( std::chrono::seconds(1) ));
        EQMSG("Read a second event", false, m_pSink->eof());
        EQMSG("Timestamp of second event",
              uint64_t(4), item.getEventTimestamp());

    }

    // test even though an item arrives within the proper correlation window,
    // it will not correlate if it is a different type
    void accumulate_1() {

        auto pItem0 = std::make_shared<V12::CPhysicsEventItem>(1, 2);
        auto pItem1 = std::make_shared<V12::CPhysicsEventItem>(2, 2);
        auto pItem2 = std::make_shared<V12::CRingScalerItem>(3, 2, 0, 0, 0,
                                                             std::vector<uint32_t>(), 1, true, 32);

        {
            CGlom glommer(m_pSink);
            glommer.disableBuilding(false);
            glommer.setCorrelationTime(5);

            glommer.handleItem(pItem0);
            glommer.handleItem(pItem1);
            glommer.handleItem(pItem2);
        }

        V12::CRawRingItem item;
        readItem(*m_pSink, item);

        V12::CCompositeRingItem comp(item);
        EQMSG("type of first outputted item", V12::COMP_PHYSICS_EVENT, item.type());
        EQMSG("number of children", size_t(2), comp.count());
        EQMSG("Timestamp", uint64_t(1), comp.getEventTimestamp());

        readItem(*m_pSink, item, CTimeout( std::chrono::seconds(1) ));
        EQMSG("Read a second event", false, m_pSink->eof());
        EQMSG("Timestamp of second event",
              uint64_t(3), item.getEventTimestamp());
        EQMSG("Second item is different", V12::COMP_PERIODIC_SCALERS, item.type());

    }


    // test that composite and normal types build together
    void accumulate_2() {

        auto pItem0 = std::make_shared<V12::CPhysicsEventItem>(1, 2);
        auto pItem1 = std::make_shared<V12::CCompositeRingItem>();
        pItem1->setType(V12::COMP_PHYSICS_EVENT);
        pItem1->setEventTimestamp(2);

        {
            CGlom glommer(m_pSink);
            glommer.disableBuilding(false);
            glommer.setCorrelationTime(5);

            glommer.handleItem(pItem0);
            glommer.handleItem(pItem1);
        }

        V12::CRawRingItem item;
        readItem(*m_pSink, item);

        V12::CCompositeRingItem comp(item);
        EQMSG("type of first outputted item", V12::COMP_PHYSICS_EVENT, item.type());
        EQMSG("number of children", size_t(2), comp.count());
        EQMSG("Timestamp", uint64_t(1), comp.getEventTimestamp());

    }


};

CPPUNIT_TEST_SUITE_REGISTRATION(CGlomTests);
