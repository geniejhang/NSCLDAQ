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


static unsigned  maxFragments;  // See gitlab issue lucky 13.

static void
dump(std::ostream& o, void* pData, size_t nBytes)
{
    uint16_t* p(reinterpret_cast<uint16_t*>(pData));
    size_t  nwds(nBytes/sizeof(uint16_t));
    
    o << std::hex;
    for (int l = 0; l < nBytes; l += 8) {
        for (int i = 0; i < 8; i++) {
            if (l+i >= nBytes) break;
            o << *p << " ";
            p++;
        }
        o << std::endl;
    }
    
    o << std::dec << std::endl;
}

/**
 * outputGlomParameters
 *
 * Output a GlomParameters ring item that describes how we are operating.
 *
 * @param dt - Build time interval.
 * @param building - True if building.
 */
static void
outputGlomParameters(uint64_t dt, bool building)
{
    pGlomParameters p = formatGlomParameters(dt, building ? 1 : 0,
                                             timestampPolicy);
    io::writeData(STDOUT_FILENO, p, p->s_header.s_size);
}


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
    io::writeData(STDOUT_FILENO, &header, sizeof(header));
    io::writeData(STDOUT_FILENO, &bHeader, sizeof(BodyHeader));
    io::writeData(STDOUT_FILENO, &eventSize,  sizeof(uint32_t));
    io::writeData(STDOUT_FILENO, pAccumulatedEvent, 
		  totalEventSize);
    free(pAccumulatedEvent);
    pAccumulatedEvent = 0;
    totalEventSize    = 0;
    firstEvent        = true;
  }
}
/**
 * outputBarrier
 *
 *  Outputs a barrier event. The ring item type of a barrier
 *  depends:
 *  - If the payload can be determined to be a ring item,
 *    it is output as is.
 *  - If the payload can't be determined to be a ring item,
 *    the entire fragment, header and all is bundled
 *    into a ring item of type EVB_UNKNOWN_PAYLOAD
 *    this is an extension that hopefully helps us deal with
 *    non NSCL DAQ things.
 *
 * @param p - Pointer to the ring item.
 *
 */
static void
outputBarrier(EVB::pFragment p)
{
  pRingItemHeader pH = 
      reinterpret_cast<pRingItemHeader>(p->s_pBody); 
  if(CRingItemFactory::isKnownItemType(p->s_pBody)) {
    
    // This is correct if there is or isn't a body header in the payload
    // ring item.
    

    io::writeData(STDOUT_FILENO, pH, pH->s_size);
    
    if (pH->s_type == BEGIN_RUN) stateChangeNesting++;
    if (pH->s_type == END_RUN)   stateChangeNesting--;
    if (pH->s_type == ABNORMAL_ENDRUN) stateChangeNesting = 0;

  } else {
    std::cerr << "Unknown barrier payload: \n";
    dump(std::cerr, pH, pH->s_size < 100 ? pH->s_size : 100);
    RingItemHeader unknownHdr;
    unknownHdr.s_type = EVB_UNKNOWN_PAYLOAD;
    //
    // Size is the fragment header + ring header + payload.
    // 
    uint32_t size = sizeof(RingItemHeader) +
      sizeof(EVB::FragmentHeader) + p->s_header.s_size;
    unknownHdr.s_size = size;

    io::writeData(STDOUT_FILENO, &unknownHdr, sizeof(RingItemHeader));
    io::writeData(STDOUT_FILENO, p, sizeof(EVB::FragmentHeader));
    io::writeData(STDOUT_FILENO, p->s_pBody, p->s_header.s_size);
  }
}
/**
 * emitAbnormalEnd
 *    Emits an abnormal end run item.
 */
void emitAbnormalEnd()
{
    CAbnormalEndItem end;
    pRingItem pItem= end.getItemPointer();
    EVB::Fragment frag = {{NULL_TIMESTAMP, 0xffffffff, pItem->s_header.s_size, 0}, pItem};
    outputBarrier(&frag);
}

/**
 * acumulateEvent
 * 
 *  This function is the meat of the program.  It
 *  glues fragments together (header and payload)
 *  into a dynamically resized chunk of memory pointed
 *  to by pAccumulatedEvent where  totalEventSize 
 *  is the number of bytes that have been accumulated 
 *  so far.
 *
 *  firstTimestamp is the timestamp of the first fragment
 *  in the acccumulated data.though it is only valid if 
 *  firstEvent is false.
 *
 *  Once the event timestamp exceeds the coincidence
 *  interval from firstTimestamp, the event is flushed
 *  and the process starts all over again.
 *
 * @param dt - Coincidence interval in timestamp ticks.
 * @param pFrag - Pointer to the next event fragment.
 */
