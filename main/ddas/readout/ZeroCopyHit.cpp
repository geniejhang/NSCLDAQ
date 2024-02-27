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

/** 
 * @file ZeroCopyHit.cpp
 * @brief Implement the ZeroCopyHit class. See header comments.
 */

#include "ZeroCopyHit.h"
#include "ReferenceCountedBuffer.h"
#include "BufferArena.h"

namespace DDASReadout {    
    /**
     * @details
     * Creates a hit. The hit must be initialized with setHit before 
     * being accessed.
     */
    ZeroCopyHit::ZeroCopyHit() :
	m_pBuffer(nullptr), m_pArena(nullptr)
    {}
    
    /**
     * @details
     * Stores stuff away and increments the refrence count on the 
     * underlying buffer.
     */
    ZeroCopyHit::ZeroCopyHit(
	size_t nWords, void* pHitData, ReferenceCountedBuffer* pBuffer,
	BufferArena* pArena    
	) :
	RawChannel(nWords, pHitData), m_pBuffer(pBuffer), m_pArena(pArena)
    {
	reference(); // Count a reference on the buffers.
    }

    /**
     * @details
     * Copy in the new hit and reference.
     */
    ZeroCopyHit::ZeroCopyHit(const ZeroCopyHit& rhs) :
	RawChannel(rhs), m_pBuffer(rhs.m_pBuffer), m_pArena(rhs.m_pArena)
    {
	reference();
    }
    
    /**
     * @details
     * Dereference.
     */
    ZeroCopyHit::~ZeroCopyHit()
    {
	if(m_pBuffer && m_pArena) {
	    dereference();
	}
    }

    /**
     * @details
     * Dereference, copy in, reference.
     */
    ZeroCopyHit&
    ZeroCopyHit::operator=(const ZeroCopyHit& rhs)
    {
	if (this != &rhs) {
	    dereference();
	    RawChannel::operator=(rhs);
	    m_pBuffer = rhs.m_pBuffer;
	    m_pArena  = rhs.m_pArena;
	    reference();
	}
	return *this;
    }  
    
    /**
     * @details
     * If the hit is associated with a zero copy buffer, the reference is 
     * released first.
     */
    void
    ZeroCopyHit::setHit(
	size_t nWords, void* pHitData, ReferenceCountedBuffer* pBuffer,
	BufferArena* pArena
	)
    {
	if (m_pBuffer) dereference();
    
	setData(nWords, pHitData);
	m_pBuffer = pBuffer;
	m_pArena = pArena;
	reference();
    }
    
    /**
     * @details
     * If this hit is associated with data, disassociates.
     */
    void
    ZeroCopyHit::freeHit()
    {
	if (m_pArena && m_pBuffer) {
	    dereference(); // Returns buffer to arena if appropriate.
	    m_pArena = nullptr;
	    m_pBuffer = nullptr;
	    s_data    = nullptr;
	    s_channelLength = 0;        
	}
    }

    void
    ZeroCopyHit::reference()
    {
	m_pBuffer->reference();
    }

    void
    ZeroCopyHit::dereference()
    {
	m_pBuffer->dereference();
	if(!m_pBuffer->isReferenced()) {
	    m_pArena->free(m_pBuffer);
	}
	m_pBuffer = nullptr;
	m_pArena  = nullptr;
    }

}
