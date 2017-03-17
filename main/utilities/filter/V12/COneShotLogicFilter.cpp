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


CRingItemPtr COneShotLogicFilter::handleRingItem(CRingItemPtr pItem)
{
   return handleItem(pItem);
}

CAbnormalEndItemPtr COneShotLogicFilter::handleAbnormalEndItem(CAbnormalEndItemPtr pItem)
{
    // because the abnormal end run logic is handled in a different filter,
    // we need to make sure that it gets passed on.
    return pItem;
}

CDataFormatItemPtr COneShotLogicFilter::handleDataFormatItem(CDataFormatItemPtr pItem)
{
    return pItem;
}

CGlomParametersPtr COneShotLogicFilter::handleGlomParameters(CGlomParametersPtr pItem)
{
    return handleItem(pItem);
}

CRingPhysicsEventCountItemPtr
COneShotLogicFilter::handlePhysicsEventCountItem(CRingPhysicsEventCountItemPtr pItem)
{
    return handleItem(pItem);
}

CPhysicsEventItemPtr COneShotLogicFilter::handlePhysicsEventItem(CPhysicsEventItemPtr pItem)
{
    return handleItem(pItem);
}

CRingScalerItemPtr COneShotLogicFilter::handleScalerItem(CRingScalerItemPtr pItem)
{
    return handleItem(pItem);
}

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

CRingTextItemPtr COneShotLogicFilter::handleTextItem(CRingTextItemPtr pItem)
{
    return handleItem(pItem);
}

CCompositeRingItemPtr COneShotLogicFilter::handleCompositeItem(CCompositeRingItemPtr pItem)
{
    return handleItem(pItem);
}

const COneShotHandler& COneShotLogicFilter::getOneShotLogic() const {
    return m_handler;
}

} // end V11
} // end DAQ
