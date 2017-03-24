#include "CTransform12p0to11p0.h"

#include <V12/CRingScalerItem.h>
#include <V12/CRingStateChangeItem.h>
#include <V12/CRawRingItem.h>
#include <V12/CRingTextItem.h>
#include <V12/CPhysicsEventItem.h>
#include <V12/CRingPhysicsEventCountItem.h>
#include <V12/CGlomParameters.h>

#include <V11/CRingItem.h>
#include <V11/DataFormat.h>

#include <stdexcept>

namespace DAQ {
namespace Transform {



V11::CRingStateChangeItem
CTransform12p0to11p0::transformStateChange(const InitialType& item)
{
    V12::CRingStateChangeItem v12(item);
    return V11::CRingStateChangeItem(v12.getEventTimestamp(),
                                     v12.getSourceId(),
                                     v12.type(),
                                     v12.type(),
                                     v12.getRunNumber(),
                                     v12.getElapsedTime(),
                                     v12.getTimestamp(),
                                     v12.getTitle().substr(0, V11_TITLE_MAXSIZE-1),
                                     v12.getOffsetDivisor());
}

V11::CRingTextItem
CTransform12p0to11p0::transformText(const InitialType& item)
{
    V12::CRingTextItem v12(item);

    uint32_t type = 0;
    if (v12.type() == V12::MONITORED_VARIABLES) {
        type = V11::MONITORED_VARIABLES;
    } else {
        type = V11::PACKET_TYPES;
    }

    return V11::CRingTextItem(type,
                              v12.getEventTimestamp(),
                              v12.getSourceId(),
                              0,
                              v12.getStrings(),
                              v12.getTimeOffset(),
                              v12.getTimestamp(),
                              v12.getTimeDivisor());
}

V11::CDataFormatItem
CTransform12p0to11p0::transformDataFormatItem(const InitialType& item)
{
    // There is no transformation logic. A 12.0 item becomes a generic 11.0
    // format item, because the 11.0 data format items are all the same.
    return V11::CDataFormatItem();
}

V11::CPhysicsEventItem
CTransform12p0to11p0::transformPhysicsEventItem(const InitialType& item)
{
    V11::CPhysicsEventItem v11(item.getEventTimestamp(),
                               item.getSourceId(),
                               0, item.size());


    // copy the v12 body into the v11
    auto& v12Body = item.getBody();
    auto pBody = reinterpret_cast<uint8_t*>(v11.getBodyPointer());

    pBody = std::copy(v12Body.begin(), v12Body.end(), pBody);
    v11.setBodyCursor(pBody);
    v11.updateSize();

    return v11;
}

V11::CRingPhysicsEventCountItem
CTransform12p0to11p0::transformPhysicsEventCountItem(const InitialType& item)
{
    V12::CRingPhysicsEventCountItem v12(item);

    return V11::CRingPhysicsEventCountItem(v12.getEventTimestamp(),
                                           v12.getSourceId(),
                                           0,
                                           v12.getEventCount(),
                                           v12.getTimeOffset(),
                                           v12.getTimestamp(),
                                           v12.getTimeDivisor());
}

V11::CRingScalerItem
CTransform12p0to11p0::transformScalerItem(const InitialType& item)
{
    V12::CRingScalerItem v12(item);

    return V11::CRingScalerItem(v12.getEventTimestamp(),
                                v12.getSourceId(),
                                0,
                                v12.getStartTime(),
                                v12.getEndTime(),
                                v12.getTimestamp(),
                                v12.getScalers(),
                                v12.getTimeDivisor(),
                                v12.isIncremental());
}

V11::CGlomParameters
CTransform12p0to11p0::transformGlomParameters(const InitialType& item)
{
    V12::CGlomParameters v12(item);

    // the V11::CGlomParameters constructor takes a V11::CGlomParameters::TimestampPolicy,
    // not a V12::CGlomParameters::TimestampPolicy. For that reason, the
    // the mapping between the two must be explicitly done.
    V11::CGlomParameters::TimestampPolicy v11Policy;
    V12::CGlomParameters::TimestampPolicy v12Policy = v12.timestampPolicy();
    if (v12Policy == V12::CGlomParameters::first) {
        v11Policy = V11::CGlomParameters::first;
    } else if (v12Policy == V12::CGlomParameters::last) {
        v11Policy = V11::CGlomParameters::last;
    } else { // v12Policy == average
        v11Policy = V11::CGlomParameters::average;
    }

    V11::CGlomParameters v11(v12.coincidenceTicks(),
                             v12.isBuilding(),
                             v11Policy);
    return v11;
}

V11::CAbnormalEndItem
CTransform12p0to11p0::transformAbnormalEndItem(const InitialType& item)
{
    V11::CAbnormalEndItem v11;
    v11.setBodyHeader(item.getEventTimestamp(), item.getSourceId(), 0);
    return v11;
}



CTransform12p0to11p0::FinalType
CTransform12p0to11p0::operator()(const InitialType &item)
{
    switch (item.type()) {
    case V12::PERIODIC_SCALERS:
        return transformScalerItem(item);
    case V12::BEGIN_RUN:
    case V12::END_RUN:
    case V12::PAUSE_RUN:
    case V12::RESUME_RUN:
        return transformStateChange(item);
    case V12::PACKET_TYPES:
    case V12::MONITORED_VARIABLES:
        return transformText(item);
    case V12::PHYSICS_EVENT:
        return transformPhysicsEventItem(item);
    case V12::PHYSICS_EVENT_COUNT:
        return transformPhysicsEventCountItem(item);
    case V12::ABNORMAL_ENDRUN:
        return transformAbnormalEndItem(item);
    case V12::EVB_GLOM_INFO:
        return transformGlomParameters(item);
    case V12::RING_FORMAT:
        return transformDataFormatItem(item);
    case V12::COMP_BEGIN_RUN:
    case V12::COMP_END_RUN:
    case V12::COMP_PAUSE_RUN:
    case V12::COMP_RESUME_RUN:
    case V12::COMP_ABNORMAL_ENDRUN:
    case V12::COMP_MONITORED_VARIABLES:
    case V12::COMP_PACKET_TYPES:
    case V12::COMP_PERIODIC_SCALERS:
    case V12::COMP_PHYSICS_EVENT:
    case V12::COMP_PHYSICS_EVENT_COUNT:
    case V12::COMP_EVB_GLOM_INFO:
        return V11::CRingItem(V11::UNDEFINED);
    default:
        throw std::invalid_argument("There is no support for transforming the argument from V12 to V11.");
    }
}

} // end Transform
} // end DAQ
