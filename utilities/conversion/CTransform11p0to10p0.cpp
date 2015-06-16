#include "CTransform11p0to10p0.h"

#include <NSCLDAQ10/CRingItemFactory.h>
#include <NSCLDAQ10/CRingScalerItem.h>
#include <NSCLDAQ10/CRingTimestampedRunningScalerItem.h>
#include <NSCLDAQ11/CRingScalerItem.h>
#include <NSCLDAQ10/CRingStateChangeItem.h>
#include <NSCLDAQ11/CRingStateChangeItem.h>
#include <NSCLDAQ10/CPhysicsEventItem.h>
#include <NSCLDAQ11/CPhysicsEventItem.h>
#include <NSCLDAQ10/CRingPhysicsEventCountItem.h>
#include <NSCLDAQ11/CRingPhysicsEventCountItem.h>
#include <NSCLDAQ10/CRingTextItem.h>
#include <NSCLDAQ11/CRingTextItem.h>
#include <NSCLDAQ10/CRingFragmentItem.h>
#include <NSCLDAQ11/CRingFragmentItem.h>
#include <NSCLDAQ10/CUnknownFragment.h>
#include <NSCLDAQ11/CUnknownFragment.h>
#include <NSCLDAQ10/DataFormatV10.h>
#include <NSCLDAQ11/DataFormatV11.h>

#include <stdexcept>
#include <string>

using namespace std;

CTransform11p0to10p0::FinalType
CTransform11p0to10p0::operator()(InitialType& item) {
    return dispatch(item);
}


CTransform11p0to10p0::FinalType
CTransform11p0to10p0::dispatch(InitialType& item)
{

    switch (item.type()) {
    case NSCLDAQ11::PERIODIC_SCALERS:
        return transformScaler(item);
        break;
    case NSCLDAQ11::BEGIN_RUN:
    case NSCLDAQ11::END_RUN:
    case NSCLDAQ11::PAUSE_RUN:
    case NSCLDAQ11::RESUME_RUN:
        return transformStateChange(item);
        break;
    case NSCLDAQ11::PHYSICS_EVENT:
        return transformPhysicsEvent(item);
        break;
    case NSCLDAQ11::PHYSICS_EVENT_COUNT:
        return transformPhysicsEventCount(item);
        break;
    case NSCLDAQ11::MONITORED_VARIABLES:
    case NSCLDAQ11::PACKET_TYPES:
        return transformText(item);
        break;
    case NSCLDAQ11::EVB_FRAGMENT:
        return transformFragment(item);
        break;
    case NSCLDAQ11::EVB_UNKNOWN_PAYLOAD:
        return transformUnknownFragment(item);
        break;
    default:
        std::string errmsg("CTransform11p0to10p0::dispatch()");
        errmsg += "Unsupported type (" + to_string(item.type()) + ") found";
        throw std::runtime_error(errmsg);
        break;
    }
    return CTransform11p0to10p0::FinalType(1);

}


CTransform11p0to10p0::FinalType
CTransform11p0to10p0::transformScaler(NSCLDAQ11::CRingItem& item)
{
    auto& sclrItem = dynamic_cast<NSCLDAQ11::CRingScalerItem&>(item);
    if (sclrItem.isIncremental()) {
        return transformIncrScaler(item);
    } else {
        return transformNonIncrScaler(item);
    }

}


NSCLDAQ10::CRingScalerItem
CTransform11p0to10p0::transformIncrScaler(InitialType &item)
{
    auto& sclrItem = dynamic_cast<NSCLDAQ11::CRingScalerItem&>(item);

    NSCLDAQ10::CRingScalerItem v10item(sclrItem.getScalerCount());

    v10item.setStartTime(sclrItem.getStartTime());
    v10item.setEndTime(sclrItem.getEndTime());
    v10item.setTimestamp(sclrItem.getTimestamp());
    v10item.setScalers(sclrItem.getScalers());

    return v10item;

}