void
accumulateEvent(uint64_t dt, EVB::pFragment pFrag)
{
  // See if we need to flush:

  uint64_t timestamp = pFrag->s_header.s_timestamp;
 // std::cerr << "fragment: " << timestamp << std::endl;

  // This bit of kludgery is because we've observed that the
  // s800 emits data 'out of order' from its filter. Specifically,
  // it emits scaler data with larger timestamps than the next
  // physics event.  This causes second level event builders to
  // declare an out of order/data late condition which can result in
  // slightly out of order fragments.
  // We're going to compute timestamp differences in 'both directions'
  // and consider ourselves inside dt if either of them makes the
  // window.
  //
  
  uint64_t tsdiff1    = timestamp-firstTimestamp;
  uint64_t tsdiff2    = firstTimestamp - timestamp;
  uint64_t tsdiff     = (tsdiff1 < tsdiff2) ? tsdiff1 : tsdiff2;
  
  /**
   * Flush the currently accumulated event if any of the following
   * hold:
   *    *   We're not in --build mode so fragemts are events.
   *    *   This fragment's timestamp is outside the coincidence interval
   *    *   There are just too many fragments (timestamp stuck).
   */
  
  if (nobuild || (!firstEvent && ((tsdiff) > dt)) || (fragmentCount > maxFragments)) {
    flushEvent();
  }
  // If firstEvent...our timestamp starts the interval:

  if (firstEvent) {
    firstTimestamp = timestamp;
    firstEvent     = false;
    fragmentCount  = 0;
    timestampSum   = 0;
  }
  lastTimestamp    = timestamp;
  fragmentCount++;
  
  timestampSum    += timestamp;
  
  // Figure out how much we're going to add to the
  // event:

  uint32_t fragmentSize = sizeof(EVB::FragmentHeader) +
    pFrag->s_header.s_size;

  // expand the event (or allocate it) and append
  // this data to it.

  uint8_t* pEvent  = 
    reinterpret_cast<uint8_t*>(realloc(pAccumulatedEvent, 
					totalEventSize + fragmentSize));
  uint8_t* pAppendPointer = pEvent + totalEventSize;
  memcpy(pAppendPointer, &(pFrag->s_header), 
	 sizeof(EVB::FragmentHeader));
  pAppendPointer += sizeof(EVB::FragmentHeader);
  memcpy(pAppendPointer, pFrag->s_pBody, 
	 pFrag->s_header.s_size);

  // finish off the book keeping;

  totalEventSize += fragmentSize;
  pAccumulatedEvent = pEvent;

}

static void outputEventFormat()
{
    // std::cerr << "Outputting event format element\n";
    DataFormat format;
    format.s_header.s_size = sizeof(DataFormat);
    format.s_header.s_type = RING_FORMAT;
    format.s_mbz         = 0;
    format.s_majorVersion = FORMAT_MAJOR;
    format.s_minorVersion = FORMAT_MINOR;
    
    io::writeData(STDOUT_FILENO, & format, sizeof(format));
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

  gengetopt_args_info args;
  cmdline_parser(argc, argv, &args);
  int dtInt = static_cast<uint64_t>(args.dt_arg);
  nobuild      = args.nobuild_given;
  timestampPolicy = args.timestamp_policy_arg;
  sourceId       = args.sourceid_arg;
  maxFragments   = args.maxfragments_arg;


    // set up a sink for STDOUT
    DAQ::CDataSinkPtr pSink(DAQ::CDataSinkFactory().makeSink("-"));


    uint64_t dt      = static_cast<uint64_t>(dtInt);
    bool     nobuild = args.nobuild_flag;
    int      sid     = args.sourceid_arg;

  std::cerr << (nobuild ? " glom: not building " : "glom: building events: ");
  if (!nobuild) {
    std::cerr << dtInt << std::endl;
  } else {
    std::cerr << std::endl;
  }

    std::cerr << (nobuild ? " glom: not building " : "glom: building") << std::endl;

    if (!nobuild && (dtInt < 0)) {
        std::cerr << "Coincidence window must be >= 0 was "
                  << dtInt << std::endl;
        exit(-1);
    }

  bool firstBarrier(true);
  bool consecutiveBarrier(false);
  try {
    while (1) {
      EVB::pFragment p = CFragIO::readFragment(STDIN_FILENO);
      
      // If error or EOF flush the event and break from
      // the loop:
      
      if (!p) {
        flushEvent();
        std::cerr << "glom: EOF on input\n";
            if(stateChangeNesting) {
                emitAbnormalEnd();
            }
        break;
      }
      // We have a fragment:
      
      if (p->s_header.s_barrier) {
        flushEvent();
        outputBarrier(p);
        
        
        // Barrier type of 1 is a begin run.
        // First begin run barrier will result in
        // emitting a glom parameter record.

        
        if(firstBarrier && (p->s_header.s_barrier == 1)) {
            outputGlomParameters(dtInt, !nobuild);
            firstBarrier = false;
        }
      } else {

        // Once we have a non barrier, reset firstBarrier so that we'll
        // emit a glom parameters next time we have a barrier.
        // This is needed if the event builder is run in persistent mode.
        // see gitlab issue #11 for nscldaq.
        
        firstBarrier = true;
        
        // If we can determine this is a valid ring item other than
        // an event fragment it goes out out of band but without flushing
        // the event.
    
        pRingItemHeader pH = reinterpret_cast<pRingItemHeader>(p->s_pBody);
        if (CRingItemFactory::isKnownItemType(p->s_pBody)) {
            
            if (pH->s_type == PHYSICS_EVENT) {
              accumulateEvent(dt, p); // Ring item physics event.
            } else {
              outputBarrier(p);	// Ring item non-physics event.
            }
        } else {		// non ring item..treat like event.
          std::cerr << "GLOM: Unknown ring item type encountered: \n";
          dump(std::cerr, pH, pH->s_size < 100 ? pH->s_size : 100); 
          
          outputBarrier(p);
        }
      }
      freeFragment(p);
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
