#ifndef DAQ_TRANSFORM_CTRANSFORM11P0TO12P0_H
#define DAQ_TRANSFORM_CTRANSFORM11P0TO12P0_H

#include <V12/CRingStateChangeItem.h>
#include <V12/CRingScalerItem.h>
#include <V12/CRingTextItem.h>
#include <V12/CPhysicsEventItem.h>
#include <V12/CRingPhysicsEventCountItem.h>
#include <V12/CAbnormalEndItem.h>
#include <V12/CDataFormatItem.h>
#include <V12/CGlomParameters.h>
#include <V12/CRawRingItem.h>

namespace DAQ {

namespace V11 {
class CRingItem;
}

namespace Transform {


/*!
 * \brief The CTransform11p0to12p0 class
 *
 * The transformation of the data format from version 11.0 to 12.0
 * is defined in this class. There is a call operator that will dispatch
 * to the flow of control to the appropriate transformation logic based
 * on the ring item type. Otherwise, the user can call specific transformation
 * methods that operate on specific types.
 *
 */
class CTransform11p0to12p0
{
public:
    using InitialType = V11::CRingItem;
    using FinalType   = V12::CRawRingItem;

public:
    /*!
     * \brief Call operator
     *
     * \param item  a serialized 11.0 data item
     *
     * \return an appropriate 12.0 data item
     *
     * This method is the entry point for just about anyone who is using
     * this class. There is a switch table that will call the correct transformation
     * depending on the type of the 11.0 item.
     *
     */
    FinalType operator()(const InitialType& item);

    /*!
     * \brief Transform state change items
     *
     * \param item  a serialized 11.0 state change item
     *
     * \return an 12.0 state change item
     *
     * The transform logic is as follows:
     *
     * - If no body header exists, the V12 event timestamp becomes V12::NULL_TIMESTAMP
     * - If no body header exists, the V12 source id becomes 0
     * - If a body header exists, the V12 event timestamp is the same as the V11 event timestamp
     * - If a body header exists, the V12 source id is the same as the V11 source id
     * - The type transforms as follows:
     *      * V11::BEGIN_RUN --> V12::BEGIN_RUN
     *      * V11::END_RUN --> V12::END_RUN
     *      * V11::PAUSE_RUN --> V12::PAUSE_RUN
     *      * V11::RESUME_RUN --> V12::RESUME_RUN
     * - V11 run number becomes V12 run number
     * - V11 time offset becomes V12 time offset
     * - V11 title length is added based on the length of the V11 title length
     * - V11 title becomes V12 title.
     * - V11 time divisor becomes V12 time divisor.
     *
     */
    V12::CRingStateChangeItem transformStateChange(const InitialType& item);

    /*!
     * \brief Transform textual items
     *
     * \param item  a serialized 11.0 text item
     *
     * \return an 12.0 text item
     *
     * The transform logic is as follows:
     *
     * - If no body header exists, the V12 event timestamp becomes V12::NULL_TIMESTAMP
     * - If no body header exists, the V12 source id becomes 0
     * - If a body header exists, the V12 event timestamp is the same as the V11 event timestamp
     * - If a body header exists, the V12 source id is the same as the V11 source id
     * - The type transforms as:
     *      * V11::PACKET_TYPES --> V12::PACKET_TYPES
     *      * V11::MONITORED_VARIABLES --> V12::MONITORED_VARIABLES
     * - V11 time offset becomes V12 time offset
     * - V11 unix timestamp becomes V12 unix timestamp
     * - V11 string count becomes V12 string count
     * - V11 offset divisor becomes V12 offset divisor
     * - V11 strings become V12 strings
     */
    V12::CRingTextItem       transformText(const InitialType& item);

    /*!
     * \brief Transform data format items
     *
     * \param item  a serialized 11.0 data format item
     * \return a V12 data format item
     *
     * This always outputs the same default V12::CDataFormatItem
     * regardless of what the input is.
     */
    V12::CDataFormatItem     transformDataFormatItem(const InitialType& item);

    /*!
     * \brief Transform physics event items
     *
     * \param item  a serialized 11.0 physics event item
     *
     * \return a V12 physics event item
     *
     * The transform logic for this is :
     * - If no body header exists, the V12 event timestamp becomes V12::NULL_TIMESTAMP
     * - If no body header exists, the V12 source id becomes 0
     * - If a body header exists, the V12 event timestamp is the same as the V11 event timestamp
     * - If a body header exists, the V12 source id is the same as the V11 source id
     * - The V11::PHYSICS_EVENT types becomes V12::PHYSICS_EVENT type
     * - V11 body is copied without change to the V12 body
     */
    V12::CPhysicsEventItem   transformPhysicsEventItem(const InitialType& item);

