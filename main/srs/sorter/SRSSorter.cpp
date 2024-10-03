// /*
//     This software is Copyright by the Board of Trustees of Michigan
//     State University (c) Copyright 2022.

//     You may use this software under the terms of the GNU public license
//     (GPL).  The terms of this license are described at:

//      http://www.gnu.org/licenses/gpl.txt

//      Authors:
//              Simon Giraud
// 	     FRIB
// 	     Michigan State University
// 	     East Lansing, MI 48824-1321
// */

// Simon - 10/2024 - Purposes: 
// (1) Group srs hits for each fec (source) based on hit timestamp.
// (2) Do sanity checks on the timestamp order and max hits per fec. 


#include "SRSSorter.h"
#include "UDPBrokerDerived.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <memory>
#include <CDataSink.h>
#include <DataFormat.h>
#include <CRingItem.h>


SRSSorter::SRSSorter() : m_maxHits(960), m_dtHits(2) {}

SRSSorter::SRSSorter(uint16_t maxHits = 960, int dtHits = 2) {
    m_maxHits = maxHits;
    m_dtHits = dtHits;
}


SRSSorter::~SRSSorter() {
    // for (int fecId = 0; fecId < MaxFECs; fecId++)
    //     if (m_event[fecId].pRingItem != nullptr) {
    //         delete m_event[fecId].pRingItem;
    //         m_event[fecId].pRingItem = nullptr;
    //     }
}


void SRSSorter::reset() {
    for(int fecId = 0; fecId < MaxFECs; fecId++)
        m_event[fecId].reset();
}

// original no sorting
// void SRSSorter::sort(uint8_t* data, const uint64_t hitTimeStamp, int sid, CDataSink& sink, size_t nBytes){

//     CRingItem* pResult = new CRingItem(PHYSICS_EVENT, hitTimeStamp, sid, 0, nBytes + 1024);
//     // Set cursor to beginning of body
//     pResult->setBodyCursor(pResult->getBodyCursor()); 
//     // Copy the 6 + 2 bytes of data into the CRingItem's body
//     memcpy(pResult->getBodyCursor(), data, HitAndMarkerSizeExtended);
//     // Update cursor after copy
//     pResult->setBodyCursor(reinterpret_cast<uint8_t*>(pResult->getBodyCursor()) + HitAndMarkerSizeExtended);
//     pResult->updateSize();
//     sink.putItem(*pResult); 
//     delete pResult;
    

//     return;
// }

// Hits (data) come here one by one
// Inputs also sid and sink because for the moment the sorter will just group, into a RI,
// hits from the same fec (sink). Then, the event builder will sort the RI from different fec.  
void SRSSorter::sort(uint8_t* data, const uint64_t hitTimeStamp, int sid, CDataSink& sink, size_t nBytes){

    int fecId = sid - 10;
    int64_t tsDiff = hitTimeStamp - m_event[fecId].timestamp;

    if (tsDiff < 0){
        printf("Simon - SRSSorter::sort - tsDiff < 0 - currentTs: %llu prevTs : %llu\n", hitTimeStamp, m_event[fecId].timestamp);
        return;
    }
    if (m_event[fecId].nHits > m_maxHits){
        printf("Simon - SRSSorter::sort - nHits > m_maxHits \n");
        m_event[fecId].nHits = 0;
        deleteRingItem(fecId);
        return;
    }
    ///* // For multi hit 
    if (tsDiff > m_dtHits || hitTimeStamp == 0){
       // printf("Simon - SRSSorter::sort - fecId: %d absDiff: %llu \n", fecId, absDiff(hitTimeStamp, m_event[fecId].timestamp));
        // Release RI into sink when the new timestamp is different enough from the current one
        //sink.putItem(*m_event[fecId].pRingItem); 
        // Delete if necessary and allocate memory for new RingItem
        newRingItem(hitTimeStamp, fecId, nBytes, sink);
        m_event[fecId].timestamp = hitTimeStamp;
        m_event[fecId].nHits = 0;
    }
    else if (tsDiff >= 0 && tsDiff <= m_dtHits){
        //printf("Simon - SRSSorter::sort - in window - currentTs: %llu preTs : %llu\n", hitTimeStamp, m_event[fecId].timestamp);
    }

    appendRingItem(fecId, data);
    //*/

        /* //For single hit
        newRingItem(hitTimeStamp, fecId, nBytes, sink);
        // Set cursor to beginning of body
        //m_event[fecId].pRingItem->setBodyCursor(m_event[fecId].pRingItem->getBodyCursor()); 
        // Copy the 6 + 2 bytes of data into the CRingItem's body
        memcpy(m_event[fecId].pRingItem->getBodyCursor(), data, HitAndMarkerSizeExtended);
        // Update cursor after copy
        m_event[fecId].pRingItem->setBodyCursor(reinterpret_cast<uint8_t*>(m_event[fecId].pRingItem->getBodyCursor()) + HitAndMarkerSizeExtended);
        m_event[fecId].pRingItem->updateSize();
        sink.putItem(*m_event[fecId].pRingItem); 
        delete m_event[fecId].pRingItem;
        m_event[fecId].pRingItem = nullptr;
        */    

    return;
}


void SRSSorter::newRingItem(const uint64_t hitTimeStamp, int fecId, size_t nBytes, CDataSink& sink){

    if (m_event[fecId].pRingItem != nullptr) {
        //printf("Simon - SRSSorter::newRingItem - before putItem - getBodySize: %llu \n", m_event[fecId].pRingItem->getBodySize());
        sink.putItem(*m_event[fecId].pRingItem);// For multi hit (just comment that line and let the rest for single hit)
        deleteRingItem(fecId);
        m_event[fecId].pRingItem = new CRingItem(PHYSICS_EVENT, hitTimeStamp, fecId + 10, 0, nBytes + 1024);
        m_event[fecId].pRingItem->setBodyCursor(m_event[fecId].pRingItem->getBodyCursor());
         

    } else {
        m_event[fecId].pRingItem = new CRingItem(PHYSICS_EVENT, hitTimeStamp, fecId + 10, 0, nBytes + 1024);
        m_event[fecId].pRingItem->setBodyCursor(m_event[fecId].pRingItem->getBodyCursor());

    }

    return;
}



void SRSSorter::appendRingItem(int fecId, uint8_t* data){
    uint16_t oldSize = m_event[fecId].pRingItem->getBodySize();
    // Copy the 8 bytes of data into the CRingItem's body
    memcpy(m_event[fecId].pRingItem->getBodyCursor(), data, HitAndMarkerSizeExtended);
    // Update cursor after copy
    m_event[fecId].pRingItem->setBodyCursor(reinterpret_cast<uint8_t*>(m_event[fecId].pRingItem->getBodyCursor()) + HitAndMarkerSizeExtended);
    m_event[fecId].pRingItem->updateSize();
    
    //printf("Simon - SRSSorter::appendRingItem - body size - old: %lu new : %lu\n", oldSize, m_event[fecId].pRingItem->getBodySize());

    // Following true if data is not empty (is not trig. marker)
    if (oldSize < m_event[fecId].pRingItem->getBodySize()){
        m_event[fecId].nHits = m_event[fecId].nHits + 1;
    }
    else {
        printf("Simon - SRSSorter::appendRingItem - body size unchanged - old: %lu new : %lu\n", oldSize, m_event[fecId].pRingItem->getBodySize());
    }
}


void SRSSorter::deleteRingItem(int fecId){

    if (m_event[fecId].pRingItem != nullptr) {
        delete m_event[fecId].pRingItem;
    }
    m_event[fecId].pRingItem = nullptr;

    return;
}