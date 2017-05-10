#ifndef DAQ_TRANSFORM_CTRANSFORM12P0TO11P0_H
#define DAQ_TRANSFORM_CTRANSFORM12P0TO11P0_H

#include <V11/CRingStateChangeItem.h>
#include <V11/CRingScalerItem.h>
#include <V11/CRingTextItem.h>
#include <V11/CPhysicsEventItem.h>
#include <V11/CRingPhysicsEventCountItem.h>
#include <V11/CAbnormalEndItem.h>
#include <V11/CDataFormatItem.h>
#include <V11/CGlomParameters.h>

namespace DAQ {

namespace V12 {
class CRawRingItem;
}

namespace Transform {


/*!
 * \brief The CTransform12p0to11p0 class
 *
 * The tranformation from 12.0 to 11.0 data is defined in this class.
 *
 */
class CTransform12p0to11p0
{
public:
    using InitialType = V12::CRawRingItem;
    using FinalType   = V11::CRingItem;

public:
    /*!
     * \brief Call operator
     *
     * \param item  a serialized 12.0 data item
     *
     * \return an appropriate 11.0 data item
     *
     * This method is the entry point for just about anyone who is using
     * this class. There is a switch table that will call the correct transformation
     * depending on the type of the 12.0 item. All items in 12.0 besides composite
     * types are supported. Any 12.0 composite datum will produce an 11.0
     * data item with type V11::UNDEFINED.
     */
    FinalType operator()(const InitialType& item);

    /*!
     * \brief Transform state change items
     *
     * \param item  a serialized 12.0 state change item
     *
     * \return an 11.0 state change item
     *
     * The transform logic is as follows:
     *
     * - V11 item is outputted with a body header
     * - V12 event timestamp becomes V11 event timestamp
     * - V12 source id becomes V11 source id
     * - V11 barrier type is set to the ring item type
     * - V12 run number becomes V11 run number
     * - V12 time offset becomes V11 time offset
     * - V12 title length is discarded
     * - V12 title becomes V11 title. It is truncated to 80 characters if necessary.
     *   Right padded with zeroes if necessary.
     * - V12 time divisor becomes V11 time divisor.
     *
     */
    V11::CRingStateChangeItem transformStateChange(const InitialType& item);

    /*!
     * \brief Transform textual items
     *
     * \param item  a serialized 12.0 text item
     *
     * \return an 11.0 text item
     *
     * The transform logic is as follows:
     * - V11 item is outputted with a body header
     * - V12 event timestamp becomes V11 event timestamp
     * - V12 source id becomes V11 source id
     * - V11 barrier type will be zero
     * - V12 time offset becomes V11 time offset
     * - V12 unix timestamp becomes V11 unix timestamp
     * - V12 string count becomes V11 string count
     * - V12 offset divisor becomes V11 offset divisor
     * - V12 strings become V11 strings
     */
    V11::CRingTextItem       transformText(const InitialType& item);

    /*!
     * \brief Transform data format items
     *
     * \param item  a serialized data format item
     * \return a V11 data format item
     *
     * This always outputs the same default V11::CDataFormatItem
     * regardless of what the input is.
     */
    V11::CDataFormatItem     transformDataFormatItem(const InitialType& item);

    /*!
     * \brief Transform physics event items
     *
     * \param item  a serialized 12.0 physics event item
     * 
     * \return a V11 physics event item
     * 
     * The transform logic for this is :
     * - V11 item is outputted with a body header
     * - V12 event timestamp becomes V11 event timestamp
     * - V12 source id becomes V11 source id
     * - V11 barrier type is set to 0
     * - V12 body is copied without change to the V11 body
     */
    V11::CPhysicsEventItem   transformPhysicsEventItem(const InitialType& item);

    /*!
     * \brief Transform event count items
     *
     * \param item  a serialized 12.0 event count item
     *
     * \return an 11.0 event count item
     *
     * The logic of the transform is:
     * - V11 item is outputted with a body header
     * - V12 event timestamp becomes the V11 event timestamp
     * - V12 source id becomes the V11 source id
     * - V11 barrier types is set to 0
     * - V12 time offset becomes the V11 time offset
     * - V12 time offset divisor becomes the V11 time offset divisor
     * - V12 unix timestamp becomes the V11 unix timestamp
     * - V12 event count becomes the V11 event count
     */
    V11::CRingPhysicsEventCountItem
                             transformPhysicsEventCountItem(const InitialType& item);

    /*!
     * \brief Transform scaler items
     *
     * \param item  a serialized 12.0 scaler item
     *
     * \return an 11.0 scaler item
     *
     * The transform logic for this is:
     * - V11 item is outputted with a body header
     * - V12 event timestamp becomes the V11 event timestamp
     * - V12 source id becomes the V11 source id
     * - V11 barrier type is set to 0
     * - V12 interval start offset becomes the V11 interval start offset
     * - V12 interval end offset becomes the V11 interval end offset
     * - V12 unix timestamp becomes the V11 unix timestamp
     * - V12 interval divisor becomes the V11 interval divisor
     * - V12 scaler count becomes the V11 scaler count
     * - V12 incremental state becomes the V11 incremental state
     * - V12 scaler width is discarded.
     * - V12 scalers become the V11 scalers
     */
    V11::CRingScalerItem     transformScalerItem(const InitialType& item);

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
    V11::CGlomParameters     transformGlomParameters(const InitialType& item);

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
    V11::CAbnormalEndItem    transformAbnormalEndItem(const InitialType& item);

};

} // end Transform
} // end DAQ

#endif // CTRANSFORM12P0TO11P0_H
