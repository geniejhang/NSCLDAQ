/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  CBuiltRingItemExtender
 *  @brief: Implement the CBuiltRingItemExtender class see the header for more.
 */
#include "CBuiltRingItemExtender.h"
#include "CSender.h"
#include "CRingItemSorter.h" // message format.

#include <fragment.h>
#include <CRingItem.h>
#include <CRingItemFactory.h>
#include <DataFormat.h>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <string.h>

/**
 * some useful types:
 */

// The first iovect in each ring item points to this.

typedef struct __attribute__(__packed__)) _EventHeader {
    RingItemHeader       s_ringHeader;
    BodyHeader           s_bodyHeader;
    uint32_t             s_evbBodySize;
} EventHeader, *pEventHeader;

// This struct actually points to the entire fragment but we define this
// because we'll need to adjust the payload size in the fragment header
// and the ringitem header size if an extension is added on.

typedef struct __attribute__(__packed__)) _FragmentItem {
    EVB::FragmentHeader   s_fragHeader;
    RingItemHeader        s_ringItemHeader;
} FragmentItem, *pFragmentItem;

/**
 * constructor
 *   @param fanin - the data source.  We are taking data from a fanout transport.
 *   @param sink  - Where our processed ring items go.  Normally this is a fanin
 *                  to a ring item sorter to put things back in time order.
 *   @param clientId - My fanout client id.  We provide that to the sorter
 *                  so it can manage its queues properly.
 *   @param pExtender - The user code that can provide our event fragments with
 *                   extension data.
 */
CBuiltRingItemExtender::CBuiltRingItemExtender(
    CFanoutClientTransport& fanin, CSender& sink, uint64_t clientId,
    CRingItemExtender* pExtender
) :
    CParallelWorker(fanin, sink, clientId), m_pExtender(pExtender), m_nId(clientId),
    m_pIoVectors(nullptr), m_nIoVectorCount(0), m_nUsedIoVectors(0)
{}

/**
 * destructor
 *    The I/O vectors are dynamic and need to be freed (free(3) not delete).
 */
CBuiltRingItemExtender::~CBuiltRingItemExtender()
{
    free(m_pIoVectors);
}
/**
 * process
 *   This gets data from the fanout.  Each datum is just a block of ring items.
 *   When we send our reply block we need to know how many I/O vectors we might
 *   be in for.
 *   *  One that points to our client id for the sorter.
 *   *  For each ring item:
 *      - One for the ring item header up through the body size from the
 *        event builder.
 *      - One for each fragment.
 *      - One for the potential extension data for each fragment.s
 *
 * @param pData  - Data block.
 * @param nBytes - Number of bytes of data.
 * @note the base class gets the data from the fanout and frees dynamic
 *       storage associated with pDataq for us.
 * @note nBytes == 0 is legal and means end of data.
 */
void
CBuiltRingItemExtender::process(void* pData, size_t nBytes)
{

    if (nBytes) {
        size_t maxVecs = iovecsNeeded(pData, nBytes);
        allocateIoVectors(maxVecs);
    
        // We send our source id at the front.
        
        m_pIoVectors[0].iov_len = sizeof(uint32_t);
        m_pIoVectors[0].iov_base = &m_nId;
        m_nUsedIoVectors = 1;
        
        // This vector of I/O vectors is used to keep track of the extensions
        // that have to be freed:
        
        std::vector<iovec> extensions;
        
        // Now we need to loop over the ring items:
        
        size_t nRingItems = countItems(pData, nBytes);
        void* pItem = pData;
        for (int i =0; i < nRingItems; i++) {
            
            // For each ring item we have an I/Vec for the
            // data in the ring item header, its body header and the
            // event builder body size.  We'll also hold a pointer to that
            // data so that we can adjust sizes as extensions are added:
            
            m_pIoVectors[m_nUsedIoVectors].iov_base = pData;
            m_pIoVectors[m_nUsedIoVectors].iov_len  = sizeof(EventHeader);
            m_nUsedIoVectors++;
            
            pEventHeader pItemHeader = static_cast<pEventHeader>(pData);
            
            // Now we need to  loop over the fragments in each event giving
            // the extender a chance to add an extension to the fragment:
            
            void* pEvent = reinterpret_cast<void*>(&(pItemHeader->s_evbBodySize));
            size_t nFragments = countFragments(pEvent);
            void*  pFrag      = firstFragment(pEvent);
            
            for (int i =0; i < nFragments; i++) {
                // The fragment points to something like:
                
                pFragmentItem pFragFront = static_cast<pFragmentItem>(pFrag);
                
                // The iovec for the fragment as a whole:
                
                m_pIoVectors[m_nUsedIoVectors].iov_base =  pFrag;
                m_pIoVectors[m_nUsedIoVectors].iov_len  =
                    sizeof(EVB::FragmentHeader) +
                    pFragFront->s_ringItemHeader.s_size;
                m_nUsedIoVectors++;
                
                pRingItem pFragmentRingItem =
                    reinterpret_cast<pRingItem>(&(pFragFront->s_ringItemHeader));
                iovec extension = (*m_pExtender)(pFragmentRingItem);
                
                // There's an extension if the size is nonzero:
                
                if(extension.iov_len) {
                    m_pIoVectors[m_nUsedIoVectors] = extension;
                    m_nUsedIoVectors++;
                    extensions.push_back(extension);
                    
                    // Adjust the sizes:
                    
                    pItemHeader->s_evbBodySize += extension.iov_len;     // EVB size,
                    pFragFront->s_ringItemHeader.s_size += extension.iov_len; // Current fragment ringitem.
                    pItemHeader->s_ringHeader.s_size += extension.iov_len;  // full ring .
                }
                
                pFrag = nextFragment(pFrag);
            }
            
            pItem = nextItem(pItem);
        }
        // At this point, the IOVec has been built and m_nUsedIoVectors is the
        // number of vectors.  Send the message then free the extension data:
        
        getSink()->sendMessage(m_pIoVectors, m_nUsedIoVectors);
        for (int i =0; i < extensions.size(); i++) {
            m_pExtender->free(extensions[i]);
        }      
    } else {
        getSink()->sendMessage(&m_nId, sizeof(m_nId));  // end of data for our id.
    }
}
///////////////////////////////////////////////////////////////////////////////
// Private utilities:

