#include <V12/CFilterAbstraction.h>
#include <V12/COneShotLogicFilter.h>

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

/*!
 * \brief CFilterAbstraction::CFilterAbstraction
 *
 * Constructs an empty abstraction with all data elements default initialized.
 */
CFilterAbstraction::CFilterAbstraction()
    : m_item(),
      m_pInputItem(nullptr),
      m_pOutputItem(nullptr),
      m_pFilter(new CCompositeFilter),
      m_predicate()
{}

/*!
 * \brief Destructor
 */
CFilterAbstraction::~CFilterAbstraction()
{
}

/*!
 * \brief CFilterAbstraction::CFilterAbstraction
 *
 * \param rhs   the object to copy
 *
 * The copy operation performs a deep copy of the pointers involved. As
 * a result, there will be no shared resources between rhs and the object.
 *
 */
CFilterAbstraction::CFilterAbstraction(const CFilterAbstraction &rhs)
    : m_item(rhs.m_item),
      m_pInputItem(),
      m_pOutputItem(),
      m_pFilter(new CCompositeFilter(*rhs.m_pFilter)),
      m_predicate(rhs.m_predicate)
{
    if (rhs.m_pInputItem) {
        m_pInputItem = rhs.m_pInputItem->clone();
    }

    if (rhs.m_pOutputItem) {
        m_pOutputItem = rhs.m_pOutputItem->clone();
    }
}

/*!
 * \brief Assignment operator
 *
 * \param rhs   the object to copy
 *
 * \return  reference to this
 *
 * Just like in the copy constructor, deep copies are made of each pointer.
 */
CFilterAbstraction&
CFilterAbstraction::operator=(const CFilterAbstraction &rhs)
{
    if (this != &rhs) {
        m_item = rhs.m_item;


        if (rhs.m_pInputItem) {
            m_pInputItem = rhs.m_pInputItem->clone();
        } else {
            // don't forget to set it to null if necessary
            m_pInputItem = nullptr;
        }

        if (rhs.m_pOutputItem) {
            m_pOutputItem = rhs.m_pOutputItem->clone();
        } else {
            m_pOutputItem = nullptr;
        }

        m_pFilter.reset(new CCompositeFilter(*rhs.m_pFilter));
        m_predicate = rhs.m_predicate;
    }

    return *this;
}


/*!
 * \brief CFilterAbstraction::readDatum
 *
 * \param source    the data source
 *
 * Performs a conditional read from source. Data types that have been
 * passed to the predicate via the setExcludeList() method will be
 * skipped over.
 */
void CFilterAbstraction::readDatum(CDataSource &source)
{
    // Read data into the raw ring item if it meets the predicate condition
    readItemIf(source, m_item, m_predicate);
}

/*!
 * \brief CFilterAbstraction::processDatum
 *
 * From the raw ring item, create a specialized ring item using the
 * factory. The specialized ring item is then passed to the dispatch()
 * method to further passed to the appropriate handler of the filter.
 * The returned ring item is referred to as the "output" ring item.
 */
void CFilterAbstraction::processDatum()
{
    m_pInputItem  = CRingItemFactory::createRingItem(m_item);
    m_pOutputItem = dispatch(m_pInputItem);
}


/*!
 * \brief CFilterAbstraction::outputDatum
 *
 * \param sink  the data sink
 *
 * If the output ring item is not a nullptr, write it to the data sink.
 */
void CFilterAbstraction::outputDatum(CDataSink& sink)
{
    if (m_pOutputItem) {
        writeItem(sink, *m_pOutputItem);
    }
}

/*!
 * \brief CFilterAbstraction::getDatumType
 *
 * \return the type of the current item
 *
 * If processDatum() method has not yet been called since the previous
 * call to cleanUp(), the type returned is from the "input item". Otherwise,
 * the type returned is that of the output ring item (the item returned from
 * the filter). Note that if the filter returns a nullptr, the type returned
 * will be that of the input item. Also, the type may
 * be V12::UNDEFINED if readDatum() has not been called since the previous
 * call to cleanUp().
 */
uint32_t CFilterAbstraction::getDatumType() const
{
    if (m_pOutputItem) {
        return m_pOutputItem->type();
    } else {
        return m_item.type();
    }
}

/*!
 * \brief CFilterAbstraction::cleanUp
 *
 * Release our claim of ownership on the input and output items. If no
 * other objects maintain a claim of ownership, the objects will be deleted.
 */
void CFilterAbstraction::cleanUp()
{
    m_pInputItem.reset();
    m_pOutputItem.reset();
}


/*!
 * \brief CFilterAbstraction::initialize
 *
 * Delegates to the filter's initialize() method.
 */
