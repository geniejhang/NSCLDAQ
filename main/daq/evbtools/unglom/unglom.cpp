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

/**
 * @file unglom.cpp
 * @brief Filter that separates an event file output by glom into its 
 *        constituent fragments. 
 * @author: Ron Fox
 */

#include <CUnglom.h>

#include <V12/DataFormat.h>
#include <V12/CRawRingItem.h>
#include <V12/CCompositeRingItem.h>
#include <V12/Serialize.h>
#include <ByteBuffer.h>


#include <CDataSourceFactory.h>
#include <CDataSinkFactory.h>
#include <CDataSource.h>
#include <CDataSink.h>
#include <RingIOV12.h>

#include "cmdline.h"

#include <fragment.h>

#include <Exception.h>
#include <exception>
#include <iostream>
#include <cstdint>
#include <cstring>
#include <cerrno>

#include <unistd.h>

using namespace DAQ;

uint32_t barrierType(V12::CRingItem &item);
void writeFragment(CDataSink& sink, V12::CRingItem &item);


/**
 * glom
 *
 *  This file contains a 'simple' filter which takes event data output by glom
 *  and sepates it back into an input stream suitable for use with glom again.
 *  A typical use would be in a pipeline with glom again to rebuild events with a 
 *  different coincidence interval  For example:
 *
 * \verbatim
 *    unglom <old-event-file | glom -dt 1234 >new-event-file
 * \endverbatim
 *
 */
int main(int argc, char**argv)
{
    struct gengetopt_args_info args;
    int status = cmdline_parser (argc, argv, &args);
    if (status < 0) {
        std::cerr << "Unrecognized option!" << std::endl;
        cmdline_parser_print_help();
        exit(EXIT_FAILURE);
    }

  CDataSourcePtr pSource(CDataSourceFactory().makeSource("-"));
  CDataSinkPtr   pSink(CDataSinkFactory().makeSink("-"));

  CUnglom glommer(pSource, pSink);
  try {
      glommer();
  }
  catch (std::string msg) {
    std::cerr << "unglom: Exception caught: " << msg << std::endl;
  }
  catch (const char* msg) {
    std::cerr << "unglom: Exception caught: " << msg << std::endl;
  }
  catch (CException& e) {
    std::cerr  << "unglom: Exception caught: " << e.ReasonText() << std::endl;
  }
  catch (int err) {
    std::cerr << "unglom: Exception caught: " << strerror(err) << std::endl;

  }
  catch (std::exception e) {
    std::cerr << "unglom: Exception caught: " << e.what() << std:: endl;
  }
  catch(...) {
    std::cerr << "unglom: Unrecognized exception caught\n";
  }
  
}


uint32_t barrierType(V12::CRingItem& item)
{
    uint32_t type = item.type();
    if (type == V12::BEGIN_RUN
            || type == V12::END_RUN
            || type == V12::PAUSE_RUN
            || type == V12::RESUME_RUN) {
        return type;
    } else {
        return 0;
    }
}

void writeFragment(CDataSink& sink, V12::CRingItem& item)
{
    Buffer::ByteBuffer fragment;
    fragment << item.getEventTimestamp();
    fragment << item.getSourceId();
    fragment << item.size();
    fragment << barrierType(item);
    fragment << V12::serializeItem(item);

    sink.put(fragment.data(), fragment.size());
}
