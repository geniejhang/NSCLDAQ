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
 * @file ModuleReader.cpp
 * @brief Implement the ModuleReader.
 */

#include "ModuleReader.h"

#include <string.h>

#include <sstream>
#include <stdexcept>
#include <iostream>

#include <config.h>
#include <config_pixie16api.h>
#include <CDDASException.h>
#include "ReferenceCountedBuffer.h"
#include "ZeroCopyHit.h"

namespace DDASReadout {
    
    /**
     * @details
     * Just save all the stuff for now.
     */
    ModuleReader::ModuleReader(
	unsigned module, unsigned evtlen, uint32_t moduleType,
	double timeMultiplier
	) :
	m_nModuleNumber(module), m_nExpectedEventLength(evtlen),
	m_tsMultiplier(timeMultiplier), m_moduleTypeWord(moduleType)
    {
	reset();
    }

    /**
     * @details
     * Kill off dynamic data we have. Note that if this is called prior to 
     * having all buffers returned holy hell will break loose eventually as 
     * there's no place to return the reference counted buffers.
     */
    ModuleReader::~ModuleReader()
    {   
	// The buffer pool cleans itself up. We need to clean up our hit pool:
	while (!m_freeHits.empty()) {
	    ZeroCopyHit* pHit = m_freeHits.front();
	    delete pHit;
	    m_freeHits.pop_front();
	}
    }

    /**
     * @details
     * If the hit list is not cleared, the data will be appended to any
     * existing data.
     *
     * @note Zero-copy strategy is used to ensure that once the data are 
     * read, they are not copied.
     */
    size_t
    ModuleReader::read(HitList& hits, size_t nWords)
    {
	int readstat;
	
	// Make nWords a multiple of m_nExpectedEventLength:
	unsigned remainder = nWords % m_nExpectedEventLength;	
	nWords = nWords - remainder;
	
	if (nWords > 0) {
	    ReferenceCountedBuffer* pBuffer =
		m_freeBuffers.allocate(nWords*sizeof(uint32_t));

	    try {
		int rv = Pixie16ReadDataFromExternalFIFO(
		    static_cast<unsigned int*>(pBuffer->s_pData),
		    (unsigned long)(nWords),
		    (unsigned short)(m_nModuleNumber)
		    );
		if (rv < 0) {
		    std::stringstream msg;
		    msg << "Error reading module " << m_nModuleNumber
			<< " FIFO. Tried to read " << nWords
			<< " uint32_t words of data."
			<< " Acting as if there are no words to read.";
		    throw CDDASException(
			rv, "Pixie16ReadDataFromExternalFIFO", msg.str()
			);
		}
	    } catch (const CDDASException& e) {
		std::cerr << e.ReasonText() << std::endl;

		return 0;
	    }
	    
	    parseHits(hits, *pBuffer, nWords); // Zero-copy process hits.
	}
    
	return nWords;
    }

    void
    ModuleReader::freeHit(HitInfo& hit)
    {
	hit.second->freeHit(); // Prepare for re-use.
	hit.first->m_freeHits.push_back(hit.second);
    }

    void
    ModuleReader::reset()
    {
	std::cerr << "Resetting last channel timestamps on module: "
		  << m_nModuleNumber << std::endl;
	// Start at timestamp == 0:
	memset(m_lastStamps, 0, 16*sizeof(double));      
    }

////////////////////////////////////////////////////////////////////////////////
// Private utility methods.
//

    /**
     * @note The members of each hit are fully filled in with the data from
     * the raw hit information.
     */
    void
    ModuleReader::parseHits(
	HitList& hits, ReferenceCountedBuffer& pBuffer, size_t nUsedWords
	)
    {
	uint32_t* pData = (uint32_t*)(pBuffer);
	while(nUsedWords > 0) {
	    uint32_t size = RawChannel::channelLength(pData);
	    ZeroCopyHit* pHit = allocateHit();
	    pHit->setHit(size, pData, &pBuffer, &m_freeBuffers);
	    HitInfo hit = std::make_pair(this, pHit);
	    if(pHit->Validate(m_nExpectedEventLength)) {
		std::stringstream s;
		s << "Inconsistent event lengths in module "
		  << m_nModuleNumber << ": Expected "
		  << m_nExpectedEventLength << ", got "
		  << pHit->s_channelLength;
		throw std::length_error(s.str());
	    }
	    if(pHit->SetTime(m_tsMultiplier)) {
		std::cerr << "Warning Hit from module" << m_nModuleNumber
			  << " does not contain a full header:"
			  << " tossing the hit\n";
            
		freeHit(hit);
		continue;
	    }
	    if(pHit->SetChannel()) {
		std::cerr << "Warning Hit from module" << m_nModuleNumber
			  << " does not contain a full header: "
			  << " tossing the hit\n";
                
		freeHit(hit);
		continue;
	    }
	    checkOrder(pHit);
	    hits.push_back(hit);
        
	    pData += size;
	    nUsedWords -= size;
        
	}
    }

    ZeroCopyHit*
    ModuleReader::allocateHit()
    {
	if(m_freeHits.empty()) {
	    m_freeHits.push_back(new ZeroCopyHit);
	}
	ZeroCopyHit* pResult = m_freeHits.front();
	m_freeHits.pop_front();
    
	return pResult;
    }
    
    /**
     * @details
     * Outputs a warning or error message to std error if the timestamp is bad:
     * - The timestamp is bad if it's less than the last one from the channel 
     *   since within a channel times monotonically increase.
     * - The timestamp is bad (different message) if it's the same as the last 
     *   timestamp from that channel.
     */
    void ModuleReader::checkOrder(ZeroCopyHit* pHit)
    {
	double newTime = pHit->s_time;
	int    ch      = pHit->s_chanid;
	double oldTime = m_lastStamps[ch];
	m_lastStamps[ch] = newTime;
    
	if (newTime == oldTime) {
	    std::cerr << "**WARNING: module " << m_nModuleNumber
		      << " channel " << ch
		      << " time is not incresing at timestamp " << newTime
		      << std::endl;
	}
	if (newTime < oldTime) {
	    std::cerr << "**ERROR: module " << m_nModuleNumber
		      << " channel " << ch
		      << " time went backwards!!! Previous timestamp: "
		      << oldTime << " current timestamp: " << newTime
		      << std::endl;
	}
    }
    
} // Namespace.
