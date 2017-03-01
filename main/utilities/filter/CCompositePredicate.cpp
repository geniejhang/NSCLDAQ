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
#include <CCompositePredicate.h>

using namespace std;

namespace DAQ {
namespace Transform {

void CCompositePredicate::addPredicate(std::shared_ptr<CPredicate> pPred)
{
    m_predicates.push_back(pPred);
}

std::vector<std::shared_ptr<CPredicate>>& CCompositePredicate::getPredicates()
{
    return m_predicates;
}

CPredicatedMediator::Action
CCompositePredicate::preInputUpdate(CPredicatedMediator& mediator)
{
    CPredicatedMediator::Action action = CPredicatedMediator::CONTINUE;
    auto currentAction = action;

    for (auto pPred : m_predicates) {
        currentAction = pPred->preInputUpdate(mediator);

        if (currentAction == CPredicatedMediator::ABORT) {
            // short circuit if abort returned
            action = currentAction;
            break;
        } else if (currentAction == CPredicatedMediator::SKIP){
            // if skipping, then let the other predicates continue to
            // update, but remember that we have to skip
            action = currentAction;
        } // else CONTINUE was returned, don't both storing result
    }
    return action;
}

CPredicatedMediator::Action
CCompositePredicate::postInputUpdate(CPredicatedMediator& mediator, int type)
{
    CPredicatedMediator::Action action = CPredicatedMediator::CONTINUE;
    auto currentAction = action;

    for (auto pPred : m_predicates) {
        currentAction = pPred->postInputUpdate(mediator, type);

        if (currentAction == CPredicatedMediator::ABORT) {
            // short circuit if abort returned
            action = currentAction;
            break;
        } else if (currentAction == CPredicatedMediator::SKIP){
            // if skipping, then let the other predicates continue to
            // update, but remember that we have to skip
            action = currentAction;
        } // else CONTINUE was returned, don't both storing result
    }
    return action;
}

CPredicatedMediator::Action
CCompositePredicate::preOutputUpdate(CPredicatedMediator& mediator, int type)
{
    CPredicatedMediator::Action action = CPredicatedMediator::CONTINUE;
    auto currentAction = action;

    for (auto pPred : m_predicates) {
        currentAction = pPred->preOutputUpdate(mediator, type);

        if (currentAction == CPredicatedMediator::ABORT) {
            // short circuit if abort returned
            action = currentAction;
            break;
        } else if (currentAction == CPredicatedMediator::SKIP){
            // if skipping, then let the other predicates continue to
            // update, but remember that we have to skip
            action = currentAction;
        } // else CONTINUE was returned, don't both storing result
    }
    return action;
}

CPredicatedMediator::Action
CCompositePredicate::postOutputUpdate(CPredicatedMediator& mediator, int type)
{
    CPredicatedMediator::Action action = CPredicatedMediator::CONTINUE;
    auto currentAction = action;

    for (auto pPred : m_predicates) {
        currentAction = pPred->postOutputUpdate(mediator, type);

        if (currentAction == CPredicatedMediator::ABORT) {
            // short circuit if abort returned
            action = currentAction;
            break;
        } else if (currentAction == CPredicatedMediator::SKIP){
            // if skipping, then let the other predicates continue to
            // update, but remember that we have to skip
            action = currentAction;
        } // else CONTINUE was returned, don't both storing result
    }
    return action;
}

void CCompositePredicate::reset()
{
    for (auto pPred : m_predicates) {
        pPred->reset();
    }
}

} // end Transform
} // end DAQ
