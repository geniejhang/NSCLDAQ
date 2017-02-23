
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
#include <CCompositePredicate.h>
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


class CActionableTestPredicate : public DAQ::Transform::CTestPredicate {

private:
    CPredicatedMediator::Action m_action;

public:
    CActionableTestPredicate(CPredicatedMediator::Action action)
        : CTestPredicate("action"), m_action(action) {}

    CPredicatedMediator::Action preInputUpdate(CPredicatedMediator& med)
    {
        CTestPredicate::preInputUpdate(med);
        return m_action;
    }

    CPredicatedMediator::Action postInputUpdate(CPredicatedMediator& med, int type)
    {
        CTestPredicate::postInputUpdate(med, type);
        return m_action;
    }

    CPredicatedMediator::Action preOutputUpdate(CPredicatedMediator& med, int type)
    {
        CTestPredicate::preOutputUpdate(med, type);
        return m_action;
    }

    CPredicatedMediator::Action postOutputUpdate(CPredicatedMediator& med, int type)
    {
        CTestPredicate::postOutputUpdate(med, type);
        return m_action;
    }
};

/*!
 * \brief The C10p0to8p0MediatorTests_PhysEventFlush class
 *
 * Here we are testing whether or not the mediator behaves properly
 * concerning buffering physics events and then flushing them when
 * necessary
 */
class CCompositePredicateTests : public CppUnit::TestFixture
{

public:
    CPPUNIT_TEST_SUITE(CCompositePredicateTests);
    CPPUNIT_TEST(preInputUpdate_0);
    CPPUNIT_TEST(preInputUpdate_1);
    CPPUNIT_TEST(preInputUpdate_2);
    CPPUNIT_TEST(postInputUpdate_0);
    CPPUNIT_TEST(preOutputUpdate_0);
    CPPUNIT_TEST(postOutputUpdate_0);
    CPPUNIT_TEST(reset_0);
    CPPUNIT_TEST_SUITE_END();

private:
    std::shared_ptr<CPredicatedMediator> m_pMediator;
    CCompositePredicate m_predicate;
    std::shared_ptr<CTestPredicate> m_pPred0;
    std::shared_ptr<CTestPredicate> m_pPred1;

public:

    void setUp() {
        m_predicate = CCompositePredicate();
        m_pPred0 = std::shared_ptr<CTestPredicate>(new CTestPredicate("pred0"));
        m_pPred1 = std::shared_ptr<CTestPredicate>(new CTestPredicate("pred1"));
        m_predicate.addPredicate(m_pPred0);
        m_predicate.addPredicate(m_pPred1);

        m_pMediator = std::shared_ptr<CPredicatedMediator>(new CDumbMediator);
    }

    void tearDown() {
    }

public:
    void preInputUpdate_0() {
        m_predicate.preInputUpdate(*m_pMediator);

        EQMSG("pred0", vector<string>({"pred0","preInputUpdate"}), m_pPred0->getLog());
        EQMSG("pred1", vector<string>({"pred1","preInputUpdate"}), m_pPred1->getLog());
    }

    void preInputUpdate_1() {

        m_predicate.getPredicates().clear();

        std::shared_ptr<CTestPredicate> pPred(new CActionableTestPredicate(CPredicatedMediator::ABORT));
        m_predicate.addPredicate(pPred);
        m_predicate.addPredicate(m_pPred0);

        auto action = m_predicate.preInputUpdate(*m_pMediator);

        // abort causes the subsequent predicates to get skipped
        EQMSG("action", CPredicatedMediator::ABORT, action);
        EQMSG("abort", vector<string>({"action","preInputUpdate"}), pPred->getLog());
        EQMSG("pred0", vector<string>({"pred0"}), m_pPred0->getLog());
    }

    void preInputUpdate_2() {

        m_predicate.getPredicates().clear();

        std::shared_ptr<CTestPredicate> pPred(new CActionableTestPredicate(CPredicatedMediator::SKIP));
        m_predicate.addPredicate(pPred);
        m_predicate.addPredicate(m_pPred0);

        auto action = m_predicate.preInputUpdate(*m_pMediator);

        // abort causes the subsequent predicates to get skipped
        EQMSG("action", CPredicatedMediator::SKIP, action);
        EQMSG("skip", vector<string>({"action","preInputUpdate"}), pPred->getLog());
        EQMSG("pred0", vector<string>({"pred0","predInputUpdate"}), m_pPred0->getLog());
    }


    void postInputUpdate_0() {
        m_predicate.postInputUpdate(*m_pMediator, 1);

        EQMSG("pred0", vector<string>({"pred0","postInputUpdate:1"}), m_pPred0->getLog());
        EQMSG("pred1", vector<string>({"pred1","postInputUpdate:1"}), m_pPred1->getLog());
    }

    void postInputUpdate_1() {

        m_predicate.getPredicates().clear();

        std::shared_ptr<CTestPredicate> pPred(new CActionableTestPredicate(CPredicatedMediator::ABORT));
        m_predicate.addPredicate(pPred);
        m_predicate.addPredicate(m_pPred0);

        auto action = m_predicate.postInputUpdate(*m_pMediator, 1);

        // abort causes the subsequent predicates to get skipped
        EQMSG("action", CPredicatedMediator::ABORT, action);
        EQMSG("abort", vector<string>({"action","preInputUpdate:1"}), pPred->getLog());
        EQMSG("pred0", vector<string>({"pred0"}), m_pPred0->getLog());
    }

    void postInputUpdate_2() {

        m_predicate.getPredicates().clear();

        std::shared_ptr<CTestPredicate> pPred(new CActionableTestPredicate(CPredicatedMediator::SKIP));
        m_predicate.addPredicate(pPred);
        m_predicate.addPredicate(m_pPred0);

        auto action = m_predicate.preInputUpdate(*m_pMediator);

        // abort causes the subsequent predicates to get skipped
        EQMSG("action", CPredicatedMediator::SKIP, action);
        EQMSG("skip", vector<string>({"action","preInputUpdate"}), pPred->getLog());
        EQMSG("pred0", vector<string>({"pred0","predInputUpdate"}), m_pPred0->getLog());
    }




    void preOutputUpdate_0() {
        m_predicate.preOutputUpdate(*m_pMediator, 2);

        EQMSG("pred0", vector<string>({"pred0","preOutputUpdate:2"}), m_pPred0->getLog());
        EQMSG("pred1", vector<string>({"pred1","preOutputUpdate:2"}), m_pPred1->getLog());
    }


    void postOutputUpdate_0() {
        m_predicate.postOutputUpdate(*m_pMediator, 2);

        EQMSG("pred0", vector<string>({"pred0","postOutputUpdate:2"}), m_pPred0->getLog());
        EQMSG("pred1", vector<string>({"pred1","postOutputUpdate:2"}), m_pPred1->getLog());

    }

    void reset_0() {
        m_predicate.reset();

        EQMSG("pred0", vector<string>({"pred0","reset"}), m_pPred0->getLog());
        EQMSG("pred1", vector<string>({"pred1","reset"}), m_pPred1->getLog());

    }
};
CPPUNIT_TEST_SUITE_REGISTRATION(CCompositePredicateTests);



