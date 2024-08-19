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

#include <stdlib.h>
#include <iostream>
#include <stdexcept>
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
        std::cerr << "could not convert raw ring item to specific one: \n";
        std::cerr << e.what() << std::endl;
        std::cerr << "Ring item type was: " << std::hex << "0x" << raw.type() << std::endl;
        exit(EXIT_FAILURE);
    }
}

//// Implement UnifiedFormatter:

/** constructor
 *    Given the major version of NSCLDAQ - construct the appropriate
 * RingItem factory so that we can create the appropriate actual ring items.
 * and use their toString to format the item.
 * 
 * @param version - major version of NSCLDAQ the ring items belong to.
 */
CUnifiedFormatter::CUnifiedFormatter(int version) : m_pFactory(0)
{
    auto p = versionMap.find(version);
    if (p == versionMap.end()) {
        // No such

        throw std::invalid_argument("Not a valid formt selector in CUnifiedFormatter constructor");
    }
    m_pFactory = &FormatSelector::selectFactory(p->second);
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

    std::string result = actualItem->toString();

    delete rawItem;
    delete actualItem;


    return result;

}
