/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
       NSCL
       Michigan State University
       East Lansing, MI 48824-1321
*/

#ifndef DAQ_CPROCESSCOUNTPREDICATE_H
#define DAQ_CPROCESSCOUNTPREDICATE_H

#include <CPredicate.h>

namespace DAQ {

class CProcessCountPredicate;

// some useful typedefs
using CProcessCountPredicateUPtr = std::unique_ptr<CProcessCountPredicate>;
using CProcessCountPredicatePtr  = std::shared_ptr<CProcessCountPredicate>;

/*!
 * \brief Predicate for skipping and processing only a certain number of items
 *
 * The CProcessCountPredicate is responsible for handling the logic associated
 * with the --skip and --count options in a filter program. The logic is as follows:
 *
 * Skip the first N items, then process the next M items.
 *
 * This logic is performed after a read occurs, so if the user has excluded certain
 * types in the reading from the source, those excluded types will not be counted
 * in the skips or the processing.
 */
class CProcessCountPredicate: public CPredicate
{
  private:
    size_t m_toSkip;        ///< the total number of items to skip
    size_t m_toProcess;     ///< the total number of items to process after skipping
    size_t m_skipped;       ///< how many items have been skipped so far
    size_t m_processed;     ///< how many items have been processed so far

  public:
    CProcessCountPredicate(size_t nToSkip=0, size_t nToProcess=0);
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

} // end DAQ

#endif
