
#include <CTransform11p0to11p0.h>
#include <NSCLDAQ11/CRingItemFactory.h>
#include <NSCLDAQ11/CRingItem.h>
#include <CFilter.h>

#include <iostream>
using namespace std;

CTransform11p0to11p0::CTransform11p0to11p0(unique_ptr<CFilter> pFilter)
  : m_pFilter(move(pFilter))
{}

CTransform11p0to11p0::FinalType 
CTransform11p0to11p0::operator()(InitialType& item)
{
  InitialType* pItem = NSCLDAQ11::CRingItemFactory::createRingItem(item);

  return dispatch(pItem);
}

CTransform11p0to11p0::FinalType 
CTransform11p0to11p0::dispatch(InitialType* pItem)
{
    using namespace NSCLDAQ11;

    // initial pointer to filtered item
  InitialType* fitem = pItem;

  switch(pItem->type()) {

    // State change items
    case BEGIN_RUN:
    case END_RUN:
    case PAUSE_RUN:
    case RESUME_RUN:
      fitem = m_pFilter->handleStateChangeItem(static_cast<CRingStateChangeItem*>(pItem));
      break;

      // Documentation items
    case PACKET_TYPES:
    case MONITORED_VARIABLES:
      fitem = m_pFilter->handleTextItem(static_cast<CRingTextItem*>(pItem));
      break;

      // Scaler items
    case PERIODIC_SCALERS:
      fitem = m_pFilter->handleScalerItem(static_cast<CRingScalerItem*>(pItem));
      break;

      // Physics event item
    case PHYSICS_EVENT:
      fitem = m_pFilter->handlePhysicsEventItem(static_cast<CPhysicsEventItem*>(pItem));
      break;

      // Physics event count
    case PHYSICS_EVENT_COUNT:
      fitem = m_pFilter->handlePhysicsEventCountItem(static_cast<CRingPhysicsEventCountItem*>(pItem));
      break;

      // Event builder fragment handlers
    case EVB_FRAGMENT:
    case EVB_UNKNOWN_PAYLOAD:
      fitem = m_pFilter->handleFragmentItem(static_cast<CRingFragmentItem*>(pItem));
      break;

      // Handle any other generic ring item...this can be 
      // the hook for handling user-defined items
    default:
      fitem = m_pFilter->handleRingItem(pItem);
      break;
  }

  return CRingItem(*fitem);
}

}
