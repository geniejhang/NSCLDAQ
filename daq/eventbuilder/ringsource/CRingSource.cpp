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
#include "cmdline.h"

#include <CRingBuffer.h>
#include <CRingItem.h>
#include <DataFormat.h>
#include <CRemoteAccess.h>
#include <EVBFramework.h>
#include <CAllButPredicate.h>
#include <fragment.h>
#include <string.h>
#include <dlfcn.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>


static size_t max_event(1024*1024); // initial Max bytes of events in a getData

/*----------------------------------------------------------------------
 * Canonicals
 */


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
  m_timestamp(0)
{
  GetOpt parsed(argc, argv);
  m_pArgs = new gengetopt_args_info;
  memcpy(m_pArgs, parsed.getArgs(), sizeof(gengetopt_args_info));



}
/**
 * destructor
 *
 *  Free the gengetopt_args_info pointer
 */
CRingSource::~CRingSource() 
{
  delete m_pArgs;
  delete m_pBuffer;		// Just in case we're destructed w/o shutdown.
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
  
  // Require only one source id:

  if (m_pArgs->ids_given > 1) {
    throw std::string("This source only supports a sinlgle event id");
  }
  m_sourceId = static_cast<uint32_t>(m_pArgs->ids_arg[0]);

  std::string dlName = m_pArgs->timestampextractor_arg;

  // Attach the ring.

  m_pBuffer = CRingAccess::daqConsumeFrom(url);

  // Load the DLL and look up the timestamp function (putting i in m_timestamp;
  // we never do a dlclose so the DLL remains loaded in all OS's.

  void* pDLL = dlopen(dlName.c_str(), RTLD_NOW);
  if (!pDLL) {
    int e = errno;
    std::string msg = "Failed to load shared lib ";
    msg += dlName;
    msg += " ";
    msg += strerror(e);
    throw msg;
  }
  m_timestamp = reinterpret_cast<tsExtractor>(dlsym(pDLL, "timestamp"));
  if (!m_timestamp) {
    int e errno;
    std::string msg = "Failed to locate timestamp function in ";
    msg += dlName;
    msg += " ";
    msg += strerror(e);
    throw msg;
  }
  
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
    if (m_pBuffer->availableData()) return true;
    m_pBuffer->pollblock();	// block a while.

    clock_gettime(CLOCK_MONOTONIC, &now);
  } while(timedifMs(now, initial) < ms);

  return false;			// timed out.
}
/**
 * getEvents
 *
 *  Takes data from the ring buffer and builds event fragment lists.
 *  - the event source comes from m_sourceId
 *  - scaler and trigger count events become untimestamped fragments.
 *  - State transition events become barriers whose type is the same as
 *    the type in their ring type
 *  - The payload of each fragment is the entire ring item (header and all).
 *
 *
 */
void
CRingSource::getEvents()
{
  size_t bytesPackaged(0);
  CAllButPredicate all;		// Predicate to selecdt all ring items.
  CEVBFragmentList frags;
  uint8_t*         pFragments = reinterpret_cast<uint8_t*>(malloc(max_event*2));
  uint8_t*         pDest = pFragments;
  if (pFragments == 0) {
    throw std::string("CRingSource::getEvents - memory allocation failed");
  }

  while ((bytesPackaged < max_event) && m_pBuffer->availableData()) {
    CRingItem* p  = CRingItem::getFromRing(*m_pBuffer, all); // should not block.
    RingItem*  pRingItem = p->getItemPointer();

    // If we got here but the data is bigger than our safety margin
    //we need to resize pFragments:

    if (pRingItem->s_header.s_size > (max_event*2 - bytesPackaged)) {
      size_t offset = pDest - pFragments; // pFragments willchange.
      max_event = pRingItem->s_header.s_size + bytesPackaged;
      pFragments = reinterpret_cast<uint8_t*>(realloc(pFragments, max_event*2));
      pDest      = pFragments + offset;

    }

    // initialize the fragment -- with the assumption that the
    // item is a non-barrier with no timestamp:

    ClientEventFragment frag;
    frag.s_timestamp = NULL_TIMESTAMP;
    frag.s_sourceId  = m_sourceId;
    frag.s_size      = pRingItem->s_header.s_size;
    frag.s_barrierType = 0;
    frag.s_payload   = pDest;
    memcpy(pDest, pRingItem,  pRingItem->s_header.s_size);
    pDest           += pRingItem->s_header.s_size;
    bytesPackaged   += pRingItem->s_header.s_size;

    // Now figure what to do based on the type...default is non-timestamped, non-barrier

    switch (pRingItem->s_header.s_type) {
    case BEGIN_RUN:
    case END_RUN:
    case PAUSE_RUN:
    case RESUME_RUN:
      frag.s_barrierType = pRingItem->s_header.s_type;
      break;
    case PHYSICS_EVENT:
      frag.s_timestamp = (*m_timestamp)(reinterpret_cast<pPhysicsEventItem>(pRingItem));
      break;
    default:
      // default is to leave things alone

      break;
    }
    frags.push_back(frag);
    delete p;



    
  }
  // Send those fragments to the event builder:

  if (frags.size()) {

    CEVBClientFramework::submitFragmentList(frags);
  }


  delete []pFragments;		// free storage.
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
  delete m_pBuffer;
  m_pBuffer = 0;
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
  time_t sec = later.tv_sec;
  long   ns  = later.tv_nsec;

  ns -= earlier.tv_nsec;

  // Borrow from seconds:
  if (ns < 0) {
    sec--;
    ns += 1000*1000*1000;	// 1e9 ns in a sec.
  }

  sec -= earlier.tv_sec;

  // Illegal difference:

  if (sec < 0) {
    throw std::string("BUGCHECK -- Invalid time difference direction");
  }

  uint64_t result = sec;
  result *= 1000;		// milliseconds.
  result += ns/1000*1000;	// Nanoseconds -> ms without rounding.

  return result;
  

}