/**
 * countItems
 *    Count the number of ring items in a block.
 * @param pData - pointer to the data block that is stuffed with ring items.
 * @param nBytes - Number of bytes of data.
 * @return size_t - number of ring items in the block.
 */
size_t
CBuiltRingItemExtender::countItems(const void* pData, size_t nBytes)
{   size_t result = 0;
    while (nBytes) {
        result++;
        const EventHeader* p = static_cast<const EventHeader*>(pData);
        nBytes -= p->s_ringHeader.s_size;
        
        pData = nextItem(pData);
    }
    
    return result;
}
/**
 * nextItem
 *    Given a pointer to a ring item returns a pointer to data just after it.
 *
 * @param pData - pointer to the ring item.
 * @return void* - pointer to the byt following the ring item.
 */
void*
CBuiltRingItemExtender::nextItem(const void* pData)
{
    const EventHeader* pItem = static_cast<const EventHeader*>(pData);
    uint8_t*  p = reinterpret_cast<uint8_t*>(const_cast<pEventHeader>(pItem));
    p += pItem->s_ringHeader.s_size;
    
    return p;
}
/**
 * firstFragment
 *    Returns a pointer to the first fragment of an event given a pointer
 *    to the event body.
 *
 *  @param pEvent - pointer to the entire event.
 *  @return void* - Pointer to the first event fragment.
 */
void*
CBuiltRingItemExtender::firstFragment(const void* pEvent)
{
    const uint32_t* p = static_cast<const uint32_t*>(pEvent);
    p++;
    
    return reinterpret_cast<void*>(const_cast<uint32_t*>(p));
}
/**
 * countFragments
 *    Given a pointer to an event built body, returns the number
 *    of fragment in the event.  The pointer is pointing to the
 *    uint32_t at the front of the event body.
 *
 *  @param pEvent - pointer to the front of the event body.
 */
size_t
CBuiltRingItemExtender::countFragments(const void* pEvent)
{
    const uint32_t* p = static_cast<const uint32_t*>(pEvent);
    size_t nBytes = *p;
    nBytes--;                        // Self Counting.
    
    void* pFrag = firstFragment(pEvent);
    size_t result(0);
    
    while(nBytes) {
        result++;
        pFragmentItem pItem = static_cast<pFragmentItem>(pFrag);
        result += pItem->s_ringItemHeader.s_size + sizeof(EVB::FragmentHeader);
        
        pFrag = nextFragment(pFrag);
    }
    
    return result;
}
/**
 * nextFragment
 *   Given a pointer to a fragment returns a pointer to the byte after
 *   the fragment ends.
 *
 * @param pData - pointer to the fragment.
 * @return void* - pointer to the byte after the fragment.
 */
void*
CBuiltRingItemExtender::nextFragment(const void* pData)
{
    const FragmentItem* pFrag = static_cast<const FragmentItem*>(pData);
    size_t fragmentSize = sizeof(EVB::FragmentHeader) + pFrag->s_ringItemHeader.s_size;
    
    uint8_t* pResult = reinterpret_cast<uint8_t*>(const_cast<pFragmentItem>(pFrag));
    pResult += fragmentSize;
    
    
    return pResult;
}
/**
 * iovecsNeeded
 *    @param pData - pointer to the raw data block.
 *    @return size_t - maximum number of I/O vectors we need to represent the output.
 *                     This is computed by assuming all fragments need an extension.
 */
size_t
CBuiltRingItemExtender::iovecsNeeded(const void* pData, size_t nBytes)
{
    size_t result = 1;                 // For our id.
    size_t nRingItems = countItems(pData, nBytes);
    result += nRingItems;              // Each Event needs one item.
    
    for (int i =0; i < nRingItems; i++) {
        const EventHeader* pEventHdr = static_cast<const EventHeader*>(pData);
        const void* pBody             = &(pEventHdr->s_evbBodySize);
        size_t frags = countFragments(pBody);
        
        // Each fragment needs at most two iovecs:
        
        result += frags*2;
    }
    
    return result;
}
/**
 * allocateIoVectors
 *    If needed, replaces the m_pIoVectors storage with enough
 *    to fit the requested numnber of iovecs..
 *  @param needed - number of IO vectors needed.
 */
void
CBuiltRingItemExtender::allocateIoVectors(size_t needed)
{
    // Only adjust if needed is > than what we have:
    
    
    if (needed > m_nIoVectorCount) {
        free(m_pIoVectors);             // Harmless if nullptr.
        m_pIoVectors = static_cast<iovec*>(malloc(needed * sizeof(iovec)));
        if (!m_pIoVectors) {
            // probably fatal:
            
            throw std::string(
                "Could not allocate sufficient i/o vector for CBuitlRingItemExtender"
            );
        }
        m_nIoVectorCount = needed;
    }
    // If needed <= what we have, leave everything alone.
}