
#include <V11/CAbnormalEndRunFilterHandler.h>
#include <V11/CRingItem.h>
#include <V11/DataFormatV11.h>

#include <CDataSink.h>
#include <RingIOV11.h>

#include <stdexcept>

namespace DAQ {
namespace V11 {

CAbnormalEndRunFilterHandler::CAbnormalEndRunFilterHandler(const CAbnormalEndRunFilterHandler& rhs) 
  : m_sink(rhs.m_sink)
{}

// All of the functionality of the other are in the handleRingItem.
CRingItem* 
CAbnormalEndRunFilterHandler::handleRingItem(CRingItem *pItem) {

    if (pItem->type() == ABNORMAL_ENDRUN) {
        writeItem(m_sink, *pItem);

        throw std::runtime_error("Found an abnormal end run item. Shutting down!");
    }

    return pItem;
}


CRingItem* CAbnormalEndRunFilterHandler::handleAbnormalEndItem(CAbnormalEndItem* pItem)
{
    return handleRingItem(pItem);
}

CRingItem* CAbnormalEndRunFilterHandler::handleDataFormatItem(CDataFormatItem *pItem)
{
    return handleRingItem(pItem);
}

CRingItem* CAbnormalEndRunFilterHandler::handleFragmentItem(CRingFragmentItem *pItem)
{
    return handleRingItem(pItem);
}

CRingItem* CAbnormalEndRunFilterHandler::handleGlomParameters(CGlomParameters *pItem)
{
    return handleRingItem(pItem);
}

CRingItem* CAbnormalEndRunFilterHandler::handlePhysicsEventCountItem(CRingPhysicsEventCountItem *pItem)
{
    return handleRingItem(pItem);
}

CRingItem* CAbnormalEndRunFilterHandler::handlePhysicsEventItem(CPhysicsEventItem *pItem)
{
    return handleRingItem(pItem);
}

CRingItem* CAbnormalEndRunFilterHandler::handleScalerItem(CRingScalerItem *pItem)
{
    return handleRingItem(pItem);
}

CRingItem* CAbnormalEndRunFilterHandler::handleStateChangeItem(CRingStateChangeItem *pItem)
{
    return handleRingItem(pItem);
}

CRingItem* CAbnormalEndRunFilterHandler::handleTextItem(CRingTextItem *pItem)
{
    return handleRingItem(pItem);
}


} // end V11
} // end DAQ
