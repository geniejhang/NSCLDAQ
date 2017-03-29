/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   CTclRingCommand.cpp
# @brief  Implement the ring Tcl command.
# @author <fox@nscl.msu.edu>
*/

#include "CTclRingCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <CRingBuffer.h>
#include <V12/CRawRingItem.h>
#include <V12/CPhysicsEventItem.h>
#include <V12/CRingStateChangeItem.h>
#include <V12/CRingScalerItem.h>
#include <V12/CRingTextItem.h>
#include <V12/CDataFormatItem.h>
#include <V12/CRingPhysicsEventCountItem.h>
#include <V12/CGlomParameters.h>
#include <V12/CRingItemParser.h>
#include <RingIOV12.h>

#include <CDataSourceFactory.h>

#include <CSimpleAllButPredicate.h>
#include <CSimpleDesiredTypesPredicate.h>
#include <CRingItemFactory.h>
#include <CAbnormalEndItem.h>
#include <CTimeout.h>

#include <tcl.h>

#include <limits>
#include <chrono>
#include <thread>
#include <iostream>

using namespace std;

namespace DAQ {
namespace V12 {

/**
 * construction
 *   @param interp - the interpreter on which we are registering the 'ring'
 *                   command.
 */
CTclRingCommand::CTclRingCommand(CTCLInterpreter& interp) :
    CTCLObjectProcessor(interp, "ring", true) {}
    
/**
 * destruction:
 *    Kill off all the CRingItems in the m_attachedRings map.
 */
CTclRingCommand::~CTclRingCommand()
{
    m_attachedRings.clear();
}

/**
 * operator()
 *   Gains control when the command is executed.
 *
 *   @param interp - reference to the interpreter that's executing the command.
 *   @param args   - The command line words.
 *
 *   @return int
 *   @retval TCL_OK - Normal return, the command was successful..
 *   @retval TCL_ERROR - Command completed in error.
 */
int
CTclRingCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    // Put everything in a try/catch block. Errors throw strings that get put
    // in result:
    
    try {
        bindAll(interp, objv);
        requireAtLeast(objv, 2, "Insufficient parameters");
        
        std::string subcommand = objv[1];
        if(subcommand == "attach") {
            attach(interp, objv);
            
        } else if (subcommand == "detach") {
            detach(interp, objv);
        } else if (subcommand == "get") {
            get(interp, objv);
        } else {
            throw std::string("bad subcommand");
        }
        
    } catch(std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    return TCL_OK;
}

/*----------------------------------------------------------------------------
 * Protected members (subcommand handlers)
 */

/**
 * attach
 *    Execute the ring attach command which attaches to a ring.
 *    *  Ensure there's a URI parameter.
 *    *  Connect to the ring creating a CRingBuffer object.
 *    *  Put the ring buffer object in the m_attachedRings map indexed by the
 *    *  ring URI.
 *
 *   @param interp - reference to the interpreter that's executing the command.
 *   @param args   - The command line words.
 *
 *   @throw std::string error message to put in result string if TCL_ERROR
 *          should be returned from operator().
 */
void
CTclRingCommand::attach(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    // need a URI:
    
    requireExactly(objv, 3, "ring attach needs a ring URI");
    std::string uri = objv[2];
    CDataSourcePtr pRing;
    try {
        if (m_attachedRings.find(uri) != m_attachedRings.end()) {
            throw std::string("ring already attached");
        }
        pRing = CDataSourceFactory().makeSource(uri);
        m_attachedRings[uri] = pRing;
    }
    catch(std::string) {
        throw;
    }
    catch (...) {
        throw std::string("Failed to attach ring");
    }
}

/**
 * detach
 *    Execute the ring detach command:
 *    *  Ensure there's a URI parameter
 *    *  Use it to look up the ring in the m_attachedRings map (error if no match).
 *    *  delete the ring buffer object -- which disconnects from the ring.
 *    *  remove the map entry.
 *
 *   @param interp - reference to the interpreter that's executing the command.
 *   @param args   - The command line words.
 *
 *   @throw std::string error message to put in result string if TCL_ERROR
 *          should be returned from operator().
 */
void
CTclRingCommand::detach(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 3, "ring detach needs a URI");
    
