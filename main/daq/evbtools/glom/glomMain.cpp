/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
         NSCL
         Michigan State University
         East Lansing, MI 48824-1321
*/

#include "glom.h"
#include <CGlom.h>
#include "fragment.h"
#include "fragio.h"

#include <V12/DataFormat.h>
#include <V12/CRingItem.h>
#include <V12/CRingItemFactory.h>
#include <V12/CAbnormalEndItem.h>
#include <V12/CDataFormatItem.h>
#include <V12/CAbnormalEndItem.h>
#include <V12/CGlomParameters.h>
#include <V12/CCompositeRingItem.h>
#include <RingIOV12.h>

#include <CDataSink.h>
#include <CDataSinkFactory.h>
#include <io.h>

#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <exception>
#include <memory>

using namespace DAQ::V12;

//// File scoped  variables:

//static uint64_t firstTimestamp;
//static uint64_t lastTimestamp;
//static uint64_t timestampSum;
//static uint32_t sourceId;
//static uint64_t fragmentCount(0);
//static uint32_t currentType;

//static bool            firstEvent(true);
//static bool            nobuild(false);
//static CGlomParameters::TimestampPolicy timestampPolicy;
//static unsigned        stateChangeNesting(0);

//std::vector<CRingItemPtr> accumulatedItems;
//DAQ::CDataSinkPtr pSink;

///**
// * outputGlomParameters
// *
// * Output a GlomParameters ring item that describes how we are operating.
// *
// * @param dt - Build time interval.
// * @param building - True if building.
// */
//static void
//outputGlomParameters(uint64_t dt, bool building)
//{
//    CGlomParameters item(dt, building, timestampPolicy);
//    writeItem(*pSink, item);
//}

///**
// * flushEvent
// *
// * Flush the physics event that has been accumulated
// * so far.
// *
// * If nothing has been accumulated, this is a noop.
// *
// */
//static void
//flushEvent()
//{
//    if (accumulatedItems.size() > 0) {

//        // Figure out which timestamp to use in the generated event:

//        uint64_t eventTimestamp;
//        switch (timestampPolicy) {
//        case CGlomParameters::first :
//            eventTimestamp = firstTimestamp;
//            break;
//        case CGlomParameters::last :
//            eventTimestamp = lastTimestamp;
//            break;
//        case CGlomParameters::average :
//            eventTimestamp = (timestampSum/fragmentCount);
//            break;
//        default:
//            // Default to earliest...but should not occur:
//            eventTimestamp = firstTimestamp;
//            break;
//        }

//        CCompositeRingItem builtItem;
//        builtItem.setType(COMPOSITE_BIT | accumulatedItems.front()->type());
//        builtItem.setEventTimestamp(eventTimestamp);
//        builtItem.setSourceId(sourceId);

//        builtItem.setChildren(accumulatedItems);

//        writeItem(*pSink, builtItem);

//        if (builtItem.type() == COMP_BEGIN_RUN)       stateChangeNesting++;
//        if (builtItem.type() == COMP_END_RUN)         stateChangeNesting--;
//        if (builtItem.type() == COMP_ABNORMAL_ENDRUN) stateChangeNesting = 0;

//        firstEvent        = true;
//        fragmentCount     = 0;
//        currentType       = UNDEFINED;
//        accumulatedItems.clear();
//    }
//}

///**
// * emitAbnormalEnd
// *    Emits an abnormal end run item.
// */
//void emitAbnormalEnd()
//{
//    CAbnormalEndItem item;
//    writeItem(*pSink, item);
//}

///**
// * acumulateEvent
// *
// *  This function is the meat of the program.  It
// *  glues fragments together (header and payload)
// *  into a dynamically resized chunk of memory pointed
// *  to by pAccumulatedEvent where  totalEventSize
// *  is the number of bytes that have been accumulated
// *  so far.
// *
// *  firstTimestamp is the timestamp of the first fragment
// *  in the acccumulated data.though it is only valid if
// *  firstEvent is false.
// *
// *  Once the event timestamp exceeds the coincidence
// *  interval from firstTimestamp, the event is flushed
// *  and the process starts all over again.
// *
// * @param dt - Coincidence interval in timestamp ticks.
// * @param pFrag - Pointer to the next event fragment.
// */
//void
//accumulateEvent(uint64_t dt, CRingItemPtr pItem)
//{
//    // See if we need to flush:

//    uint64_t timestamp = pItem->getEventTimestamp();

//    // If firstEvent...our timestamp starts the interval:

//    if (firstEvent) {
//        firstTimestamp = timestamp;
//        firstEvent     = false;
//        fragmentCount  = 0;
//        timestampSum   = 0;
//        currentType    = pItem->type();
//    }
//    lastTimestamp    = timestamp;
//    fragmentCount++;
//    timestampSum    += timestamp;

//    // Figure out how much we're going to add to the
//    // event:

//    accumulatedItems.push_back(pItem);

//}

//static void outputEventFormat()
//{
//    CDataFormatItem format;
//    writeItem(*pSink, format);
//}

CGlomParameters::TimestampPolicy
mapTimestampPolicy(enum enum_timestamp_policy& policy)
{
    switch(policy) {
    case timestamp_policy_arg_earliest :
        return CGlomParameters::first;

    case timestamp_policy_arg_latest:
        return CGlomParameters::last;

    case timestamp_policy_arg_average:
        return CGlomParameters::average;

    default:
        return CGlomParameters::first;
    }
}

/**
 * Main for the glommer
 * - Parse the arguments and extract the dt.
 * - Until EOF on input, or error, get fragments from stdin.
 * - If fragments are not barriers, accumulate events
 * - If fragments are barriers, flush any accumulated
 *   events and output the barrier body as a ring item.
 *
 * @param argc - Number of command line parameters.
 * @param argv - array of pointers to the parameters.
 */
int
main(int argc, char**  argv)
{
    // Parse the parameters;

    gengetopt_args_info args;
    cmdline_parser(argc, argv, &args);
    uint64_t dtInt   = static_cast<uint64_t>(args.dt_arg);

    CGlomParameters::TimestampPolicy
            timestampPolicy = mapTimestampPolicy(args.timestamp_policy_arg);

    // set up a sink for STDOUT
    DAQ::CDataSinkPtr pSink(DAQ::CDataSinkFactory().makeSink("-"));

    uint64_t dt      = static_cast<uint64_t>(dtInt);
    bool     nobuild = args.nobuild_flag;

    std::cerr << (nobuild ? " glom: not building " : "glom: building") << std::endl;

    if (!nobuild && (dtInt < 0)) {
        std::cerr << "Coincidence window must be >= 0 was "
                  << dtInt << std::endl;
        exit(-1);
    }


    DAQ::CGlom glommer(pSink);
    glommer.disableBuilding(nobuild);
    glommer.setCorrelationTime(dt);
    glommer.setTimestampPolicy(timestampPolicy);

    glommer.outputEventFormat();

    try {
        glommer();
    }
    catch (std::string msg) {
        std::cerr << "glom: " << msg << std::endl;
    }
    catch (const char* msg) {
        std::cerr << "glom: " << msg << std::endl;
    }
    catch (int e) {
        std::string msg = "glom: Integer error: ";
        msg += strerror(e);
        std::cerr << msg << std::endl;
    }
    catch (std::exception& except) {
        std::string msg = "glom: ";
        msg += except.what();
        std::cerr << msg << std::endl;
    }
    catch(...) {
        std::cerr << "Unanticipated exception caught\n";

    }

}
