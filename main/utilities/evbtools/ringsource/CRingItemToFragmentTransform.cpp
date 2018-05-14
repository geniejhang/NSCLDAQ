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
#include "CRingItemToFragmentTransform.h"
#include "GetOpt.h"
#include "rfcmdline.h"

#include <V12/CRingItem.h>
#include <V12/DataFormat.h>
#include <V12/Serialize.h>
#include <V12/CRingItemFactory.h>
#include <RingIOV12.h>

#include <EVBFramework.h>
#include <fragment.h>

#include <iterator>
#include <algorithm>
#include <string>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <thread>


using namespace std;


namespace DAQ {

static uint64_t lastTimestamp = V12::NULL_TIMESTAMP;


static size_t max_event(1024*128); // initial Max bytes of events in a getData

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
CRingItemToFragmentTransform::CRingItemToFragmentTransform() :
  m_allowedSourceIds(1,0)
{
}

/**
 * destructor
 *
 *  Free the gengetopt_args_info pointer
 */
CRingItemToFragmentTransform::~CRingItemToFragmentTransform() 
{
}

/*---------------------------------------------------------------------
 * Public interface:
 */

  ClientEventFragment
CRingItemToFragmentTransform::operator()(const V12::CRawRingItem& item)
{
  // initialize the fragment -- with the assumption that the
  // item is a non-barrier with no timestamp:

  ClientEventFragment frag;
  frag.s_timestamp = item.getEventTimestamp();
  frag.s_sourceId  = item.getSourceId();
  frag.s_size      = item.size();
  frag.s_barrierType = 0;

  auto pBuffer = new uint8_t[item.size()];

  V12::serializeItem(item, pBuffer);

  frag.s_payload = pBuffer;

  switch (item.type()) {
  case V12::BEGIN_RUN:
  case V12::END_RUN:
  case V12::PAUSE_RUN:
  case V12::RESUME_RUN:
      frag.s_barrierType = item.type();
      break;
  case V12::COMP_BEGIN_RUN:
  case V12::COMP_END_RUN:
  case V12::COMP_PAUSE_RUN:
  case V12::COMP_RESUME_RUN:
      frag.s_barrierType = item.type() & 0x7fff;
      break;
  case V12::PHYSICS_EVENT:
      if (formatPhysicsEvent(item, frag)) {
          lastTimestamp = frag.s_timestamp;
          break;
      }
  default:
      // default is to leave things alone
      // this includes the DataFormat item

      break;
  }

  validateSourceId(frag.s_sourceId);

  if (frag.s_timestamp == 0ll) {
    std::cerr << "Zero timestamp in source!?!\n";
  }

  return frag;
}

/** Handle the case of a physics event without a body header.
  * 
  * This should throw if there is not tstamp extractor provided.
  * Otherwise, if there are no bodyheaders and the tstamp
  *
  * \param item ring item C structure being accessed
  * \param p    ring item object that manages the item param
  * \param frag fragment header being filled in.
  *
  * \returns boolean whether or not the ring item was non-null
  * 
  * \throws when no tstamplib is provided and --expectbodyheaders is specified 
  *
  */
bool
CRingItemToFragmentTransform::formatPhysicsEvent (const V12::CRawRingItem& p,
                                                  ClientEventFragment& frag)
{
  bool retval = true;

  // warn if timestamp is not monotonic
  if ( ((frag.s_timestamp - lastTimestamp) > 0x100000000ll)  &&
           (lastTimestamp != V12::NULL_TIMESTAMP)) {
      V12::CRingItemPtr pSpecificItem = V12::CRingItemFactory::createRingItem(p);
      std::cerr << "Timestamp skip from "  << lastTimestamp << " to " << frag.s_timestamp << endl;
      std::cerr << "Ring item: " << pSpecificItem->toString() << endl;
      retval = false;
  }

  return retval;
}

void
CRingItemToFragmentTransform::validateSourceId(std::uint32_t sourceId) 
{

  if ( ! isValidSourceId(sourceId) ) {
    string errmsg("Source id found that was not provided via the --ids option");
    throw runtime_error(errmsg);
  }
}

bool CRingItemToFragmentTransform::isValidSourceId(std::uint32_t sourceId) 
{
  auto searchResult = std::find(m_allowedSourceIds.begin(),
                          m_allowedSourceIds.end(),
                          sourceId);
  return (searchResult != m_allowedSourceIds.end());
}


} // end DAQ
