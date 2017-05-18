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

/*!
 * \brief Constructor
 *
 * \param nToSkip       the total number to skip
 * \param nToProcess    the total number to process
 *
 * The counters for how many have been skipped or processed already are zeroed.
 */
CProcessCountPredicate::CProcessCountPredicate(size_t nToSkip, size_t nToProcess)
  : m_toSkip(nToSkip),
    m_toProcess(nToProcess),
    m_skipped(0),
    m_processed(0)
{}

/*!
 * \return CONTINUE
 */
CPredicatedMediator::Action
CProcessCountPredicate::preInputUpdate(CPredicatedMediator &transform)
{
    return CPredicatedMediator::CONTINUE;
}

/*!
 * \brief The guts of the logic for this class
 *
 * \param transform     the transform
 * \param type          the type of the data item
 *
 * The skip count is incremented until the total number of skippable items has been
 * met. After that point, the process count is incremented until the total number of
 * items to process is met.
 *
 * \retval SKIP     if the number of skipped items is less than the total to skip
 * \retval CONTINUE if the number of skipped items is greater than or equal to total skip count,
 *                  and the number of items processed is less than total process count
 * \retval ABORT    if the number of skipped items is greater than or equal to total skip count,
 *                  and the number of items processed is greater than or equal to total process count
 */
CPredicatedMediator::Action
CProcessCountPredicate::postInputUpdate(CPredicatedMediator &transform, int type)
{
    if (m_skipped < m_toSkip && m_toSkip != 0) {
        m_skipped++;
        return CPredicatedMediator::SKIP;
    }

    if (m_toProcess == 0) {
        // there is no set number to process... just continue processing
        return CPredicatedMediator::CONTINUE;
    } else if (m_processed < m_toProcess ) {
        m_processed++;
        return CPredicatedMediator::CONTINUE;
    } else {
        return CPredicatedMediator::ABORT;
    }
}

/*! \returns CONTINUE */
CPredicatedMediator::Action
CProcessCountPredicate::preOutputUpdate(CPredicatedMediator &transform, int type)
{
    return CPredicatedMediator::CONTINUE;
}

/*! \returns CONTINUE */
CPredicatedMediator::Action
CProcessCountPredicate::postOutputUpdate(CPredicatedMediator &transform, int type)
{
    return CPredicatedMediator::CONTINUE;
}

/*! \brief zero the skip and process counters */
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


} // end DAQ
