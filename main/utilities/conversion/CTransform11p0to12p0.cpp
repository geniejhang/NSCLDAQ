#include "CTransform11p0to12p0.h"

#include <V11/CRingScalerItem.h>
#include <V11/CRingStateChangeItem.h>
#include <V11/CRingTextItem.h>
#include <V11/CPhysicsEventItem.h>
#include <V11/CRingPhysicsEventCountItem.h>
#include <V11/CGlomParameters.h>
#include <V11/CRingFragmentItem.h>
#include <V11/CUnknownFragment.h>

#include <V12/CRawRingItem.h>
#include <V12/DataFormat.h>

#include <ByteBuffer.h>

#include <stdexcept>

namespace DAQ {
namespace Transform {



V12::CRingStateChangeItem
CTransform11p0to12p0::transformStateChange(const InitialType& item)
{
    V11::CRingStateChangeItem v11(item);

    if (v11.hasBodyHeader()) {
      return V12::CRingStateChangeItem(v11.getEventTimestamp(),
                                     v11.getSourceId(),
                                     v11.type(),
                                     v11.getRunNumber(),
                                     v11.getElapsedTime(),
                                     v11.getTimestamp(),
                                     v11.getTitle(),
                                     v11.getOffsetDivisor());
    } else {
      return V12::CRingStateChangeItem(V12::NULL_TIMESTAMP,
                                     0,
                                     v11.type(),
                                     v11.getRunNumber(),
                                     v11.getElapsedTime(),
                                     v11.getTimestamp(),
                                     v11.getTitle(),
                                     v11.getOffsetDivisor());
    }
}

V12::CRingTextItem
CTransform11p0to12p0::transformText(const InitialType& item)
{
    V11::CRingTextItem v11(item);

    uint32_t type = 0;
    if (v11.type() == V11::MONITORED_VARIABLES) {
        type = V12::MONITORED_VARIABLES;
    } else {
        type = V12::PACKET_TYPES;
    }

    if (item.hasBodyHeader()) {
      return V12::CRingTextItem(type,
                                v11.getEventTimestamp(),
                                v11.getSourceId(),
                                v11.getStrings(),
                                v11.getTimeOffset(),
                                v11.getTimestamp(),
                                v11.getTimeDivisor());
    } else {
      return V12::CRingTextItem(type,
                                V12::NULL_TIMESTAMP,
                                0, 
                                v11.getStrings(),
                                v11.getTimeOffset(),
                                v11.getTimestamp(),
                                v11.getTimeDivisor());
    }
}

V12::CDataFormatItem
CTransform11p0to12p0::transformDataFormatItem(const InitialType& item)
{
    // There is no transformation logic. An 11.0 data format item 
    // becomes a generic 12.0
    // format item, because the 12.0 data format items are all the same.
    return V12::CDataFormatItem();
}

V12::CPhysicsEventItem
CTransform11p0to12p0::transformPhysicsEventItem(const InitialType& item)
{

    Buffer::ByteBuffer buffer;

    auto pBody = reinterpret_cast<uint8_t*>(item.getBodyPointer());
    buffer.insert(buffer.end(), pBody, pBody + item.getBodySize());

    if (item.hasBodyHeader()) {
      return V12::CPhysicsEventItem(item.getEventTimestamp(),
                                      item.getSourceId(),
                                      buffer);
    } else {
      return V12::CPhysicsEventItem(V12::NULL_TIMESTAMP,
                                      0,
                                      buffer);
    }
}

V12::CRingPhysicsEventCountItem
CTransform11p0to12p0::transformPhysicsEventCountItem(const InitialType& item)
{
    V11::CRingPhysicsEventCountItem v11(item);

    if (item.hasBodyHeader()) {
      return V12::CRingPhysicsEventCountItem(v11.getEventTimestamp(),
                                             v11.getSourceId(),
                                             v11.getEventCount(),
                                             v11.getTimeOffset(),
                                             v11.getTimestamp(),
                                             v11.getTimeDivisor());
    } else {
      return V12::CRingPhysicsEventCountItem(V12::NULL_TIMESTAMP,
                                             0,
                                             v11.getEventCount(),
                                             v11.getTimeOffset(),
                                             v11.getTimestamp(),
                                             v11.getTimeDivisor());
    } 
}


V12::CRingScalerItem
CTransform11p0to12p0::transformScalerItem(const InitialType& item)
{
    V11::CRingScalerItem v11(item);

    if (v11.hasBodyHeader()) {

        return V12::CRingScalerItem(v11.getEventTimestamp(),
                                    v11.getSourceId(),
                                    v11.getStartTime(),
                                    v11.getEndTime(),
                                    v11.getTimestamp(),
                                    v11.getScalers(),
                                    v11.getTimeDivisor(),
                                    v11.isIncremental(),
                                    32); // scaler width
    } else {
        return V12::CRingScalerItem(V12::NULL_TIMESTAMP,
                                    0,
                                    v11.getStartTime(),
                                    v11.getEndTime(),
                                    v11.getTimestamp(),
                                    v11.getScalers(),
                                    v11.getTimeDivisor(),
                                    v11.isIncremental(),
                                    32); // scaler width
    }
}

V12::CGlomParameters
CTransform11p0to12p0::transformGlomParameters(const InitialType& item)
{
    V11::CGlomParameters v11(item);

    // the V12::CGlomParameters constructor takes a V12::CGlomParameters::TimestampPolicy,
    // not a V11::CGlomParameters::TimestampPolicy. For that reason, the
    // the mapping between the two must be explicitly done.
    V12::CGlomParameters::TimestampPolicy v12Policy;
    V11::CGlomParameters::TimestampPolicy v11Policy = v11.timestampPolicy();
    if (v11Policy == V11::CGlomParameters::first) {
        v12Policy = V12::CGlomParameters::first;
    } else if (v11Policy == V11::CGlomParameters::last) {
        v12Policy = V12::CGlomParameters::last;
    } else { // v11Policy == average
        v12Policy = V12::CGlomParameters::average;
    }

    return V12::CGlomParameters(v11.coincidenceTicks(),
                                v11.isBuilding(),
                                v12Policy);
}

V12::CAbnormalEndItem
CTransform11p0to12p0::transformAbnormalEndItem(const InitialType& item)
{
    return V12::CAbnormalEndItem();
}

V12::CPhysicsEventItem
CTransform11p0to12p0::transformFragment(const InitialType& item)
{
    V11::CRingFragmentItem v11(item);
    auto pBody = reinterpret_cast<uint8_t*>(v11.getBodyPointer());

    Buffer::ByteBuffer body(pBody, pBody + v11.getBodySize());
    V12::CPhysicsEventItem v12(v11.getEventTimestamp(), v11.getSourceId(),
                               body);

    return v12;
}

V12::CPhysicsEventItem
CTransform11p0to12p0::transformUnknownFragment(const InitialType& item)
{
    V11::CUnknownFragment v11(item);
    auto pBody = reinterpret_cast<uint8_t*>(v11.getBodyPointer());

    Buffer::ByteBuffer body(pBody, pBody + v11.getBodySize());
    V12::CPhysicsEventItem v12(v11.getEventTimestamp(), v11.getSourceId(),
                               body);

    return v12;
}


CTransform11p0to12p0::FinalType
CTransform11p0to12p0::operator()(const InitialType &item)
{
    switch (item.type()) {
    case V11::PERIODIC_SCALERS:
        return transformScalerItem(item);
    case V11::BEGIN_RUN:
    case V11::END_RUN:
    case V11::PAUSE_RUN:
    case V11::RESUME_RUN:
        return transformStateChange(item);
    case V11::PACKET_TYPES:
    case V11::MONITORED_VARIABLES:
        return transformText(item);
    case V11::PHYSICS_EVENT:
        return transformPhysicsEventItem(item);
    case V11::PHYSICS_EVENT_COUNT:
        return transformPhysicsEventCountItem(item);
    case V11::ABNORMAL_ENDRUN:
        return transformAbnormalEndItem(item);
    case V11::EVB_GLOM_INFO:
        return transformGlomParameters(item);
    case V11::EVB_FRAGMENT:
        return transformFragment(item);
    case V11::EVB_UNKNOWN_PAYLOAD:
        return transformUnknownFragment(item);
    case V11::RING_FORMAT:
        return transformDataFormatItem(item);
    default:
        throw std::invalid_argument("There is no support for transforming the argument from V11 to V12.");
    }
}

} // end Transform
} // end DAQ
