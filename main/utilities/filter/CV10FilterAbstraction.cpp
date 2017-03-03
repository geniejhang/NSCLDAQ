#include "CV10FilterAbstraction.h"

#include <CDataSource.h>
#include <CDataSink.h>

#include <RingIOV10.h>

#include <V10/CRingItemFactory.h>
#include <V10/CRingItem.h>
#include <V10/CRingScalerItem.h>
#include <V10/CRingTimestampedRunningScalerItem.h>
#include <V10/CRingStateChangeItem.h>
#include <V10/CRingTextItem.h>
#include <V10/CPhysicsEventItem.h>
#include <V10/CRingFragmentItem.h>
#include <V10/CRingPhysicsEventCountItem.h>
#include <V10/DataFormatV10.h>

#include <V10/CFilter.h>

namespace DAQ {
namespace V10 {

CFilterAbstraction::CFilterAbstraction()
    : m_item(),
      m_pInputItem(nullptr),
      m_pOutputItem(nullptr),
      m_pFilter()
{}

CFilterAbstraction::~CFilterAbstraction()
{
    if (m_pOutputItem != m_pInputItem) {
        // this is legal if m_pOutputItem == nullptr
        delete m_pOutputItem;
    }

    delete m_pInputItem;
}

void CFilterAbstraction::readDatum(CDataSource &source)
{
    readItem(source, m_item);
}

void CFilterAbstraction::processDatum()
{
    m_pInputItem = V10::CRingItemFactory::createItem(item);
    m_pOutputItem = dispatch(*m_pInputItem);
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

void CFilterAbstraction::cleanUp()
{
    if (m_pOutputItem != m_pInputItem) {
        delete m_pOutputItem;
        m_pOutputItem = nullptr;
    }

    delete m_pInputItem;
}


V10::CRingItem*
CFilterAbstraction::dispatch(V10::CRingItem &item)
{
    if (!m_pFilter) {
        throw std::runtime_error("CFilterAbstraction::dispatch() User must provide a filter prior to dispatching");
    }

    // initial pointer to filtered item
    V10::CRingItem* pFilteredItem = &item;

    switch(item.type()) {

    // State change items
    case BEGIN_RUN:
    case END_RUN:
    case PAUSE_RUN:
    case RESUME_RUN:
        pFilteredItem = m_pFilter->handleStateChangeItem(static_cast<CRingStateChangeItem*>(&item));
        break;

        // Documentation items
    case PACKET_TYPES:
    case MONITORED_VARIABLES:
        pFilteredItem = m_pFilter->handleTextItem(static_cast<CRingTextItem*>(&item));
        break;

        // Scaler items
    case INCREMENTAL_SCALERS:
        pFilteredItem = m_pFilter->handleScalerItem(static_cast<CRingScalerItem*>(&item));
        break;

    case TIMESTAMPED_NONINCR_SCALERS:
        pFilteredItem = m_pFilter->handleTimestampedScaler(static_cast<CRingTimestampedRunningScalerItem*>(&item));
        break;

        // Physics event item
    case PHYSICS_EVENT:
        pFilteredItem = m_pFilter->handlePhysicsEventItem(static_cast<CPhysicsEventItem*>(&item));
        break;

        // Physics event count
    case PHYSICS_EVENT_COUNT:
        pFilteredItem = m_pFilter->handlePhysicsEventCountItem(static_cast<CRingPhysicsEventCountItem*>(&item));
        break;

        // Event builder fragment handlers
    case EVB_FRAGMENT:
    case EVB_UNKNOWN_PAYLOAD:
        pFilteredItem = m_pFilter->handleFragmentItem(static_cast<CRingFragmentItem*>(&item));
        break;

        // Handle any other generic ring item...this can be
        // the hook for handling user-defined items
    default:
        pFilteredItem = m_pFilter->handleRingItem(pItem);
        break;
    }

    return pFilteredItem;
}

}


} // end V10
} // end DAQ
