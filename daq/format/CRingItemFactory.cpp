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


#include "CRingItemFactory.h"
#include "CRingItem.h"
#include "CPhysicsEventItem.h"
#include "CRingFragmentItem.h"
#include "CRingPhysicsEventCountItem.h"
#include "CRingScalerItem.h"
#include "CRingStateChangeItem.h"
#include "CRingTextItem.h"
#include "CPhysicsEventItem.h"
#include "DataFormat.h"

#include <vector>
#include <string>
#include <string.h>

/**
 * Create a ring item of the correct underlying type as indicated by the
 * ring item type.  Note that the result is dynamically allocated and must be
 * freed by the caller (via delete).
 *
 * @param item - Reference to the item.
 * 
 * @return CRingItem*
 * @retval Pointer to an object that is some subclass of CRingItem.  Note that if the
 *         ring item type is not recognized, a true CRing Item is produced.
 *
 */
CRingItem*
CRingItemFactory::createRingItem(CRingItem& item)
{
  switch (item.type()) {
    // State change:

  case BEGIN_RUN:
  case END_RUN:
  case PAUSE_RUN:
  case RESUME_RUN:
    {
      pStateChangeItem pSrcBody = reinterpret_cast<pStateChangeItem>(item.getItemPointer());
      return new CRingStateChangeItem(
         item.type(), pSrcBody->s_runNumber, pSrcBody->s_timeOffset, pSrcBody->s_Timestamp,
	 std::string(pSrcBody->s_title)
      );
    }

    // String list.

  case PACKET_TYPES:
  case MONITORED_VARIABLES:
    {
      pTextItem pSrcBody = reinterpret_cast<pTextItem>(item.getItemPointer());
      std::vector<std::string> strings;
      char* pString = pSrcBody->s_strings;
      for (int i = 0; i < pSrcBody->s_stringCount; i++) {
	strings.push_back(pString);
	pString += strlen(pString) + 1; // +1 for the terminating null.
      }
      return new CRingTextItem(
          item.type(), strings, pSrcBody->s_timeOffset, pSrcBody->s_timestamp
      );
    }
    // Scalers:

  case INCREMENTAL_SCALERS:
    {
      pScalerItem pSrcBody = reinterpret_cast<pScalerItem>(item.getItemPointer());
      std::vector<uint32_t> scalers(
          pSrcBody->s_scalers, pSrcBody->s_scalers + pSrcBody->s_scalerCount
      );
      return new CRingScalerItem(
          pSrcBody->s_intervalStartOffset, pSrcBody->s_intervalEndOffset, 
	  pSrcBody->s_timestamp, scalers
      );
      
    }

    // Physics trigger:

  case PHYSICS_EVENT:
    {
      CPhysicsEventItem* pItem = new CPhysicsEventItem(PHYSICS_EVENT, item.getStorageSize());
      uint8_t* pDest = reinterpret_cast<uint8_t*>(pItem->getBodyCursor());
      memcpy(pDest, item.getBodyPointer(), item.getBodySize());
      pDest += item.getBodySize();
      pItem->setBodyCursor(pDest);
      pItem->updateSize();
      return pItem;
    }
    // trigger count.

  case PHYSICS_EVENT_COUNT:
    {
      pPhysicsEventCountItem pItem = reinterpret_cast<pPhysicsEventCountItem>(item.getItemPointer());
      return new CRingPhysicsEventCountItem(pItem->s_eventCount, pItem->s_timeOffset, pItem->s_timestamp);
      break;
    }
  // /Event builder fragment.
  case EVB_FRAGMENT:
    {
      pEventBuilderFragment pItem = reinterpret_cast<pEventBuilderFragment>(item.getItemPointer());
      return new CRingFragmentItem(
          pItem->s_timestamp, pItem->s_sourceId, pItem->s_payloadSize, 
          pItem->s_body, pItem->s_barrierType
      );
    }
    break;
   // Nothing we know about:

  default:
    return new CRingItem(item);
  }
}
