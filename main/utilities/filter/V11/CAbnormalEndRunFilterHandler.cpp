
#include <V11/CAbnormalEndRunFilterHandler.h>
#include <V11/CRingItem.h>
#include <V11/DataFormat.h>

#include <CDataSink.h>
#include <RingIOV11.h>

#include <stdexcept>

namespace DAQ {
namespace V11 {

CAbnormalEndRunFilterHandler::CAbnormalEndRunFilterHandler(const CAbnormalEndRunFilterHandler& rhs) 
  : m_sink(rhs.m_sink)
{}

/*! \brief Checks for ABNORMAL_ENDRUN presence
 *
 * If the ring item is an ABNORMAL_ENDRUN, it outputs the item immediately and
 * then throws an exception.
 *
 * \return  the pointer that was passed in
 *
 * \throws std::runtime_error if ring item has type ABNORMAL_ENDRUN
 */
CRingItem* 
CAbnormalEndRunFilterHandler::handleRingItem(CRingItem *pItem) {

    if (pItem->type() == ABNORMAL_ENDRUN) {
        writeItem(m_sink, *pItem);

        throw std::runtime_error("Found an abnormal end run item. Shutting down!");
    }

    return pItem;
}

/*!
 * \brief CAbnormalEndRunFilterHandler::handleAbnormalEndItem
 *
 * \param pItem     the ring item to process
 *
 * \return the output of handleRingItem
 *
 * delegates all logic to handleRingItem.
 */
CRingItem* CAbnormalEndRunFilterHandler::handleAbnormalEndItem(CAbnormalEndItem* pItem)
{
    return handleRingItem(pItem);
}

/*!
* \brief CAbnormalEndRunFilterHandler::handleDataFormatItem
*
* \param pItem     the ring item to process
*
* \return the output of handleRingItem
*
* delegates all logic to handleRingItem.
*/
CRingItem* CAbnormalEndRunFilterHandler::handleDataFormatItem(CDataFormatItem *pItem)
{
    return handleRingItem(pItem);
}

/*!
* \brief CAbnormalEndRunFilterHandler::handleFragmentItem
*
* \param pItem     the ring item to process
*
* \return the output of handleRingItem
*
* delegates all logic to handleRingItem.
*/
CRingItem* CAbnormalEndRunFilterHandler::handleFragmentItem(CRingFragmentItem *pItem)
{
    return handleRingItem(pItem);
}

/*!
* \brief CAbnormalEndRunFilterHandler::handleGlomParameters
*
* \param pItem     the ring item to process
*
* \return the output of handleRingItem
*
* delegates all logic to handleRingItem.
*/
CRingItem* CAbnormalEndRunFilterHandler::handleGlomParameters(CGlomParameters *pItem)
{
    return handleRingItem(pItem);
}

/*!
* \brief CAbnormalEndRunFilterHandler::handlePhysicsEventCountItem
*
* \param pItem     the ring item to process
*
* \return the output of handleRingItem
*
* delegates all logic to handleRingItem.
*/
CRingItem* CAbnormalEndRunFilterHandler::handlePhysicsEventCountItem(CRingPhysicsEventCountItem *pItem)
{
    return handleRingItem(pItem);
}

/*!
* \brief CAbnormalEndRunFilterHandler::handlePhysicsEventItem
*
* \param pItem     the ring item to process
*
* \return the output of handleRingItem
*
* delegates all logic to handleRingItem.
*/
CRingItem* CAbnormalEndRunFilterHandler::handlePhysicsEventItem(CPhysicsEventItem *pItem)
{
    return handleRingItem(pItem);
}

/*!
* \brief CAbnormalEndRunFilterHandler::handleScalerItem
*
* \param pItem     the ring item to process
*
* \return the output of handleRingItem
*
* delegates all logic to handleRingItem.
*/
CRingItem* CAbnormalEndRunFilterHandler::handleScalerItem(CRingScalerItem *pItem)
{
    return handleRingItem(pItem);
}

/*!
* \brief CAbnormalEndRunFilterHandler::handleStateChangeItem
*
* \param pItem     the ring item to process
*
* \return the output of handleRingItem
*
* delegates all logic to handleRingItem.
*/
CRingItem* CAbnormalEndRunFilterHandler::handleStateChangeItem(CRingStateChangeItem *pItem)
{
    return handleRingItem(pItem);
}

/*!
* \brief CAbnormalEndRunFilterHandler::handleTextItem
*
* \param pItem     the ring item to process
*
* \return the output of handleRingItem
*
* delegates all logic to handleRingItem.
*/
CRingItem* CAbnormalEndRunFilterHandler::handleTextItem(CRingTextItem *pItem)
{
    return handleRingItem(pItem);
}


} // end V11
} // end DAQ
