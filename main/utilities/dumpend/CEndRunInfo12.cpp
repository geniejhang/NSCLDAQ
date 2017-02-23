/**
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   <filename>
# @brief  <brief description>
# @author <fox@nscl.msu.edu>
*/

#include "CEndRunInfo12.h"
#include <CFileDataSource.h>
#include "V10/CRingStateChangeItem.h"
#include "V10/DataFormatV10.h"
#include <RingIOV10.h>
#include <stdexcept>
#include <fstream>
#include <io.h>

using namespace DAQ;

/**
 * constructor
 *
 *  @param fd - File descriptor open on the event file.
 */
CEndRunInfo12::CEndRunInfo12(int fd) :
CEndRunInfo(fd)
{
//  loadEndRuns();
}
/**
 *  destructor
 */
CEndRunInfo12::~CEndRunInfo12() {}

/**
 * numEnds
 *   @return unsigned - the number of end runs in the file analyzed by the constructor.
 */
unsigned
CEndRunInfo12::numEnds() const
{
//  return m_endRuns.size();
}

/**
 * hasBodyHeader
 *   @param which - number of the end run we are asking about.
 *   @return bool - false - version10.x ring items never had body headers.
 *   @throw std::range_error - if which is outside the range of valid indices in m_endRuns.
 *
 */
bool
CEndRunInfo12::hasBodyHeader(int which) const
{
//  throwIfBadIndex(which);
  return false;
}

/**
 * getEventTimestamp
 *   @param which - Number of the end run record to get info about.
 *   @throw std::range_Error - if which is not a valid index in m_endRuns.
 *   @throw std::string - because there are no body headers to have an even timestamp in
 *                        10.x
 */
uint64_t
CEndRunInfo12::getEventTimestamp(int which) const
{
//  throwIfBadIndex(which);
//  throw std::string("Version 10.x ring items don't have body headers");
}
/**
 * getSourceId
 *   @param which - which end run record being queried.
 *   @throw std::range_error - if 'which' is not a valid index in m_endRuns.
 *   @throw std::string      - because version 10.x has no body headers in which to have srcids.
 */
uint32_t
CEndRunInfo12::getSourceId(int which) const
{
//  throwIfBadIndex(which);
//  throw std::string("Version 10.x ring items dont' have body headers");
}
/**
 *  getBarrierType
 *    @param which - end run being queried.
 *    @throw std::range_error - if which is ont a valid index in m_endRuns.
 *    @throw std::string - because version 10.x has nobody headeers in which to have barrier type codes.
 */
uint32_t
CEndRunInfo12::getBarrierType(int which) const
{
//  throwIfBadIndex(which);
//  throw std::string("Version 10.x ring items don't have a body headers");
}

/**
 * getRunNumber
 *   Return the run number from an end run item.
 *   @param which - index of the end run item to get.
 *   @return uint32_t - The run number.
 *   @throw std::range_error - which is out of range.
 */
uint32_t
CEndRunInfo12::getRunNumber(int which) const
{
//  throwIfBadIndex(which);
//  return m_endRuns[which]->getRunNumber();
    return 0;
}
/**
 * getElapsedTime
 *    Return the number of seconds the run lasted. Note that in 10.x this is always a whole number.
 *   @param  which - the number of the end run record being queried.
 *   @return float  - Number of seconds into the run the end occured.
 *   @throw std::range_error - if which is not a valid index.
 */
float
CEndRunInfo12::getElapsedTime(int which) const
{
//  throwIfBadIndex(which);
//  return m_endRuns[which]->getElapsedTime();
    return 0;
}

/**
 * getTItle
 *   Return the run's title.
 *   @param which - which end run is being interrogated.
 *   @return std::string - the title.
 *   @throw std::range_error - if which is not a valid index.
 */
std::string
CEndRunInfo12::getTitle(int which) const
{
//  throwIfBadIndex(which);
//  return m_endRuns[which]->getTitle();        // Required to be null terminated.
    return 0;
}
/**
 * getTod
 *   Return date/type of the end of run.
 *   @param which - which end run is being interrogated.
 *   @return time_t - unix timestamp of the end time.
 *   @throw std::range_error - if which is invalid.
 */
time_t
CEndRunInfo12::getTod(int which) const
{
//  throwIfBadIndex(which);
//  return m_endRuns[which]->getTimestamp();
    return 0;
}
/*----------------------------------------------------------------------------------------------------
** Private utilities.
*/

/**
 * throwIfBadIndex
 *   @param which - index into the m_endRuns. vector.
 *   @throw std::range_error - if the index is bad.
 */
void
CEndRunInfo12::throwIfBadIndex(int which) const
{
//  unsigned w = which;
//  if (w >= m_endRuns.size()) {
//    throw std::range_error("CEndRunInfo12  -- End run selected does not exist");
//  }
}
/**
 * loadEndRuns
 *    Flip through the event file and load all end run items into the m_endRuns
 *    vector.
 *
 *   @note we're going to read ring item headers.  If the ring item is not an
 *          END_RUN we'll lseek past the body.  If it is, we'll load the remainder
 *          if the ring item into a new'd struct and save it in the ring array.
 */
void
CEndRunInfo12::loadEndRuns()
{
//    CFileDataSource source(m_nFd);
//    V10::CRingItem item(V10::VOID);

//  while (true) {
//    readItem(source, item);
//    if (source.eof()) break;

//    if (item.type() == V10::END_RUN) {

//        std::unique_ptr<V10::CRingStateChangeItem>
//                pItem(new V10::CRingStateChangeItem(item));
//        m_endRuns.push_back(std::move(pItem));

//    }
//  }
}
