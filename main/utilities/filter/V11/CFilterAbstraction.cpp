#include <V11/CFilterAbstraction.h>

#include <V11/CRingItemFactory.h>
#include <V11/CRingItem.h>
#include <V11/CRingScalerItem.h>
#include <V11/CRingStateChangeItem.h>
#include <V11/CRingTextItem.h>
#include <V11/CPhysicsEventItem.h>
#include <V11/CRingFragmentItem.h>
#include <V11/CRingPhysicsEventCountItem.h>
#include <V11/StringsToIntegers.h>
#include <V11/DataFormatV11.h>

#include <RingIOV11.h>

#include <sstream>
#include <stdexcept>

namespace DAQ {
namespace V11 {

/////////////////////////////////////////////////////////////////

CFilterAbstraction::CFilterAbstraction()
    : m_item(UNDEFINED),
      m_pInputItem(nullptr),
      m_pOutputItem(nullptr),
      m_pFilter(new CCompositeFilter),
      m_predicate()
{}

CFilterAbstraction::~CFilterAbstraction()
{
}

void CFilterAbstraction::readDatum(CDataSource &source)
{
    readItemIf(source, *m_pInputItem, m_predicate);
}

void CFilterAbstraction::processDatum()
{
    m_pInputItem  = CRingItemFactory::createRingItem(m_item);
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
    }

    delete m_pInputItem;

    m_pInputItem = nullptr;
    m_pOutputItem = nullptr;

}


void CFilterAbstraction::initialize()
{
    m_pFilter->initialize();
}


void CFilterAbstraction::finalize()
{
    m_pFilter->finalize();
}

void CFilterAbstraction::setFilterMediator(CFilterMediator &mediator)
{
    m_pMediator = &mediator;
}

CFilterMediator* CFilterAbstraction::getFilterMediator()
{
    return m_pMediator;
}

void CFilterAbstraction::setExcludeList(const std::string &excludeList)
{
    std::vector<int> excludes;
    try {
        excludes = stringListToIntegers(excludeList);
    }
    catch (...) {
        std::stringstream errMsg;
        errMsg << "Invalid value for --exclude, must be a list of item types was: ";
        errMsg << excludeList;
        throw std::invalid_argument(errMsg.str());
    }

    for (auto& type : excludes) {
        m_predicate.addExceptionType(type);
    }
}

void CFilterAbstraction::setSampleList(const std::string &sampleList)
{
    // we currently don't support sampling
}


CRingItem*
CFilterAbstraction::dispatch(CRingItem &item)
{
    if (!m_pFilter) {
        throw std::runtime_error("V11::CFilterAbstraction::dispatch() User must provide a filter prior to dispatching");
    }

    // initial pointer to filtered item
  CRingItem* pFilteredItem = &item;

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

  case ABNORMAL_ENDRUN:
      pFilteredItem = m_pFilter->handleAbnormalEndItem(static_cast<CAbnormalEndItem*>(&item));
      break;

  case RING_FORMAT:
      pFilteredItem = m_pFilter->handleDataFormatItem(static_cast<CDataFormatItem*>(&item));
      break;

  case EVB_GLOM_INFO:
      pFilteredItem = m_pFilter->handleGlomParameters(static_cast<CGlomParameters*>(&item));
      break;

      // Handle any other generic ring item...this can be
      // the hook for handling user-defined items
    default:
      pFilteredItem = m_pFilter->handleRingItem(&item);
      break;
  }

  return pFilteredItem;
}

void CFilterAbstraction::registerFilter(CFilterPtr pFilter)
{
    m_pFilter->registerFilter(pFilter);
}

CFilterPtr CFilterAbstraction::getFilter() const
{
    return m_pFilter;
}


} // end V11
} // end DAQ
