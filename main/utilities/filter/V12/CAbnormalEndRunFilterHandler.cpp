
#include <V12/CAbnormalEndRunFilterHandler.h>
#include <V12/CRingItem.h>
#include <V12/DataFormat.h>

#include <CDataSink.h>
#include <RingIOV12.h>

#include <make_unique.h>

#include <stdexcept>

namespace DAQ {
namespace V12 {

CAbnormalEndRunFilterHandler::CAbnormalEndRunFilterHandler(const CAbnormalEndRunFilterHandler& rhs) 
  : m_sink(rhs.m_sink)
{}

/*!
 * \brief CAbnormalEndRunFilterHandler::clone
 *
 * \return a unique_ptr to a newly allocated copy of this object
 */
CFilterUPtr CAbnormalEndRunFilterHandler::clone() const {
  return DAQ::make_unique<CAbnormalEndRunFilterHandler>(*this);
}

/*!
 * \brief CAbnormalEndRunFilterHandler::handleAbnormalEndItem
 *
 * \param pItem     the ring item to process
 *
 * \return the output of handleAnyRingItem()
 *
 * This just delegates its logic to the handleAnyRingItem() template method.
 */
CAbnormalEndItemPtr
CAbnormalEndRunFilterHandler::handleAbnormalEndItem(CAbnormalEndItemPtr pItem)
{
    return handleAnyRingItem(pItem);
}

/*!
 * \brief CAbnormalEndRunFilterHandler::handleDataFormatItem
 *
 * \param pItem     the ring item to process
 *
 * \return the output of handleAnyRingItem()
 *
 * This just delegates its logic to the handleAnyRingItem() template method.
 */
CDataFormatItemPtr CAbnormalEndRunFilterHandler::handleDataFormatItem(CDataFormatItemPtr pItem)
{
    return handleAnyRingItem(pItem);
}

/*!
 * \brief CAbnormalEndRunFilterHandler::handleGlomParameters
 *
 * \param pItem     the ring item to process
 *
 * \return the output of handleAnyRingItem()
 *
 * This just delegates its logic to the handleAnyRingItem() template method.
 */
CGlomParametersPtr CAbnormalEndRunFilterHandler::handleGlomParameters(CGlomParametersPtr pItem)
{
    return handleAnyRingItem(pItem);
}

/*!
 * \brief CAbnormalEndRunFilterHandler::handlePhysicsEventCountItem
 *
 * \param pItem     the ring item to process
 *
 * \return the output of handleAnyRingItem()
 *
 * This just delegates its logic to the handleAnyRingItem() template method.
 */
CRingPhysicsEventCountItemPtr
CAbnormalEndRunFilterHandler::handlePhysicsEventCountItem(CRingPhysicsEventCountItemPtr pItem)
{
    return handleAnyRingItem(pItem);
}

/*!
 * \brief CAbnormalEndRunFilterHandler::handlePhysicsEventItem
 *
 * \param pItem     the ring item to process
 *
 * \return the output of handleAnyRingItem()
 *
 * This just delegates its logic to the handleAnyRingItem() template method.
 */
CPhysicsEventItemPtr CAbnormalEndRunFilterHandler::handlePhysicsEventItem(CPhysicsEventItemPtr pItem)
{
    return handleAnyRingItem(pItem);
}

/*!
 * \brief CAbnormalEndRunFilterHandler::handleScalerItem
 *
 * \param pItem     the ring item to process
 *
 * \return the output of handleAnyRingItem()
 *
 * This just delegates its logic to the handleAnyRingItem() template method.
 */
CRingScalerItemPtr CAbnormalEndRunFilterHandler::handleScalerItem(CRingScalerItemPtr pItem)
{
    return handleAnyRingItem(pItem);
}

/*!
 * \brief CAbnormalEndRunFilterHandler::handleStateChangeItem
 *
 * \param pItem     the ring item to process
 *
 * \return the output of handleAnyRingItem()
 *
 * This just delegates its logic to the handleAnyRingItem() template method.
 */
CRingStateChangeItemPtr CAbnormalEndRunFilterHandler::handleStateChangeItem(CRingStateChangeItemPtr pItem)
{
    return handleAnyRingItem(pItem);
}

/*!
 * \brief CAbnormalEndRunFilterHandler::handleTextItem
 *
 * \param pItem     the ring item to process
 *
 * \return the output of handleAnyRingItem()
 *
 * This just delegates its logic to the handleAnyRingItem() template method.
 */
CRingTextItemPtr CAbnormalEndRunFilterHandler::handleTextItem(CRingTextItemPtr pItem)
{
    return handleAnyRingItem(pItem);
}

/*!
 * \brief CAbnormalEndRunFilterHandler::handleCompositeItem
 *
 * \param pItem     the ring item to process
 *
 * \return the output of handleAnyRingItem()
 *
 * This just delegates its logic to the handleAnyRingItem() template method.
 */
CCompositeRingItemPtr CAbnormalEndRunFilterHandler::handleCompositeItem(CCompositeRingItemPtr pItem)
{
    return handleAnyRingItem(pItem);
}


} // end V12
} // end DAQ
