#include "COneShotLogicFilter.h"

#include <V12/DataFormat.h>
#include <V12/CFilterAbstraction.h>
#include <CFilterMediator.h>

namespace DAQ {
namespace V12 {

COneShotLogicFilter::COneShotLogicFilter(int nSources, CFilterAbstraction& abstraction)
    : m_handler(nSources, BEGIN_RUN, END_RUN, {BEGIN_RUN, END_RUN, PAUSE_RUN, RESUME_RUN}),
      m_pAbstraction(&abstraction)
{
}

/*!
 * \brief COneShotLogicFilter::handleRingItem
 *
 * delegates operations to handleItem()
 */
CRingItemPtr COneShotLogicFilter::handleRingItem(CRingItemPtr pItem)
{
   return handleItem(pItem);
}

/*!
 * \brief COneShotLogicFilter::handleAbnormalEndItem
 *
 * \param pItem  an abnormal end item
 *
 * \return the same item that was passed in
 *
 * Abnormal end run items are a bit special and we need to make sure that they are always
 * passed on.
 *
 */
CAbnormalEndItemPtr COneShotLogicFilter::handleAbnormalEndItem(CAbnormalEndItemPtr pItem)
{
    return pItem;
}

/*!
 * \brief COneShotLogicFilter::handleAbnormalEndItem
 *
 * \param pItem  an abnormal end item
 *
 * \return the same item that was passed in
 *
 * Data format items are special and need to always be passed on.
 */
CDataFormatItemPtr COneShotLogicFilter::handleDataFormatItem(CDataFormatItemPtr pItem)
{
    return pItem;
}

/*!
 * \brief COneShotLogicFilter::handleGlomParameters
 *
 * delegates operations to handleItem()
 */
CGlomParametersPtr COneShotLogicFilter::handleGlomParameters(CGlomParametersPtr pItem)
{
    return handleItem(pItem);
}

/*!
 * \brief COneShotLogicFilter::handlePhysicsEventCountItem
 *
 * delegates operations to handleItem()
 */
CRingPhysicsEventCountItemPtr
COneShotLogicFilter::handlePhysicsEventCountItem(CRingPhysicsEventCountItemPtr pItem)
{
    return handleItem(pItem);
}

/*!
 * \brief COneShotLogicFilter::handlePhysicsEventItem
 *
 * delegates operations to handleItem()
 */
CPhysicsEventItemPtr COneShotLogicFilter::handlePhysicsEventItem(CPhysicsEventItemPtr pItem)
{
    return handleItem(pItem);
}

/*!
 * \brief COneShotLogicFilter::handleScalerItem
 *
 * delegates operations to handleItem()
 */
CRingScalerItemPtr COneShotLogicFilter::handleScalerItem(CRingScalerItemPtr pItem)
{
    return handleItem(pItem);
}

/*!
 * \brief COneShotLogicFilter::handleStateChangeItem
 *
 * Most of the logic is done in this method. First of all, the one shot handler
 * is passed the type and run number encapsulated by the state change item passed
 * in as an argument. After that information is used to update the handler,
 * we use the new state to condition the remaining logic. If the current state
 * change item completed the one shot logic (i.e. it was an end run item and
 * caused the number of end run types observed to be equal to the number that was
 * expected), then the filter mediator will be told to abort after processing
 * the current item. The ring item will then be returned. If, on the other hand,
 * the one shot handler's logic is not completed, there action taken will depend
 * on the type of the object. If the current object is not a BEGIN_RUN and we are
 * waiting for a begin run, then nullptr will be returned. Otherwise, the pointer
 * passed in will be returned.
 *
 * \param pItem     a shared pointer to a state change item
 *
 * \returns see the description of the logic for the return value.
 */
CRingStateChangeItemPtr
COneShotLogicFilter::handleStateChangeItem(CRingStateChangeItemPtr pItem)
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

/*!
 * \brief COneShotLogicFilter::handleScalerItem
 *
 * delegates operations to handleItem()
 */
CRingTextItemPtr COneShotLogicFilter::handleTextItem(CRingTextItemPtr pItem)
{
    return handleItem(pItem);
}

/*!
 * \brief COneShotLogicFilter::handleScalerItem
 *
 * delegates operations to handleItem()
 */
CCompositeRingItemPtr COneShotLogicFilter::handleCompositeItem(CCompositeRingItemPtr pItem)
{
    return handleItem(pItem);
}

/*!
 * \return a reference to the oneshot logic
 */
const COneShotHandler& COneShotLogicFilter::getOneShotLogic() const {
    return m_handler;
}

} // end V11
} // end DAQ
