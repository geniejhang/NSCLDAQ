#include "COneShotLogicFilter.h"

#include <V11/DataFormat.h>
#include <V11/CFilterAbstraction.h>
#include <CFilterMediator.h>

namespace DAQ {
namespace V11 {

COneShotLogicFilter::COneShotLogicFilter(int nSources, CFilterAbstraction& abstraction)
    : m_handler(nSources, BEGIN_RUN, END_RUN, {BEGIN_RUN, END_RUN, PAUSE_RUN, RESUME_RUN}),
      m_pAbstraction(&abstraction)
{
}


/*!
 * \brief Logic for all ring items besides state change items
 *
 * \param pItem     a pointer to a ring item
 *
 * \retval nullptr  if waiting for begin
 * \retval pointer to the original item if not waiting for begin
 */
CRingItem* COneShotLogicFilter::handleRingItem(CRingItem* pItem)
{
    if (m_handler.waitingForBegin()) {
        return nullptr;
    } else {
        return pItem;
    }
}

/*!
 * \brief COneShotLogicFilter::handleAbnormalEndItem
 * \param pItem     pointer to the item to process
 * \return the same pointer item that was passed in
 */
CRingItem* COneShotLogicFilter::handleAbnormalEndItem(CAbnormalEndItem* pItem)
{
    // because the abnormal end run logic is handled in a different filter,
    // we need to make sure that it gets passed on.
    return pItem;
}

/*!
 * \brief Handle the data format item
 *
 * \param pItem a pointer to a data format item
 *
 * \return  the same pointer that was passed in
 */
CRingItem* COneShotLogicFilter::handleDataFormatItem(CDataFormatItem *pItem)
{
    return pItem;
}

CRingItem* COneShotLogicFilter::handleFragmentItem(CRingFragmentItem *pItem)
{
    return handleRingItem(pItem);
}

CRingItem* COneShotLogicFilter::handleGlomParameters(CGlomParameters *pItem)
{
    return handleRingItem(pItem);
}

CRingItem* COneShotLogicFilter::handlePhysicsEventCountItem(CRingPhysicsEventCountItem *pItem)
{
    return handleRingItem(pItem);
}

CRingItem* COneShotLogicFilter::handlePhysicsEventItem(CPhysicsEventItem *pItem)
{
    return handleRingItem(pItem);
}

CRingItem* COneShotLogicFilter::handleScalerItem(CRingScalerItem *pItem)
{
    return handleRingItem(pItem);
}


/*!
 * \brief Handle state change items
 *
 * \param pItem    a pointer to a ring item
 *
 * \return pointer to a ring item
 * \retval nullptr if waiting for begin and pItem does not refer to a begin type
 * \retval pointer to the original item passed in otherwise
 *
 * \throws COneShotException if run number changes
 * \throws COneShotException if more begin run items are observed than are expected
 *
 * If pItem points to an item that completes the oneshot logic, the filter mediator
 * will be told to abort after outputting the result of the filter processing.
 */
CRingItem* COneShotLogicFilter::handleStateChangeItem(CRingStateChangeItem *pItem)
{
    m_handler.update(pItem->type(), pItem->getRunNumber());

    if (m_handler.complete()) {
        // this will cause the mediator to stop after outputting this event.
        // not that if this is in a composite filter, any filter that is after
        // this filter will still process this item. However, no new items
        // will be processed.
        CFilterMediator* pMediator = m_pAbstraction->getFilterMediator();
        if (pMediator) {
            pMediator->setAbort();
        }

        return pItem;
    } else {

        if (m_handler.waitingForBegin() && (pItem->type() != BEGIN_RUN)) {
            return nullptr;
        } else {
            return pItem;
        }
    }
}

CRingItem* COneShotLogicFilter::handleTextItem(CRingTextItem *pItem)
{
    return handleRingItem(pItem);
}

const COneShotHandler& COneShotLogicFilter::getOneShotLogic() const {
    return m_handler;
}

} // end V11
} // end DAQ