NSCLDAQ10::CRingTimestampedRunningScalerItem
CTransform11p0to10p0::transformNonIncrScaler(InitialType &item)
{
    auto& sclrItem = dynamic_cast<NSCLDAQ11::CRingScalerItem&>(item);

    NSCLDAQ10::CRingTimestampedRunningScalerItem v10item(
                sclrItem.getEventTimestamp(),
                sclrItem.getStartTime(),
                sclrItem.getEndTime(),
                sclrItem.getTimeDivisor(),
                sclrItem.getTimestamp(),
                sclrItem.getScalers()
                );

    return v10item;
}

NSCLDAQ10::CRingStateChangeItem
CTransform11p0to10p0::transformStateChange(InitialType &item)
{
    auto& v11Item = dynamic_cast<NSCLDAQ11::CRingStateChangeItem&>(item);

    NSCLDAQ10::CRingStateChangeItem v10item(v11Item.type(),
                                            v11Item.getRunNumber(),
                                            v11Item.getElapsedTime(),
                                            v11Item.getTimestamp(),
                                            v11Item.getTitle());
    return v10item;
}

NSCLDAQ10::CPhysicsEventItem
CTransform11p0to10p0::transformPhysicsEvent(InitialType& item)
{
    auto& v11Item = dynamic_cast<NSCLDAQ11::CPhysicsEventItem&>(item);
    NSCLDAQ10::CPhysicsEventItem v10item(NSCLDAQ11::PHYSICS_EVENT,
                                         v11Item.getStorageSize());

    auto pBegin11 = reinterpret_cast<char*>(v11Item.getBodyPointer());
    auto pEnd11 = reinterpret_cast<char*>(v11Item.getBodyCursor());

    auto pBegin10 = reinterpret_cast<char*>(v10item.getBodyPointer());
    auto pEnd10 =  std::copy(pBegin11, pEnd11, pBegin10);
    v10item.setBodyCursor(pEnd10);
    v10item.updateSize();

  return v10item;
}

NSCLDAQ10::CRingPhysicsEventCountItem
CTransform11p0to10p0::transformPhysicsEventCount(InitialType& item)
{
    auto& v11Item = dynamic_cast<NSCLDAQ11::CRingPhysicsEventCountItem&>(item);
    NSCLDAQ10::CRingPhysicsEventCountItem v10item(v11Item.getEventCount(),
                                                  v11Item.getTimeOffset(),
                                                  v11Item.getTimestamp());

    return v10item;
}

NSCLDAQ10::CRingFragmentItem
CTransform11p0to10p0::transformFragment(InitialType& item)
{
    auto& v11Item = dynamic_cast<NSCLDAQ11::CRingFragmentItem&>(item);
    NSCLDAQ10::CRingFragmentItem v10item(v11Item.timestamp(),
                                         v11Item.source(),
                                         v11Item.payloadSize(),
                                         v11Item.payloadPointer(),
                                         v11Item.barrierType());

    return v10item;

}

NSCLDAQ10::CUnknownFragment
CTransform11p0to10p0::transformUnknownFragment(InitialType& item)
{
    auto& v11Item = dynamic_cast<NSCLDAQ11::CUnknownFragment&>(item);
    NSCLDAQ10::CUnknownFragment v10item(v11Item.timestamp(),
                                        v11Item.source(),
                                        v11Item.barrierType(),
                                        v11Item.payloadSize(),
                                        v11Item.payloadPointer());

    return v10item;

}

NSCLDAQ10::CRingTextItem
CTransform11p0to10p0::transformText(InitialType& item)
{
    auto& v11Item = dynamic_cast<NSCLDAQ11::CRingTextItem&>(item);
    NSCLDAQ10::CRingTextItem v10item(v11Item.type(),
                                     v11Item.getStrings(),
                                     v11Item.getTimeOffset(),
                                     v11Item.getTimestamp());

    return v10item;
}
