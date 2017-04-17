/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#include "CRingSource.h"
#include "GetOpt.h"
#include "rfcmdline.h"

#include <CDataSourceFactory.h>
#include <CDataSource.h>

#include <V12/CRawRingItem.h>
#include <V12/DataFormat.h>
#include <RingIOV12.h>

#include <EVBFramework.h>
#include <fragment.h>
#include <ByteBuffer.h>

#include <fstream>
#include <iostream>

#include <iterator>
#include <algorithm>
#include <thread>
#include <stdexcept>
#include <cassert>
#include <cstdint>

#include <sys/time.h>

namespace DAQ {

static std::ostream& logfile(std::cerr);
static uint64_t lastTimestamp = V12::NULL_TIMESTAMP;

static size_t max_event(1024*1024*10); // initial Max bytes of events in a getData

/*----------------------------------------------------------------------
 * Canonicals
 */


CRingSource::CRingSource(CDataSourcePtr pBuffer,
                         const std::vector<uint32_t>& allowedIds)
  : m_pArgs(nullptr),
  m_pBuffer(pBuffer),
  m_allowedSourceIds(allowedIds),
  m_fOneshot(false),
  m_nEndRuns(1),
  m_nEndsSeen(0),
  m_nTimeout(0),
  m_nTimeWaited(0),
  m_wrapper()
{
    m_wrapper.setAllowedSourceIds(m_allowedSourceIds);
}


/**
 * constructor:
 *  
 *  Parse and save the commandline options.
 *
 * @param argc - number of command line words.
 * @param argv - array of pointers to command line words.
 */
CRingSource::CRingSource(int argc, char** argv) :
  m_pArgs(0),
  m_pBuffer(0),
  m_nEndRuns(1),
  m_nEndsSeen(0),
  m_nTimeout(0),
  m_nTimeWaited(0)
{
  GetOpt parsed(argc, argv);
  m_pArgs = new gengetopt_args_info;
  memcpy(m_pArgs, parsed.getArgs(), sizeof(gengetopt_args_info));

  if (m_pArgs->oneshot_given) {
    m_fOneshot  = true;
    m_nEndRuns = m_pArgs->oneshot_arg;
  } else {
    m_fOneshot = false;
  }
  m_nTimeout = m_pArgs->timeout_arg * 1000;        // End run timeouts in ms.
  m_nTimeOffset = m_pArgs->offset_arg;             // tick time offset.
  
}

/**
 * destructor
 *
 *  Free the gengetopt_args_info pointer
 */
CRingSource::~CRingSource() 
{
  delete m_pArgs;
}

/*---------------------------------------------------------------------
 * Public interface:
 */


/**
 * initialize
 *
 *  One time initialization what we need to do is:
 *  - get the URL that is the ring data source and make a consumer attachment.
 *  - Get our source id
 *  - Get a pointer to the timestamp extraction function.
 *
 * @throw std::string in the event of an error with the reason for the error
 *        as the string value.
 */
void
CRingSource::initialize()
{
  std::string url = m_pArgs->ring_arg;
  
  // Process the source id and body headers flags:

  if (m_pArgs->ids_given > 0) {
    m_allowedSourceIds.insert(m_allowedSourceIds.end(),
			      m_pArgs->ids_arg, 
			      m_pArgs->ids_arg + m_pArgs->ids_given);
  } else { //(m_pArgs->ids_given==0) {
    throw std::string("The list of source ids (--ids) are required for this source!");
  }

  m_wrapper.setAllowedSourceIds(m_allowedSourceIds);

  logfile << std::hex;

  // Attach the ring.

  m_pBuffer = CDataSourceFactory().makeSource(url);

}
/**
 * dataReady
 *
 *   Waits until there is data in the ring for at most the specified number
 *   of ms.
 *
 * @param ms - Number of milliseconds to block.
 *
 * @return bool - true if there was data after the time.
 */
bool
CRingSource::dataReady(int ms)
{
  struct timespec initial;
  struct timespec now;

  clock_gettime(CLOCK_MONOTONIC, &initial);

  do {
    if (m_pBuffer->availableData()) {
      m_nTimeWaited = 0;
      return true;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    clock_gettime(CLOCK_MONOTONIC, &now);
  } while(timedifMs(now, initial) < ms);
  m_nTimeWaited += ms;
  if (m_fOneshot && (m_nEndsSeen > 0) && (m_nTimeWaited > m_nTimeout)) {
    std::cerr << "End run timeout expired exiting\n";
    exit(EXIT_FAILURE);
  }
  
  return false;			// timed out.
}
/**
 * getEvents
 *
 *  Takes data from the ring buffer and builds event fragment lists.
 *  - the event source comes from m_allowedSourceIds.
 *  - scaler and trigger count events become untimestamped fragments.
 *  - State transition events become barriers whose type is the same as
 *    the type in their ring type
 *  - The payload of each fragment is the entire ring item (header and all).
 *
 *
 */
void CRingSource::transformAvailableData()
{
    size_t bytesPackaged(0);

    V12::CRawRingItem item;

    while ((bytesPackaged < max_event) && m_pBuffer->availableData()) {
        readItem(*m_pBuffer, item);

        // check for end runs for oneshot logic
        if (item.type() == V12::END_RUN) {
            m_nEndsSeen++;
        }

        auto frag = m_wrapper(item);

        if (item.size() > (max_event*2 - bytesPackaged)) {
            max_event = item.size() + bytesPackaged;
        }

        bytesPackaged += item.size() + sizeof(::EVB::FragmentHeader);

        m_frags.push_back(frag);
    }
}

void
CRingSource::getEvents()
{

  m_frags.clear(); // start fresh

  transformAvailableData();

  // Send those fragments to the event builder:

  if (m_frags.size()) {
    CEVBClientFramework::submitFragmentList(m_frags);
  }

  if (oneshotComplete()) {
    exit(EXIT_SUCCESS);
  }

}

bool CRingSource::oneshotComplete()
{
     return (m_fOneshot && (m_nEndsSeen >= m_nEndRuns));
}

/**
 * shutdown 
 *
 * Shuts the data sourcd down.  For us that's just killing off the
 * m_pBuffer.
 */
void
CRingSource::shutdown()
{
}

void
CRingSource::validateItem(const DAQ::V12::CRingItem &item)
{
    auto searchResult = std::find(m_allowedSourceIds.begin(),
                            m_allowedSourceIds.end(),
                            item.getSourceId());
    if  (searchResult == m_allowedSourceIds.end()) {
        string errmsg("Observed source id that was not provided via the --ids option");
        throw runtime_error(errmsg);
    }

    if (item.getEventTimestamp() == 0ull) {
      std::cerr << "Zero timestamp in source!?!\n";
    }
}


void CRingSource::setAllowedSourceIds(const std::vector<uint32_t> &ids)
{
    m_allowedSourceIds = ids;
}


/*----------------------------------------------------------------------
** Private utilities:
*/

/**
 * timedifMs 
 *
 * Returns the difference between two times as timespec structs in milliseconds.
 *
 * @param later - The 'later time.'
 * @param earlier - The 'earlier time'.  This is subtracted from later to give the answer.
 *
 * @return uint64_t
 * @retval (later - earlier) in millisecond units.
 *
 * @throw - negative time differences throw an std::string (BUG).
 */
uint64_t
CRingSource::timedifMs(struct timespec& later, struct timespec& earlier)
{
  struct timeval l = {later.tv_sec, later.tv_nsec/1000};
  struct timeval e = {earlier.tv_sec, earlier.tv_nsec/1000};
  struct timeval res;
  timersub(&l, &e, &res);
  
  
  uint64_t result = res.tv_sec * 1000;
  result         += res.tv_usec/1000;
  
  
  return result;
  

}

} // end DAQ
