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
#ifndef DAQ_CCOMPOSITEPREDICATE_H
#define DAQ_CCOMPOSITEPREDICATE_H

#include <CPredicate.h>

#include <vector>
#include <memory>

namespace DAQ {

class CCompositePredicate;
using CCompositePredicateUPtr = std::unique_ptr<CCompositePredicate>;
using CCompositePredicatePtr = std::shared_ptr<CCompositePredicate>;

class CCompositePredicate : public CPredicate
{
  private:
    std::vector<CPredicatePtr> m_predicates;

  public:
    void addPredicate(CPredicatePtr pPred);
    std::vector<CPredicatePtr>& getPredicates();

    virtual CPredicatedMediator::Action preInputUpdate(CPredicatedMediator& mediator);
    virtual CPredicatedMediator::Action postInputUpdate(CPredicatedMediator& mediator, int type);
    virtual CPredicatedMediator::Action preOutputUpdate(CPredicatedMediator& mediator, int type);
    virtual CPredicatedMediator::Action postOutputUpdate(CPredicatedMediator& mediator, int type);

    virtual void reset();
};

} // end DAQ

#endif
