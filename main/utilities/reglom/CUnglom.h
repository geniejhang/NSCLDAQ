/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  CRingItemDecoder.h
 *  @brief: Provide the interface for the class that decodes ring items.
 *  
 */

#ifndef CRINGITEMDECODER_H            // Multiple include gaurd.
#define CRINGITEMDECODER_H
#include <CDataSourceFactory.h>
#include <CDataSource.h>
#include <V12/CRingItemFactory.h>
#include <V12/DataFormat.h>                    // Defines ring item types inter alia.
#include <V12/CRingItem.h>                     // Base class for all ring items.
#include <V12/CPhysicsEventItem.h>

#include <RingIOV12.h>
// Ordinary C++ includes.

#include <map>
#include <cstdint>
#include "FragmentIndex.h"

using namespace EVB;
using namespace DAQ;
using namespace DAQ::V12;

/* forward class definitions. */

class CFragmentHandler;
class CEndOfEventHandler;



/**
 * CRingItemDecoder - this class is independent of any data analysis
 *                    framework.  In root, for example, it can be used
 *                    directly given a pointer to a CRingItem object.  In
 *                    SpecTcl, you can construct a CRingItem from the return
 *                    value of the CRingBufferDecoder::getItemPointer method
 *                    and pass it to us as well.
 *
 *                    The decoder, for now just outputs as strings all
 *                    ring items that are not PHYSICS_EVENT items.  Those;
 *                    it assumes are event built data and iterates over the fragments.
 *                    calling registered handlers for each source id found.
 *                    If a source id does not have a registered handler, this will
 *                    just report that to stderr and ignore that fragment.
 **/

class CRingItemDecoder {
private:
    
    // Holds information about a source.
    
    typedef struct _sourceInfo {
        std::uint64_t s_lastTimestamp;
        int           s_nFd;
    } sourceInfo, *psourceInfo;
    
    // Map of sourceInfo structs indexed by source id:
    
    std::map<std::uint32_t, sourceInfo> m_sourceMap;
    
public:
    CRingItemDecoder();
    void operator()(CRingItem* pItem);
    void onEndFile();
    
private:
    void decodePhysicsEvent(CPhysicsEventItem* pItem);
    void decodeOtherItems(CRingItem* pItem);
    void makeNewInfoItem(std::uint32_t sid);
    void checkTimestamp(const FragmentInfo& finfo);
};

#endif