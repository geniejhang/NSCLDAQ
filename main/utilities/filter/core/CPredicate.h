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

#ifndef DAQ_CPREDICATE_H
#define DAQ_CPREDICATE_H

#include <memory>
#include <CPredicatedMediator.h>

namespace DAQ {

class CPredicate;
using CPredicatePtr = std::shared_ptr<CPredicate>;

class CPredicate
{
public:
    virtual CPredicatedMediator::Action preInputUpdate(CPredicatedMediator& transform) = 0;

    virtual CPredicatedMediator::Action postInputUpdate(CPredicatedMediator& transform,
                                                       int type) = 0;

    virtual CPredicatedMediator::Action preOutputUpdate(CPredicatedMediator& transform,
                                                       int type) = 0;

    virtual CPredicatedMediator::Action postOutputUpdate(CPredicatedMediator& transform,
                                                        int type) = 0;

    virtual void reset() = 0;

};

} // end DAQ

#endif