    /*!
     * \brief Transform event count items
     *
     * \param item  a serialized 11.0 event count item
     *
     * \return an 12.0 event count item
     *
     * The logic of the transform is:
     * - If no body header exists, the V12 event timestamp becomes V12::NULL_TIMESTAMP
     * - If no body header exists, the V12 source id becomes 0
     * - If a body header exists, the V12 event timestamp is the same as the V11 event timestamp
     * - If a body header exists, the V12 source id is the same as the V11 source id
     * - The V11::PHYSICS_EVENT_COUNT types becomes V12::PHYSICS_EVENT_COUNT type
     * - V11 time offset becomes the V12 time offset
     * - V11 time offset divisor becomes the V12 time offset divisor
     * - V11 unix timestamp becomes the V12 unix timestamp
     * - V11 event count becomes the V12 event count
     */
    V12::CRingPhysicsEventCountItem
                             transformPhysicsEventCountItem(const InitialType& item);

    /*!
     * \brief Transform scaler items
     *
     * \param item  a serialized 11.0 scaler item
     *
     * \return an 12.0 scaler item
     *
     * The transform logic for this is:
     * - If no body header exists, the V12 event timestamp becomes V12::NULL_TIMESTAMP
     * - If no body header exists, the V12 source id becomes 0
     * - If a body header exists, the V12 event timestamp is the same as the V11 event timestamp
     * - If a body header exists, the V12 source id is the same as the V11 source id
     * - The V11::PERIODIC_SCALER type becomes V12::PERIODIC_SCALER type
     * - V11 interval start offset becomes the V12 interval start offset
     * - V11 interval end offset becomes the V12 interval end offset
     * - V11 unix timestamp becomes the V12 unix timestamp
     * - V11 interval divisor becomes the V12 interval divisor
     * - V11 scaler count becomes the V12 scaler count
     * - V11 incremental state becomes the V12 incremental state
     * - V11 scaler width is set to 32.
     * - V11 scalers become the V12 scalers
     */
    V12::CRingScalerItem     transformScalerItem(const InitialType& item);

    /*!
     * \brief Transform glom parameter types
     *
     * \param item  a serialized 12.0 glom type
     *
     * \return an 11.0 glom parameters item
     *
     * The transform logic is as follows:
     * - V11 item is outputted with no body header
     * - V12 event timestamp is discarded
     * - V12 source id is discarded
     * - V12 coincidence ticks interval becomes V11 coincidence ticks
     * - V12 is building status becomes V11 is building status
     * - V12 timestamp policy becomes the V11 timestamp policy
     *   (last --> last, first --> first, average --> average)
     */
    V12::CGlomParameters     transformGlomParameters(const InitialType& item);

    /*!
     * \brief Transform abnormal end run items
     *
     * \param item  a serialized 12.0 abnormal end run item
     *
     * \return  an 11.0 abnormal end run item
     *
     * The transform logic is as follows:
     * - V11 item outputted with a body header
     * - V12 event timestamp becomes V11 event timestamp
     * - V12 source id becomes V11 source id
     * - V11 barrier type becomes 0
     */
    V12::CAbnormalEndItem    transformAbnormalEndItem(const InitialType& item);

    /*!
     * \brief Transform an EVB Frament (i.e. fragment with ring item payload)
     *
     * \param item  the generic ring item of type EVB_FRAGMENT
     *
     * \return  a V12::PHYSICS_EVENT item
     *
     * The logic for this is the same as transformPhysicsEvent.
     * - The V11 event timestamp becomes the V12 event timestamp
     * - The V11 source id becomes the V12 source id
     * - The V11 payload (i.e. body) becomes the V12 body
     * - The V11::EVB_FRAGMENT type becomes V12::PHYSICS_EVENT
     */
    V12::CPhysicsEventItem transformFragment(const InitialType& item);


    /*!
     * \brief Transform an EVB Frament of unknown payload
     *
     * \param item  the generic ring item of type EVB_UNKNOWN_PAYLOAD
     *
     * \return  a V12::PHYSICS_EVENT item
     *
     * The logic for this is the same as transformPhysicsEvent.
     * - The V11 event timestamp becomes the V12 event timestamp
     * - The V11 source id becomes the V12 source id
     * - The V11 payload (i.e. body) becomes the V12 body
     * - The V11::EVB_UNKNOWN_PAYLOAD type becomes V12::PHYSICS_EVENT
     */
    V12::CPhysicsEventItem transformUnknownFragment(const InitialType& item);
};

} // end Transform
} // end DAQ


#endif // DAQ_TRANSFORM_CTRANSFORM11P0TO12P0_H
