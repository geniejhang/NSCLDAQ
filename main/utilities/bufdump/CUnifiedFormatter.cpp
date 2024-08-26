/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#include "CUnifiedFormatter.h"
#include <RingItemFactoryBase.h>         // unified fmt
#include <NSCLDAQFormatFactorySelector.h>  // unified fmt.
#include <CRingItem.h>                    // unified fmt
#include <DataFormat.h>                   // unified fmt.
// Ring items from ufmt:

#include <CAbnormalEndItem.h>
#include <CDataFormatItem.h>
#include <CGlomParameters.h>
#include <CPhysicsEventItem.h>
#include <CRingFragmentItem.h>
#include <CRingItem.h>
#include <CRingPhysicsEventCountItem.h>
#include <CRingScalerItem.h>
#include <CRingStateChangeItem.h>
#include <CRingTextItem.h>
#include  <CUnknownFragment.h>
#include <DataFormat.h>

#include <stdlib.h>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <map>
using namespace ufmt;

/// Data and static utilities -- with new major versions of NSCLDAQ
/// This map needs to be extended.

std::map<int, FormatSelector::SupportedVersions> versionMap = {
    {10, FormatSelector::v10},
    {11, FormatSelector::v11},
    {12, FormatSelector::v12}
};

// This must be outside the class, again to avoid CRingItemConflicts:

static CRingItem*
makeActualItem(const CRingItem& raw, ::ufmt::RingItemFactoryBase& fact) {
    // the cases in the switch below will need to be expanded 
    // as more ring item types are defined:

    try {
        switch (raw.type()) {
        case BEGIN_RUN:
        case END_RUN:
        case PAUSE_RUN:
        case RESUME_RUN:
            return fact.makeStateChangeItem(raw);
        case ABNORMAL_ENDRUN:
            return fact.makeAbnormalEndItem(raw);
        case PACKET_TYPES:
        case MONITORED_VARIABLES:
            return fact.makeTextItem(raw);
        case RING_FORMAT:
            return fact.makeDataFormatItem(raw);
        case INCREMENTAL_SCALERS:
        case TIMESTAMPED_NONINCR_SCALERS:
            return fact.makeScalerItem(raw);
        case PHYSICS_EVENT:
            return fact.makePhysicsEventItem(raw);
        case PHYSICS_EVENT_COUNT:
            return fact.makePhysicsEventCountItem(raw);
        case  EVB_FRAGMENT:
            return fact.makeRingFragmentItem(raw);
        case EVB_UNKNOWN_PAYLOAD:
            return fact.makeUnknownFragment(raw);
        case EVB_GLOM_INFO:
            return fact.makeGlomParameters(raw);
        }
        return fact.makeRingItem(raw);
    } catch (std::exception &e){
        std::stringstream err;
        err << "could not convert raw ring item to specific one: \n";
        err << e.what() << std::endl;
        err << "Ring item type was: " << std::hex << "0x" << raw.type() << std::endl;
        err << raw.toString() << std::endl;
        std::string errormsg = err.str();
        throw std::runtime_error(errormsg);
    }
}

//// Implement UnifiedFormatter:

/** constructor
 *    Given the major version of NSCLDAQ - construct the appropriate
 * RingItem factory so that we can create the appropriate actual ring items.
 * and use their toString to format the item.
 * 
 * @param version - major version of NSCLDAQ the ring items belong to.
 * @param pDetail - detail desired should be "headers", "bodies" or "fragments"
 */
CUnifiedFormatter::CUnifiedFormatter(int version, const char* pDetail) : 
    m_pFactory(0) , m_detail(CUnifiedFormatter::bodies)

{
    auto p = versionMap.find(version);
    if (p == versionMap.end()) {
        // No such

        throw std::invalid_argument("Not a valid formt selector in CUnifiedFormatter constructor");
    }
    m_pFactory = &FormatSelector::selectFactory(p->second);

    // Figure out the detail.  Will throw std::invalid_argument if it's not valid but
    // gengetopt should make sure it is before we get to it:

    std::string d(pDetail);    // easier to compare std::string.
    if (d == "headers") {
        m_detail = headers;
    } else if (d == "bodies") {
        m_detail = bodies;
    } else if (d == "fragments") {
        m_detail = fragments;
    } else {
        std::stringstream err;
        err << pDetail << " Is not a valid dump detail value\n";
        err << "Must be one of 'headers', 'bodies', or 'fragments'\n";
        std::string errorMsg = err.str();
        throw std::invalid_argument(errorMsg);
    }

}
/**
 * destructor
 *  
 */
CUnifiedFormatter::~CUnifiedFormatter() {
    delete m_pFactory;
}

/**
 *  operator()
 *     Given a pointer to a raw ring item formats returns
 * a string representation for it.
 * 
 *  @param pItem - pointer to the raw ring item storage.
 *  @return std::string - string representation of the ring item.
 *  
 */
std::string
CUnifiedFormatter::operator()(const void* pItem) {
    const RingItem* pRaw = reinterpret_cast<const RingItem*>(pItem);

    auto rawItem = m_pFactory->makeRingItem(pRaw);
    auto actualItem = makeActualItem(*rawItem, *m_pFactory);

    // How we format it depends on the detail:

    std::string result; 
    switch (m_detail) {
        case headers:
            result = actualItem->headerToString();
            break;
        case bodies:
            result  = actualItem->toString();
            break;
        case fragments:
            if (actualItem->type() == ufmt::PHYSICS_EVENT) {
                result = actualItem->headerToString();
                ufmt::CPhysicsEventItem& 
                    physics(reinterpret_cast<ufmt::CPhysicsEventItem&>(*actualItem));
                result += listFragments(physics);
            } else {
                result = actualItem->toString();
            }
            break;
        default:             // Defensive programming.
            throw std::runtime_error("Invalid value vfor m_detail in CUNifiedFormatter::operator()");
    }

    

    delete rawItem;
    delete actualItem;


    return result;

}

/// private utilities:

/**
 * listFragments
 *     Called to produce a string that consists of the body of a physics
 * event with the fragments dumped.  What we do:
 * 1. Ask the item for its fragments.
 * 2. For each fragment:
 *    - Dump the header of the fragment.
 *    - Convert the fragment payload to the appropriate ring item.
 *    - Dump it.
 * If the conversion threw an exception, dump the generic ring item.
 */
std::string
CUnifiedFormatter::listFragments(ufmt::CPhysicsEventItem& event) {
    std::string result;
    auto fragments = event.getFragments();
    for (auto& f : fragments) {
        result += ">>>>>> Fragment\n";
        std::stringstream fheader;
        fheader << "Timestamp: " << std::hex << f.s_timestamp << std::dec << std::endl;
        fheader << "Source id: " << f.s_sourceId << std::endl;
        fheader << "Barrier  : " << f.s_barrier << std::endl;
        fheader << "Payload size in bytes: " << f.s_size << std::endl;
        fheader << "Payload: \n:";
        result += fheader.str();

        // Make a ring item of the payload and make it the right one:

        ufmt::CRingItem* pRaw = 
            m_pFactory->makeRingItem(reinterpret_cast<const ufmt::RingItem*>(f.s_itemhdr));
        ufmt::CRingItem* pActual(0);
        try {
            pActual = makeActualItem(*pRaw, *m_pFactory);
        }
        catch (...) {
            pActual = m_pFactory->makeRingItem(*pRaw);    // failed actual is just a raw item.
        }
        result += pActual->toString();

        delete pRaw;
        delete pActual;

    }


    return result;
}