    std::string uri = objv[2];
    std::map<std::string, CDataSourcePtr>::iterator p = m_attachedRings.find(uri);
    if (p == m_attachedRings.end()) {
        throw std::string("ring is not attached");
    }
    m_attachedRings.erase(p);
}                             

/**
 * get
 *   Execute the ring get command (blocks until an item is available);
 *    * Ensure there's a ring URI parameter
 *    * Looks up the CRingBuffer in the map (error if no match).
 *    * Gets a CRingItem from the ring with the appropriate filter.
 *    * Produces a dict whose keys/contents will depend on the item type
 *      (which will always be in the -type key).  See the private formattting
 *      functions for more on what's in each dict.
 *   @param interp - reference to the interpreter that's executing the command.
 *   @param args   - The command line words.
 *
 *   @throw std::string error message to put in result string if TCL_ERROR
 *          should be returned from operator().
 */
CTCLObject CTclRingCommand::dispatch(CRingItemPtr pSpecificItem, CTCLInterpreter& interp)
{
    switch(pSpecificItem->type()) {
    case BEGIN_RUN:
    case END_RUN:
    case PAUSE_RUN:
    case RESUME_RUN:
        return formatStateChangeItem(interp, pSpecificItem);

    case PERIODIC_SCALERS:
        return formatScalerItem(interp, pSpecificItem);

    case PACKET_TYPES:
    case MONITORED_VARIABLES:
        return formatStringItem(interp, pSpecificItem);

    case RING_FORMAT:
        return formatFormatItem(interp, pSpecificItem);

    case PHYSICS_EVENT:
        return formatEvent(interp, pSpecificItem);

    case PHYSICS_EVENT_COUNT:
        return formatTriggerCount(interp, pSpecificItem);

    case EVB_GLOM_INFO:
        return formatGlomParams(interp,  pSpecificItem);

    case ABNORMAL_ENDRUN:
        return formatAbnormalEnd(interp, pSpecificItem);

    case COMP_BEGIN_RUN:
    case COMP_END_RUN:
    case COMP_PAUSE_RUN:
    case COMP_RESUME_RUN:
    case COMP_PERIODIC_SCALERS:
    case COMP_PACKET_TYPES:
    case COMP_MONITORED_VARIABLES:
    case COMP_RING_FORMAT:
    case COMP_PHYSICS_EVENT:
    case COMP_PHYSICS_EVENT_COUNT:
    case COMP_EVB_GLOM_INFO:
    case COMP_ABNORMAL_ENDRUN:
        return formatComposite(interp, pSpecificItem);
    default:
        return CTCLObject();
        // TO DO:
    }
}

void
CTclRingCommand::get(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireAtLeast(objv, 3, "ring get needs a URI");
    requireAtMost(objv, 6, "Too many command parameters");

    CSimpleAllButPredicate all;
    CSimpleDesiredTypesPredicate some;
    CDataSourcePredicate* pred;
    pred = &all;
    
    // If there's a 4th parameter it must be a list of item types to select
    // from

    size_t nSeconds = std::numeric_limits<size_t>::max();

    size_t paramIndexOffset = 0;
    if (std::string(objv[2]) == "-timeout") {
        if (objv.size() >= 4) {
            CTCLObject object = objv[3];
            nSeconds = int(object.lindex(0));
            if (nSeconds == 0) {
                throw std::string("A nonzero timeout value must be provided.");
            }
        } else {
            throw std::string("Insufficient number of parameters");
        }
        paramIndexOffset = 2;
    }

    std::string uri  = std::string(objv[2+paramIndexOffset]);
    std::map<std::string, CDataSourcePtr>::iterator p =  m_attachedRings.find(uri);
    if (p == m_attachedRings.end()) {
        throw std::string("ring is not attached");
    }

    if (objv.size() == 4+paramIndexOffset) {
        CTCLObject types = objv[3+paramIndexOffset];
        for (int i = 0; i < types.llength(); i++) {
            int type = int(types.lindex(i));
            some.addDesiredType(type);            
        }
        pred = &some;
    }
    
    // Get the item from the ring.
    
    CTimeout timeout(nSeconds);
    CDataSourcePtr pRing = p->second;
    auto pSpecificItem = getFromRing(*pRing, *pred, timeout);

    if (!pSpecificItem) {
        // oops... we timed out. return an empty string
        CTCLObject result;
        result.Bind(&interp);
        interp.setResult(result);
        return;
    }
    
    // Actual upcast depends on the type...and that describes how to format:
    
    
    CTCLObject result = dispatch(pSpecificItem, interp);
    
    interp.setResult(result);
}

/*-----------------------------------------------------------------------------
 * Private utilities
 */



/**
 *  format a ring state change item.  We're going to use the trick that a dict
 *  has a list rep where even elements are keys and odd elements their values.
 *  Users of the dict will shimmer into its dict rep. at first access.
 *
 *  @param interp - the interpreter.
 *  @param pItem  - Actually a CRingStateChangeItem.
 *
 *  Result is set with a dict with the keys:
 *
 *      type       - ring item type (textual)
 *      run        - run number
 *      timeoffset - Time offset in seconds.
 *      realtime   - Time of day of the ring item (can use with [time format])
 *      title      - The run title.
 *      bodyheader - only if there is a body header in the item.  That in turn is a dict with
 *                   timestamp, source, and barrier keys.
 */
 
CTCLObject CTclRingCommand::formatStateChangeItem(CTCLInterpreter& interp, CRingItemPtr pItem)
{
    CRingStateChangeItemPtr p = std::dynamic_pointer_cast<CRingStateChangeItem>(pItem);
    CTCLObject result;
    result.Bind(interp);
    
    // type:
    
    result += "type";
    result += p->typeName();
    
    result += "run";
    result += static_cast<int>(p->getRunNumber());
    
    result += "timeoffset";
    result += static_cast<int>(p->getElapsedTime());
    
    result += "realtime";
    result += static_cast<int>(p->getTimestamp());
    
    result += "title";
    result += p->getTitle();
    
    formatHeaderInfo(p, result);

    return result;
    
}
/**
 * formatScalerItem
 *    Formats a scaler item.  This creates a list that can be shimmered into a
 *    dict with the keys:
 *    - type - going to be Scaler
 *    - start - When in the run the start was.
 *    - end   - When in the run the end of the period was.
 *    - realtime  Time emitted ([clock format] can take this)
 *    - divisor - What to divide start or end by to get seconds.
 *    - incremental - Bool true if this is incremental.
 *    - scalers     - List of scaler values.
 *
 *    If there is a body header, the bodyheader key will be present and will be
 *    the body header dict.
 *
 *    @param interp - the intepreter object whose result will be set by this.
 *    @param pSpecificItem - Actually a CRingScalerItem pointer.
 */
CTCLObject CTclRingCommand::formatScalerItem(CTCLInterpreter& interp, CRingItemPtr pSpecificItem)
{
    CRingScalerItemPtr pItem = std::dynamic_pointer_cast<CRingScalerItem>(pSpecificItem);
    
    // Stuff outside the body header
   
    CTCLObject result;
    result.Bind(interp);
    
    result += "type";
    result += pItem->typeName();
    
    result += "start";
    result += static_cast<int>(pItem->getStartTime());
    
    result += "end";
    result += static_cast<int>(pItem->getEndTime());
    
    result += "realtime";
    result += static_cast<int>(pItem->getTimestamp());
    
    result += "divisor";
    result += static_cast<int>(pItem->getTimeDivisor());
    
    result += "incremental";
    result += static_cast<int>(pItem->isIncremental() ? 1 : 0);
    
    result += "scalerwidth";
    result += static_cast<int>(pItem->getScalerWidth());

    // Now the scaler vector itself:
    
    std::vector<uint32_t> scalerVec = pItem->getScalers();
    CTCLObject scalerList;
    scalerList.Bind(interp);
    for (int i=0; i < scalerVec.size(); i++) {
        scalerList += static_cast<int>(scalerVec[i]);
    }
    result += "scalers";
    result += scalerList;
    
    formatHeaderInfo(pItem, result);

    return result;
}
/**
 * formatStringItem:
 *    Formats a ring item that contains a list of strings.   This produces a dict
 *    with the following keys:
 *    -   type - result of typeName.
*     -  timeoffset will have the offset time.
*     -  divisor will have the time divisor to get seconds.
*     -  realtime will have something that can be given to [clock format] to get
*        when this was emitted
*     -  strings - list of strings that are in the ring item.
*     -  bodyheader - if the item has a body header.
*     @param interp - the intepreter object whose result will be set by this.
*     @param pSpecificItem - Actually a CRingTextItem pointer.
*     */
CTCLObject CTclRingCommand::formatStringItem(CTCLInterpreter& interp, CRingItemPtr pSpecificItem)
{
    CRingTextItemPtr p = std::dynamic_pointer_cast<CRingTextItem>(pSpecificItem);
    
    CTCLObject result;
    result.Bind(interp);
    
    result += "type";
    result += p->typeName();
    
    result += "timeoffset";
    result += static_cast<int>(p->getTimeOffset());
    
    result += "divisor";
    result += static_cast<int>(p->getTimeDivisor());
    
    result += "realtime";
    result += static_cast<int>(p->getTimestamp());
    
    CTCLObject stringList;
    stringList.Bind(interp);
    std::vector<std::string> strings = p->getStrings();
    for (int i =0; i < strings.size(); i++){
        stringList += strings[i];
    }
    result += "strings";
    result += stringList;
    
    formatHeaderInfo(p, result);

    return result;
}
/**
 * formatFormatitem
 *
 *    Formats a ring format item.  This will have the dict keys:
 *    *  type - what comes back from typeName()
 *    *  major - Major version number
 *    *  minor - minor version number.
 *
 *    @param interp - the intepreter object whose result will be set by this.
 *    @param pSpecificItem - Actually a CDataFormatItem pointer.
*/ 
void CTclRingCommand::formatHeaderInfo(CRingItemPtr p, CTCLObject& result)
{
    result += "timestamp";
    if (p->getEventTimestamp() == V12::NULL_TIMESTAMP) {
        result += "NULL_TIMESTAMP";
    } else {
        Tcl_Obj* tstamp = Tcl_NewWideIntObj(p->getEventTimestamp());
        result += CTCLObject(tstamp);
    }

    result += "source";
    result += static_cast<int>(p->getSourceId());
}

CTCLObject
CTclRingCommand::formatFormatItem(CTCLInterpreter& interp, CRingItemPtr pSpecificItem)
{
    CDataFormatItemPtr p = std::dynamic_pointer_cast<CDataFormatItem>(pSpecificItem);
    
    CTCLObject result;
    result.Bind(interp);
    
    result += "type";
    result += p->typeName();

    result += "major";
    result += static_cast<int>(p->getMajor());
    
    result += "minor";
    result += static_cast<int>(p->getMinor());
    
    formatHeaderInfo(p, result);

    return result;
}
/**
 * formatEvent
 *    Formats a physics event.  This is going ot have:
 *    *  type - "Event"
 *    *  bodyheader if one exists.
 *    *  size - number of bytes in the event
 *    *  body - byte array containing the event data.
 *    @param interp - the intepreter object whose result will be set by this.
 *    @param pSpecificItem - The ring item.
*/ 

CTCLObject CTclRingCommand::formatEvent(CTCLInterpreter& interp, CRingItemPtr pSpecificItem)
{
    CPhysicsEventItemPtr pItem = std::dynamic_pointer_cast<CPhysicsEventItem>(pSpecificItem);

    CTCLObject result;
    result.Bind(interp);
    
    result += "type";
    result += pSpecificItem->typeName();
    
    const auto& body = pItem->getBody();

    result += "size";
    result += static_cast<int>(body.size());

    result += "body";
    Tcl_Obj* tclBody = Tcl_NewByteArrayObj(body.data(), body.size());

    CTCLObject obj(tclBody);
    obj.Bind(interp);
    result += obj;
    
    formatHeaderInfo(pItem, result);

    return result;
}


/**
 * formatTriggerCount
 *
 *   Format dicts for PHYSICS_EVENT_COUNT items.
 *   this dict has:
 *
 *    *   type : "Trigger count"
*     *   bodyheader if the item has a body header present.
*     *   timeoffset - 123 (offset into the run).
*     *   divisor    - 1   divisor needed to turn that to seconds.
*     *   triggers   - 1000 Number of triggers (note 64 bits).
*     *   realtime   - 0 time of day emitted.
*/
    
CTCLObject CTclRingCommand::formatTriggerCount(CTCLInterpreter& interp, CRingItemPtr pSpecificItem)
{
    CRingPhysicsEventCountItemPtr p =
            std::dynamic_pointer_cast<CRingPhysicsEventCountItem>(pSpecificItem);
    
    CTCLObject result;
    result.Bind(interp);
    
    result += "type";
    result += p->typeName();
    
    result += "timeoffset";
    result += static_cast<int>(p->getTimeOffset());
    
    result += "divisor";
    result += static_cast<int>(p->getTimeDivisor());
    
    result += "realtime";
    result += static_cast<int>(p->getTimestamp());
    
    uint64_t ec = p->getEventCount();
    Tcl_Obj* eventCount = Tcl_NewWideIntObj(static_cast<Tcl_WideInt>(ec));
    CTCLObject eventCountObj(eventCount);
    eventCountObj.Bind(interp);
    result += "triggers";
    result += eventCountObj;
    
    formatHeaderInfo(p, result);
    
    return result;
}
/**
 * formatGlomParams
 *     Format a glom parameters item.  This dict will have:
 *
 *     *  type - "Glom Parameters"
 *     *  bodyheader - never has this
 *     *  isBuilding - boolean True if building false otherwise.
 *     *  coincidenceWindow - Number of ticks in the coincidence window.
 *     *  timestampPolicy - one of "first", "last" or "average" indicating
 *        how the timestamp for the items are derived fromt he timestamps
 *        of their constituent fragments.
*/
CTCLObject CTclRingCommand::formatGlomParams(CTCLInterpreter& interp, CRingItemPtr pSpecificItem)
{
    CGlomParametersPtr p = std::dynamic_pointer_cast<CGlomParameters>(pSpecificItem);
    CTCLObject result;
    result.Bind(interp);
    
    result += "type";
    result += p->typeName();
    
    result += "isBuilding";
    result += (p->isBuilding() ? 1 : 0);
    
    result += "coincidenceWindow";
    Tcl_Obj* dTicks = Tcl_NewWideIntObj(
        static_cast<Tcl_WideInt>(p->coincidenceTicks())
    );
    CTCLObject window(dTicks);
    window.Bind(interp);
    result += window;
    
    CGlomParameters::TimestampPolicy policy = p->timestampPolicy();
    result += "timestampPolicy";
    if (policy == CGlomParameters::first) {
        result += "first";
    } else if (policy == CGlomParameters::last) {
        result += "last";
    } else {
        result += "average";
    }
    
    formatHeaderInfo(p, result);
    
    return result;
}
/**
 * formatAbnormalEnd
 *   We only provide  the type (Abnormal End).
 */
CTCLObject CTclRingCommand::formatAbnormalEnd(CTCLInterpreter& interp, CRingItemPtr pSpecificItem)
{

    CTCLObject result;
    result.Bind(interp);
    result += "type";
    result += pSpecificItem->typeName();

    formatHeaderInfo(pSpecificItem, result);

    return result;
}

CTCLObject CTclRingCommand::formatComposite(CTCLInterpreter &interp, CRingItemPtr pSpecificItem)
{
    CCompositeRingItemPtr pItem = std::dynamic_pointer_cast<CCompositeRingItem>(pSpecificItem);

    CTCLObject result;
    result.Bind(interp);
    result += "type";
    result += pItem->typeName();

    formatHeaderInfo(pItem, result);


    CTCLObject children;
    children.Bind(interp);
    for (auto& pChild : pItem->getChildren()) {
        auto childResult = dispatch(pChild, interp);
        children += childResult;
    }

    result += "children";
    result += children;

    return result;
}


CRingItemPtr
CTclRingCommand::getFromRing(CDataSource &ring, CDataSourcePredicate &predicate,
                             const CTimeout& timer)
{

    CRawRingItem item;
    readItemIf(ring, item, predicate, timer);

    if (ring.eof() || timer.expired()) {
        return nullptr;
    } else {
        return CRingItemFactory::createRingItem(item);
    }
}

CRingItemPtr
CTclRingCommand::getFromRing(CDataSource &ring, const CTimeout& timer)
{
    CRawRingItem item;
    readItem(ring, item, timer);

    if (timer.expired()) {
        return nullptr;
    } else {
        return CRingItemFactory::createRingItem(item);
    }
}


} // end V12
} // end DAQ

/*-------------------------------------------------------------------------------
 * Package initialization:
 */

extern "C" 
int Tclringbuffer_Init(Tcl_Interp* pInterp)
{
    Tcl_PkgProvide(pInterp, "TclRingBuffer", "1.0");
    CTCLInterpreter* interp = new CTCLInterpreter(pInterp);
    DAQ::V12::CTclRingCommand* pCommand = new DAQ::V12::CTclRingCommand(*interp);
    
    return TCL_OK;
}


int gpTCLApplication = 0;
