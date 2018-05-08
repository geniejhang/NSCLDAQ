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
    int      sid     = args.sourceid_arg;

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
    glommer.setSourceId(sid);

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
