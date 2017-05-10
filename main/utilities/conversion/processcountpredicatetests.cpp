

/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

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

#include <DebugUtils.h>

#include <CPredicatedMediator.h>
#include <CProcessCountPredicate.h>
#include <CTestPredicate.h>

#include <memory>

using namespace std;
using namespace DAQ::Transform;


class CDumbMediator : public DAQ::Transform::CPredicatedMediator
{
public:
    CDumbMediator() : CPredicatedMediator() {}
    virtual void mainLoop() {};
    virtual void initialize() {};
    virtual void finalize() {};
    virtual std::shared_ptr<CPredicate> getPredicate() { return nullptr; }
    virtual void setPredicate(std::shared_ptr<CPredicate>) {}
};

/*!
 * \brief The C10p0to8p0MediatorTests_PhysEventFlush class
 *
 * Here we are testing whether or not the mediator behaves properly
 * concerning buffering physics events and then flushing them when
 * necessary
 */
class CProcessCountPredicateTests : public CppUnit::TestFixture
{

public:
    CPPUNIT_TEST_SUITE(CProcessCountPredicateTests);
    CPPUNIT_TEST(preInputUpdate_0);
    CPPUNIT_TEST(postInputUpdate_0);
    CPPUNIT_TEST(postInputUpdate_1);
    CPPUNIT_TEST(postInputUpdate_2);
    CPPUNIT_TEST(postInputUpdate_3);
    CPPUNIT_TEST(preOutputUpdate_0);
    CPPUNIT_TEST(postOutputUpdate_0);
    CPPUNIT_TEST(reset_0);
    CPPUNIT_TEST_SUITE_END();

private:
    std::shared_ptr<CPredicatedMediator> m_pMediator;
    CProcessCountPredicate m_predicate;
public:
    CProcessCountPredicateTests() :
        m_pMediator(), m_predicate(0, 1)
    {}

    void setUp() {
        m_predicate = CProcessCountPredicate(0, 1);

        m_pMediator = std::shared_ptr<CPredicatedMediator>(new CDumbMediator);
    }

    void tearDown() {
    }

public:
    void preInputUpdate_0() {
        EQMSG("preInputUpdate always returns continue",
              CPredicatedMediator::CONTINUE,
              m_predicate.preInputUpdate(*m_pMediator));
    }


    void postInputUpdate_0() {
        m_predicate.setNumberToSkip(1);

        auto result = m_predicate.postInputUpdate(*m_pMediator, 1);
        EQMSG("first processed item w/ skip count=1 should skip",
              CPredicatedMediator::SKIP,
              result);

    }

    void postInputUpdate_1() {
        m_predicate.setNumberToSkip(0);
        m_predicate.setNumberToProcess(1);

        auto result = m_predicate.postInputUpdate(*m_pMediator, 1);
        EQMSG("first processed item with skip count=0 & proc count=1 should continue",
              CPredicatedMediator::CONTINUE,
              result);

    }

    void postInputUpdate_2() {
        m_predicate.setNumberToSkip(1);
        m_predicate.setNumberToProcess(1);

        auto result0 = m_predicate.postInputUpdate(*m_pMediator, 1);
        auto result1 = m_predicate.postInputUpdate(*m_pMediator, 1);

        EQMSG("first processed item with skip count=1 & proc count=1 should skip",
              CPredicatedMediator::SKIP,
              result0);

        EQMSG("second processed item with skip count=1 & proc count=1 should continue",
              CPredicatedMediator::CONTINUE,
              result1);

    }

    void postInputUpdate_3() {
        m_predicate.setNumberToSkip(0);
        m_predicate.setNumberToProcess(1);

        auto result0 = m_predicate.postInputUpdate(*m_pMediator, 1);
        auto result1 = m_predicate.postInputUpdate(*m_pMediator, 1);

        EQMSG("first processed item with skip count=0 & proc count=1 should continue",
              CPredicatedMediator::CONTINUE,
              result0);

        EQMSG("second processed item with skip count=0 & proc count=1 should abort",
              CPredicatedMediator::ABORT,
              result1);

    }


    void preOutputUpdate_0() {
        auto result = m_predicate.preOutputUpdate(*m_pMediator, 2);

        EQMSG("preOutputUpdate should always continue",
              CPredicatedMediator::CONTINUE,
              result);
    }


    void postOutputUpdate_0() {
        auto result = m_predicate.postOutputUpdate(*m_pMediator, 2);

        EQMSG("postOutputUpdate should always continue",
              CPredicatedMediator::CONTINUE,
              result);
    }

    void reset_0() {

        m_predicate.setSkipCount(123);
        m_predicate.setProcessCount(321);

        EQMSG("setSkipCount worked", size_t(123), m_predicate.getSkipCount());
        EQMSG("setProcessCount worked", size_t(321), m_predicate.getProcessCount());

        m_predicate.reset();

        EQMSG("reset affected the skip count", size_t(0), m_predicate.getSkipCount());
        EQMSG("reset affected the process count", size_t(0), m_predicate.getProcessCount());
    }
};
CPPUNIT_TEST_SUITE_REGISTRATION(CProcessCountPredicateTests);