void CFilterAbstraction::initialize()
{
    m_pFilter->initialize();
}

/*!
 * \brief CFilterAbstraction::finalize
 *
 * Delegates to the filter's finalize() method.
 */
void CFilterAbstraction::finalize()
{
    m_pFilter->finalize();
}

/*!
 * \brief CFilterAbstraction::setFilterMediator
 *
 * \param mediator  reference to the mediator
 *
 * A non-owning reference to the filter mediator is cached. This is called
 * automatically when the user calls CFilterMediator::setVersionAbstraction().
 */
void CFilterAbstraction::setFilterMediator(CFilterMediator &mediator)
{
    m_pMediator = &mediator;
}

/*!
 * \brief CFilterAbstraction::getFilterMediator
 *
 * \return pointer to the cached filter mediator reference
 */
CFilterMediator* CFilterAbstraction::getFilterMediator()
{
    return m_pMediator;
}


int CFilterAbstraction::getMajorVersion() const
{
    return 12;
}


void CFilterAbstraction::setOneShotMode(int nSources)
{
    COneShotLogicFilterPtr pOneShot(new COneShotLogicFilter(nSources, *this));

    CCompositeFilter::FilterContainer& filters = m_pFilter->getFilters();

    filters.insert(filters.begin(), pOneShot);
}

/*!
 * \brief CFilterAbstraction::setExcludeList
 *
 * \param excludeList   a list of comma separated integers or type names
 *
 * The list of types will be parsed using the V12::stringListToIntegers() function
 * and must match the format specified in that documentation. For convienence, the
 * accepted type names are listed here: BEGIN_RUN, END_RUN, PAUSE_RUN, RESUME_RUN,
 * ABNORMAL_ENDRUN, RING_FORMAT, MONITORED_VARIABLES, PACKET_TYPES, PERIODIC_SCALERS,
 * PHYSICS_EVENT, PHYSICS_EVENT_COUNT, EVB_GLOM_INFO, and all composite types associated
 * with these. A composite type is formed by adding the COMP_ prefix to the name. For example,
 * the composite type of BEGIN_RUN is COMP_BEGIN_RUN.
 *
 * The types listed in the argument will be skipped over when reading data. They will never
 * be passed to a filter handler.
 */
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

/*!
 * \brief CFilterAbstraction::setSampleList
 *
 * \param sampleList    a list of types to sample
 *
 * When a type is sampled, the program can choose to skip over it and not return it
 * to the user when space is tight in the data sink. Currently, sampling logic is not
 * supported in the filter framework.
 */
void CFilterAbstraction::setSampleList(const std::string &sampleList)
{
    // we currently don't support sampling
}


/*!
 * \brief CFilterAbstraction::dispatch
 *
 * \param pItem     a shared pointer of a specialized ring item
 *
 * \return  the output of the appropriate handler method
 *
 * The pointer passed in is upcasted to the appropriate specialized ring item type
 * (i.e. not a CRawRingItem) and then passed to the appropriate handler method.
 *
 * \throws std::runtime_error if the composite filter does not exist (which should
 * never actually happen).
 */
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

  case COMP_BEGIN_RUN:
  case COMP_END_RUN:
  case COMP_PAUSE_RUN:
  case COMP_RESUME_RUN:
  case COMP_PACKET_TYPES:
  case COMP_MONITORED_VARIABLES:
  case COMP_PERIODIC_SCALERS:
  case COMP_PHYSICS_EVENT:
  case COMP_PHYSICS_EVENT_COUNT:
  case COMP_ABNORMAL_ENDRUN:
  case COMP_RING_FORMAT:
  case COMP_EVB_GLOM_INFO:
      pFilteredItem = m_pFilter->handleCompositeItem(std::static_pointer_cast<CCompositeRingItem>(pItem));
      break;

      // Handle any other generic ring item...this can be
      // the hook for handling user-defined items
  default:
      pFilteredItem = m_pFilter->handleRingItem(pItem);
      break;
  }

  return pFilteredItem;
}

/*!
 * \brief Register a user's filter to the composite filter
 *
 * \param pFilter   the filter to append
 *
 * Note that contrary to in the past, the filter that is passed in is actually stored
 * in the composite filter. This means that anything the user does to the filter
 * passed in as an argument after calling this method, will affect the filter that is
 * used in processing.
 */
void CFilterAbstraction::registerFilter(CFilterPtr pFilter)
{
    m_pFilter->registerFilter(pFilter);
}

/*!
 * \return the composite filter owned by this class
 */
CCompositeFilterPtr CFilterAbstraction::getFilter() const
{
    return m_pFilter;
}


} // end V12
} // end DAQ
