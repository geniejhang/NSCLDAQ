#include "CV12FilterAbstraction.h"

#include <CDataSource.h>
#include <CDataSink.h>

#include <CRingIOV12.h>

#include <V12/CRingItemFactory.h>
#include <V12/CRingStateChangeItem.h>
#include <V12/CRingScalerItem.h>
#include <V12/CRingTextItem.h>
#include <V12/CPhysicsEventItem.h>
#include <V12/CRingPhysicsEventCountItem.h>
#include <V12/CCompoundRingItem.h>
#include <V12/DataFormat.h>

#include <V12/CFilter.h>

namespace DAQ {
namespace V12 {

CFilterAbstraction::CFilterAbstraction()
    : m_item(),
      m_pInputItem(),
      m_pOutputItem(),
      m_pFilter()
{}

CFilterAbstraction::~CFilterAbstraction()
{
}

void CFilterAbstraction::readDatum(CDataSource &source)
{
    readItem(source, m_item);
}

void CFilterAbstraction::processDatum()
{
    m_pInputItem = CRingItemFactory::createItem(m_item);
    m_pOutputItem = dispatch(m_pInputItem);
}


void CFilterAbstraction::outputDatum(CDataSink& sink)
{
    if (m_pOutputItem) {
        writeItem(sink, *m_pOutputItem);
    }
}

uint32_t CFilterAbstraction::getDatumType() const
{
    if (m_pOutputItem) {
        return m_pOutputItem->type();
    } else {
        return m_pInputItem->type();
    }
}

void CFilterAbstraction::cleanup()
{
    m_pInputItem = nullptr;
    m_pOutputItem = nullptr;
}


CRingItemPtr CFilterAbstraction::dispatch(CRingItemPtr pItem)
{
    if (!m_pFilter) {
        throw std::runtime_error("CV11VersionFilterAbstraction::dispatch() User must provide a filter prior to dispatching");
    }

    // initial pointer to filtered item
  CRingItemPtr pFilteredItem = pItem;

  switch(pItem->type()) {

  // State change items
  case BEGIN_RUN:
  case END_RUN:
  case PAUSE_RUN:
  case RESUME_RUN:
      pFilteredItem = m_pFilter->handleStateChangeItem(
                  std::dynamic_pointer_cast<CRingStateChangeItem>(pItem)
                  );
      break;

  case ABNORMAL_ENDRUN:
      pFilteredItem = m_pFilter->handleAbnormalEndRun(
                  std::dynamic_pointer_cast<CAbnormalEndRun>(pItem)
                  );
      break;

      // Documentation items
  case PACKET_TYPES:
  case MONITORED_VARIABLES:
      pFilteredItem = m_pFilter->handleTextItem(
                  std::dynamic_pointer_cast<CRingTextItem>(pItem)
                  );
      break;


  case RING_FORMAT:
      pFilteredItem = m_pFilter->handleDataFormatItem(
                  std::dynamic_pointer_cast<CDataFormatItem>(pItem)
                  );
      break;

      // Scaler items
  case PERIODIC_SCALERS:
      pFilteredItem = m_pFilter->handleScalerItem(
                  std::dynamic_pointer_cast<CRingScalerItem>(pItem)
                  );
      break;

      // Physics event item
  case PHYSICS_EVENT:
      pFilteredItem = m_pFilter->handlePhysicsEventItem(
                  std::dynamic_pointer_cast<CPhysicsEventItem>(pItem)
                  );
      break;

      // Physics event count
  case PHYSICS_EVENT_COUNT:
      pFilteredItem = m_pFilter->handlePhysicsEventCountItem(
                  std::dynamic_pointer_cast<CRingPhysicsEventCountItem>(pItem)
                  );
      break;

  case EVB_GLOM_INFO:
      pFilteredItem = m_pFilter->handleGlomInfoItem(
                  std::dynamic_pointer_cast<CGlomParameters>(pItem)
                  );
      break;

  case COMP_BEGIN_RUN:
  case COMP_END_RUN:
  case COMP_RESUME_RUN:
  case COMP_PAUSE_RUN:
  case COMP_ABNORMAL_ENDRUN:
  case COMP_MONITORED_VARIABLES:
  case COMP_PACKET_TYPES:
  case COMP_RING_FORMAT:
  case COMP_PERIODIC_SCALERS:
  case COMP_PHYSICS_EVENT:
  case COMP_PHYSICS_EVENT_COUNT:
  case COMP_EVBGLOM_INFO:
      pFilteredItem = m_pFilter->handleCompositeItem(
                  std::dynamic_pointer_cast<CCompositeItem>(pItem)
                  );
      break;

      // Handle any other generic ring item...this can be
      // the hook for handling user-defined items
  default:
      pFilteredItem = m_pFilter->handleRingItem(pItem);
      break;
  }

  return pFilteredItem;

}


} // end V12
} // end DAQ

