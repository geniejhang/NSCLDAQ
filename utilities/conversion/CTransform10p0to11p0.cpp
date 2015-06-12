
#include <CTransform10p0to11p0.h>
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
#include <NSCLDAQ10/DataFormatV10.h>
#include <NSCLDAQ11/DataFormatV11.h>

#include <stdexcept>
#include <iostream>

using namespace std;

CTransform10p0to11p0::FinalType
CTransform10p0to11p0::operator()(InitialType& item)
{
  InitialType* pItem = NSCLDAQ10::CRingItemFactory::createRingItem(item);

  return dispatch(*pItem);
}

CTransform10p0to11p0::FinalType
CTransform10p0to11p0::dispatch(InitialType& item)
{
    switch (item.type()) {
    case NSCLDAQ10::INCREMENTAL_SCALERS :
        return transformScaler(item);
        break;
    case NSCLDAQ10::BEGIN_RUN:
    case NSCLDAQ10::END_RUN:
    case NSCLDAQ10::PAUSE_RUN:
    case NSCLDAQ10::RESUME_RUN:
        return transformStateChange(item);
        break;
    case NSCLDAQ10::PHYSICS_EVENT:
        return transformPhysicsEvent(item);
        break;
    case NSCLDAQ10::PHYSICS_EVENT_COUNT:
        return transformPhysicsEventCount(item);
        break;
    case NSCLDAQ10::MONITORED_VARIABLES:
    case NSCLDAQ10::PACKET_TYPES:
        return transformText(item);
        break;
    case NSCLDAQ10::TIMESTAMPED_NONINCR_SCALERS:
        return transformNonIncrScaler(item);
        break;
    case NSCLDAQ10::EVB_FRAGMENT:
        return transformFragment(item);
        break;
    default:
        std::string errmsg("CTransform10p0to11p0::dispatch()");
        errmsg += "Unsupported type (" + to_string(item.type()) + ") found";
        throw std::runtime_error(errmsg);
        break;
    }
    return CTransform10p0to11p0::FinalType(1);
}

CTransform10p0to11p0::FinalType
CTransform10p0to11p0::transformScaler(InitialType& item)
{
    NSCLDAQ10::CRingScalerItem& v10item = dynamic_cast<NSCLDAQ10::CRingScalerItem&>(item);
    NSCLDAQ11::CRingScalerItem v11item(v10item.getScalerCount());

    v11item.setStartTime(v10item.getStartTime());

    v11item.setEndTime(v10item.getEndTime());

    v11item.setTimestamp(v10item.getTimestamp());

    v11item.setScalers(v10item.getScalers());

    return FinalType(v11item);
}

CTransform10p0to11p0::FinalType
CTransform10p0to11p0::transformStateChange(InitialType& item)
{
    NSCLDAQ10::CRingStateChangeItem& v10item = dynamic_cast<NSCLDAQ10::CRingStateChangeItem&>(item);
    NSCLDAQ11::CRingStateChangeItem v11item(v10item.type());

    v11item.setRunNumber(v10item.getRunNumber());

    v11item.setElapsedTime(v10item.getElapsedTime());

    v11item.setTimestamp(v10item.getTimestamp());

    v11item.setOffsetDivisor(1);

    v11item.setTitle(v10item.getTitle());

    return FinalType(v11item);
}

CTransform10p0to11p0::FinalType
CTransform10p0to11p0::transformPhysicsEvent(InitialType& item)
{
    NSCLDAQ10::CPhysicsEventItem& v10item = dynamic_cast<NSCLDAQ10::CPhysicsEventItem&>(item);
    NSCLDAQ11::CPhysicsEventItem v11item(v10item.getStorageSize());

    uint8_t* pBegin10  = reinterpret_cast<uint8_t*>(v10item.getBodyPointer());
    uint8_t* pEnd10    = reinterpret_cast<uint8_t*>(v10item.getBodyCursor());
    uint8_t* pCursor11 = reinterpret_cast<uint8_t*>(v11item.getBodyPointer());

    pCursor11 = std::copy(pBegin10, pEnd10, pCursor11);

    v11item.setBodyCursor(pCursor11);
    v11item.updateSize();

    return FinalType(v11item);
}

CTransform10p0to11p0::FinalType
CTransform10p0to11p0::transformPhysicsEventCount(InitialType& item)
{
    NSCLDAQ10::CRingPhysicsEventCountItem& v10item
            = dynamic_cast<NSCLDAQ10::CRingPhysicsEventCountItem&>(item);

    NSCLDAQ11::CRingPhysicsEventCountItem v11item(v10item.getEventCount(),
                                                  v10item.getTimeOffset(),
                                                  v10item.getTimestamp());

    return FinalType(v11item);
}

CTransform10p0to11p0::FinalType
CTransform10p0to11p0::transformText(InitialType& item)
{
    NSCLDAQ10::CRingTextItem& v10item
            = dynamic_cast<NSCLDAQ10::CRingTextItem&>(item);

    NSCLDAQ11::CRingTextItem v11item(v10item.type(),
                                     v10item.getStrings(),
                                     v10item.getTimeOffset(),
                                     v10item.getTimestamp());
    v11item.setTimeDivisor(1);
    return FinalType(v11item);
}

CTransform10p0to11p0::FinalType
CTransform10p0to11p0::transformNonIncrScaler(InitialType& item)
{
    NSCLDAQ10::CRingTimestampedRunningScalerItem& v10item
            = dynamic_cast<NSCLDAQ10::CRingTimestampedRunningScalerItem&>(item);

    NSCLDAQ11::CRingScalerItem v11item(v10item.getOffsetStart(),
                                       v10item.getOffsetEnd(),
                                       v10item.getCalendarTime(),
                                       v10item.getScalers(),
                                       false,
                                       1);
    return FinalType(v11item);
}

CTransform10p0to11p0::FinalType
CTransform10p0to11p0::transformFragment(InitialType& item)
{
    NSCLDAQ10::CRingFragmentItem& v10item
            = dynamic_cast<NSCLDAQ10::CRingFragmentItem&>(item);

    NSCLDAQ11::CRingFragmentItem v11item(v10item.timestamp(),
                                         v10item.source(),
                                         v10item.payloadSize(),
                                         v10item.payloadPointer(),
                                         v10item.barrierType());

    return FinalType(v11item);
}

