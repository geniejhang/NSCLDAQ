#include <V12/CFilterAbstraction.h>

#include <V12/CRingItemFactory.h>
#include <V12/CRingItem.h>
#include <V12/CRingScalerItem.h>
#include <V12/CRingStateChangeItem.h>
#include <V12/CRingTextItem.h>
#include <V12/CPhysicsEventItem.h>
#include <V12/CRingPhysicsEventCountItem.h>
#include <V12/StringsToIntegers.h>
#include <V12/DataFormat.h>

#include <RingIOV12.h>

#include <sstream>
#include <stdexcept>

namespace DAQ {
namespace V12 {

/////////////////////////////////////////////////////////////////

CFilterAbstraction::CFilterAbstraction()
    : m_item(),
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
    readItemIf(source, m_item, m_predicate);
}

void CFilterAbstraction::processDatum()
{
    m_pInputItem  = CRingItemFactory::createRingItem(m_item);
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

void CFilterAbstraction::cleanUp()
{
    m_pInputItem.reset();
    m_pOutputItem.reset();
}


void CFilterAbstraction::initialize()
{
    m_pFilter->initialize();
}


void CFilterAbstraction::finalize()
{
    m_pFilter->finalize();
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


CRingItemPtr
CFilterAbstraction::dispatch(CRingItemPtr pItem)
{
    if (!m_pFilter) {
        throw std::runtime_error("V12::CFilterAbstraction::dispatch() User must provide a filter prior to dispatching");
    }

    // initial pointer to filtered item
  CRingItemPtr pFilteredItem = pItem;

  switch(pItem->type()) {

    // State change items
    case BEGIN_RUN:
    case END_RUN:
    case PAUSE_RUN:
    case RESUME_RUN:
      pFilteredItem = m_pFilter->handleStateChangeItem(std::static_pointer_cast<CRingStateChangeItem>(pItem));
      break;

      // Documentation items
    case PACKET_TYPES:
    case MONITORED_VARIABLES:
      pFilteredItem = m_pFilter->handleTextItem(std::static_pointer_cast<CRingTextItem>(pItem));
      break;

      // Scaler items
    case PERIODIC_SCALERS:
      pFilteredItem = m_pFilter->handleScalerItem(std::static_pointer_cast<CRingScalerItem>(pItem));
      break;

      // Physics event item
    case PHYSICS_EVENT:
      pFilteredItem = m_pFilter->handlePhysicsEventItem(std::static_pointer_cast<CPhysicsEventItem>(pItem));
      break;

      // Physics event count
    case PHYSICS_EVENT_COUNT:
      pFilteredItem = m_pFilter->handlePhysicsEventCountItem(std::static_pointer_cast<CRingPhysicsEventCountItem>(pItem));
      break;

  case ABNORMAL_ENDRUN:
      pFilteredItem = m_pFilter->handleAbnormalEndItem(std::static_pointer_cast<CAbnormalEndItem>(pItem));
      break;

  case RING_FORMAT:
      pFilteredItem = m_pFilter->handleDataFormatItem(std::static_pointer_cast<CDataFormatItem>(pItem));
      break;

  case EVB_GLOM_INFO:
      pFilteredItem = m_pFilter->handleGlomParameters(std::static_pointer_cast<CGlomParameters>(pItem));
      break;

      // Handle any other generic ring item...this can be
      // the hook for handling user-defined items
    default:
      pFilteredItem = m_pFilter->handleRingItem(pItem);
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


} // end V12
} // end DAQ
