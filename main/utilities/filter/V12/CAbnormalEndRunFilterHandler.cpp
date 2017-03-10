
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

CFilterUPtr CAbnormalEndRunFilterHandler::clone() const {
  return DAQ::make_unique<CAbnormalEndRunFilterHandler>(*this);
}

CAbnormalEndItemPtr
CAbnormalEndRunFilterHandler::handleAbnormalEndItem(CAbnormalEndItemPtr pItem)
{
    return handleAnyRingItem(pItem);
}

CDataFormatItemPtr CAbnormalEndRunFilterHandler::handleDataFormatItem(CDataFormatItemPtr pItem)
{
    return handleAnyRingItem(pItem);
}

CGlomParametersPtr CAbnormalEndRunFilterHandler::handleGlomParameters(CGlomParametersPtr pItem)
{
    return handleAnyRingItem(pItem);
}

CRingPhysicsEventCountItemPtr
CAbnormalEndRunFilterHandler::handlePhysicsEventCountItem(CRingPhysicsEventCountItemPtr pItem)
{
    return handleAnyRingItem(pItem);
}

CPhysicsEventItemPtr CAbnormalEndRunFilterHandler::handlePhysicsEventItem(CPhysicsEventItemPtr pItem)
{
    return handleAnyRingItem(pItem);
}

CRingScalerItemPtr CAbnormalEndRunFilterHandler::handleScalerItem(CRingScalerItemPtr pItem)
{
    return handleAnyRingItem(pItem);
}

CRingStateChangeItemPtr CAbnormalEndRunFilterHandler::handleStateChangeItem(CRingStateChangeItemPtr pItem)
{
    return handleAnyRingItem(pItem);
}

CRingTextItemPtr CAbnormalEndRunFilterHandler::handleTextItem(CRingTextItemPtr pItem)
{
    return handleAnyRingItem(pItem);
}

CCompositeRingItemPtr CAbnormalEndRunFilterHandler::handleCompositeItem(CCompositeRingItemPtr pItem)
{
    return handleAnyRingItem(pItem);
}


} // end V12
} // end DAQ
