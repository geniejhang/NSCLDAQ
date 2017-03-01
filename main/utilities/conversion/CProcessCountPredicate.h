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

#ifndef CPROCESSCOUNTPREDICATE_H
#define CPROCESSCOUNTPREDICATE_H

#include <CPredicate.h>

namespace DAQ {
namespace Transform {

class CProcessCountPredicate: public CPredicate
{
  private:
    size_t m_toSkip;
    size_t m_toProcess;
    size_t m_skipped;
    size_t m_processed;

  public:
    CProcessCountPredicate(size_t nToSkip, size_t nToProcess);
    CProcessCountPredicate(const CProcessCountPredicate& ) = default;
    CProcessCountPredicate& operator=(const CProcessCountPredicate& ) = default;

    virtual CPredicatedMediator::Action
    preInputUpdate(CPredicatedMediator& transform);

    virtual CPredicatedMediator::Action
    postInputUpdate(CPredicatedMediator& transform,
                    int type);

    virtual CPredicatedMediator::Action
    preOutputUpdate(CPredicatedMediator& transform,
                    int type);

    virtual CPredicatedMediator::Action
    postOutputUpdate(CPredicatedMediator& transform,
                     int type);

    virtual void reset();

    void setNumberToSkip(size_t nToSkip);
    size_t getNumberToSkip() const;

    void setSkipCount(size_t count);
    size_t getSkipCount() const;

    void setNumberToProcess(size_t nToProcess);
    size_t getNumberToProcess() const;

    void setProcessCount(size_t count);
    size_t getProcessCount() const;


};


} // end Transform
} // end DAQ

#endif
