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

#include <CProcessCountPredicate.h>

namespace DAQ {
namespace Transform {


CProcessCountPredicate::CProcessCountPredicate(size_t nToSkip, size_t nToProcess)
  : m_toSkip(nToSkip),
    m_toProcess(nToProcess),
    m_skipped(0),
    m_processed(0)
{}


CPredicatedMediator::Action
CProcessCountPredicate::preInputUpdate(CPredicatedMediator &transform)
{
    return CPredicatedMediator::CONTINUE;
}


CPredicatedMediator::Action
CProcessCountPredicate::postInputUpdate(CPredicatedMediator &transform, int type)
{
    if (m_skipped < m_toSkip) {
        m_skipped++;
        return CPredicatedMediator::SKIP;
    }

    if (m_processed < m_toProcess) {
        m_processed++;
        return CPredicatedMediator::CONTINUE;
    } else {
        return CPredicatedMediator::ABORT;
    }
}


CPredicatedMediator::Action
CProcessCountPredicate::preOutputUpdate(CPredicatedMediator &transform, int type)
{
    return CPredicatedMediator::CONTINUE;
}

CPredicatedMediator::Action
CProcessCountPredicate::postOutputUpdate(CPredicatedMediator &transform, int type)
{
    return CPredicatedMediator::CONTINUE;
}

void CProcessCountPredicate::reset()
{
    m_skipped = 0;
    m_processed = 0;
}

void CProcessCountPredicate::setSkipCount(size_t count)
{
    m_skipped = count;
}

size_t CProcessCountPredicate::getSkipCount() const
{
    return m_skipped;
}



void CProcessCountPredicate::setNumberToSkip(size_t nToSkip)
{
    m_toSkip = nToSkip;
}

size_t CProcessCountPredicate::getNumberToSkip() const
{
    return m_toSkip;
}



void CProcessCountPredicate::setProcessCount(size_t count)
{
    m_processed = count;
}

size_t CProcessCountPredicate::getProcessCount() const
{
    return m_processed;
}


void CProcessCountPredicate::setNumberToProcess(size_t nToProcess)
{
    m_toProcess = nToProcess;
}

size_t CProcessCountPredicate::getNumberToProcess() const
{
    return m_toProcess;
}


} // end Transform
} // end DAQ
