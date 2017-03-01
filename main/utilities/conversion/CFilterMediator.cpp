#include "CFilterMediator.h"
#include <RingIOV10.h>
#include <RingIOV11.h>
#include <RingIOV12.h>
#include <V10/CRingItem.h>
#include <V11/CRingItem.h>
#include <V12/CRawRingItem.h>

namespace DAQ {
namespace Transform {


CV10VersionAbstraction::CV10VersionAbstraction()
    : m_item(),
      m_pInputItem(nullptr),
      m_pOutputItem(nullptr),
      m_pFilter()
{}

CV10VersionAbstraction::~CV10VersionAbstraction()
{
    if (m_pOutputItem != m_pInputItem) {
        // this is legal if m_pOutputItem == nullptr
        delete m_pOutputItem;
    }

    delete m_pInputItem;
}

void CV10VersionAbstraction::readDatum(CDataSource &source)
{
    readItem(source, m_item);
}

void CV10VersionAbstraction::processDatum()
{
    m_pInputItem = V10::CRingItemFactory::createItem(item);
    m_pOutputItem = dispatch(*m_pInputItem);
}


void CV10VersionAbstraction::outputDatum(CDataSink& sink)
{
    if (m_pOutputItem) {
        writeItem(sink, *m_pOutputItem);
    }
}

uint32_t CV10VersionAbstraction::getDatumType() const
{
    if (m_pOutputItem) {
        return m_pOutputItem->type();
    } else {
        return m_pInputItem->type();
    }
}

void CV10VersionAbstraction::cleanup()
{
    if (m_pOutputItem != m_pInputItem) {
        delete m_pOutputItem;
        m_pOutputItem = nullptr;
    }

    delete m_pInputItem;
}


V10::CRingItem*
CV10VersionAbstraction::dispatch(V10::CRingItem &item)
{
    using namespace V10;

    if (!m_pFilter) {
        throw std::runtime_error("CV10VersionAbstraction::dispatch() User must provide a filter prior to dispatching");
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
        pFilteredItem = m_pFilter->handleTstampScaler(static_cast<CRingTimestampedRunningScalerItem*>(&item));
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

/////////////////////////////////////////////////////////////////

CV11VersionAbstraction::CV11VersionAbstraction()
    : m_item(),
      m_pInputItem(new V11::CRingItem(VOID)),
      m_pOutputItem(nullptr),
      m_pFilter()
{}

CV11VersionAbstraction::~CV11VersionAbstraction()
{
    if (m_pOutputItem != m_pInputItem) {
        // this is legal if m_pOutputItem == nullptr
        delete m_pOutputItem;
    }

    delete m_pInputItem;
}

void CV11VersionAbstraction::readDatum(CDataSource &source)
{
    readItem(source, *m_pInputItem);
}

void CV11VersionAbstraction::processDatum()
{
    m_pInputItem  = V11::CRingItemFactory::createItem(m_item);
    m_pOutputItem = m_pFilter->handleItem(m_pInputItem);
}


void CV11VersionAbstraction::outputDatum(CDataSink& sink)
{
    if (m_pOutputItem) {
        writeItem(sink, *m_pOutputItem);
    }
}

uint32_t CV11VersionAbstraction::getDatumType() const
{
    if (m_pOutputItem) {
        return m_pOutputItem->type();
    } else {
        return m_pInputItem->type();
    }
}

void CV11VersionAbstraction::cleanup()
{
    if (m_pOutputItem != m_pInputItem) {
        delete m_pOutputItem;
        m_pOutputItem = nullptr;
    }

    m_pInputItem->setType(V11::VOID);
}




V11::CRingItem*
CV11VersionAbstraction::dispatch(V11::CRingItem &item)
{
    using namespace V11;

    if (!m_pFilter) {
        throw std::runtime_error("CV11VersionAbstraction::dispatch() User must provide a filter prior to dispatching");
    }

    // initial pointer to filtered item
  V11::CRingItem* pFilteredItem = &item;

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
    case PERIODIC_SCALERS:
      pFilteredItem = m_pFilter->handleScalerItem(static_cast<CRingScalerItem*>(&item));
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


/////////////////////////////////////////////////////////



CV12VersionAbstraction::CV12VersionAbstraction()
    : m_pItem(new V12::CRawRingItem),
      m_pOutputItem(nullptr)
{}

CV12VersionAbstraction::~CV12VersionAbstraction()
{
    if (m_pOutputItem != m_pItem) {
        // this is legal if m_pOutputItem == nullptr
        delete m_pOutputItem;
    }

    delete m_pItem;
}

void CV12VersionAbstraction::readDatum(CDataSource &source)
{
    readItem(source, *m_pItem);
}

void CV12VersionAbstraction::processDatum()
{
    m_pOutputItem = m_pFilter->handleItem(m_pItem);
}


void CV12VersionAbstraction::outputDatum(CDataSink& sink)
{
    if (m_pOutputItem) {
        writeItem(sink, *m_pOutputItem);
    }
}

uint32_t CV12VersionAbstraction::getDatumType() const
{
    if (m_pOutputItem) {
        return m_pOutputItem->type();
    } else {
        return m_pItem->type();
    }
}

void CV12VersionAbstraction::cleanup()
{
    if (m_pOutputItem != m_pItem) {
        delete m_pOutputItem;
        m_pOutputItem = nullptr;
    }

    m_pItem->setType(V12::VOID);
}


V12::CRawRingItem
CV12VersionAbstraction::dispatch(V12::CRawRingItem &item)
{

}


/////////////////////////////////////////////////////////////

CFilterMediator::CFilterMediator()
{
}


void CFilterMediator::mainLoop()
{

    // Dereference our pointers before entering
    // the main loop
    CDataSource& source = *getDataSource();
    CDataSink& sink = *getDataSink();

    while (1) {
        auto action = m_pPredicate->preInputUpdate(*this);
        if (action == CPredicatedMediator::SKIP) {
            continue;
        } else if (action == CPredicatedMediator::ABORT) {
            break;
        }

        m_pVsnAbstraction->readDatum(source);

        action = m_pPredicate->postInputUpdate(*this, pItem->type());
        if (action == CPredicatedMediator::SKIP) {
            continue;
        } else if (action == CPredicatedMediator::ABORT) {
            break;
        }

        m_pVsnAbstraction->processDatum();

        // Only send an item if it is not null.
        // The user could return null to prevent sending data
        // to the sink
        action = m_pPredicate->preOutputUpdate(*this, m_pVsnAbstraction->getDatumType());
        if (action == CPredicatedMediator::SKIP) {
            continue;
        } else if (action == CPredicatedMediator::ABORT) {
            break;
        }

        m_pVsnAbstraction->outputDatum(sink);

        action = m_pPredicate->postOutputUpdate(*this, pNewItem->type());
        if (action == CPredicatedMediator::SKIP) {
            continue;
        } else if (action == CPredicatedMediator::ABORT) {
            break;
        }

        m_pVsnAbstraction->cleanup();
    }

}

void CFilterMediator::initialize()
{
  getFilter()->initialize();
}

void CFilterMediator::finalize()
{
  getFilter()->finalize();
}

std::shared_ptr<CPredicate> CFilterMediator::getPredicate() const
{
    return m_pPredicate;
}

void CFilterMediator::setPredicate(std::shared_ptr<CPredicate> pPredicate)
{
    m_pPredicate = pPredicate;
}

} // end Transform
} // end DAQ
