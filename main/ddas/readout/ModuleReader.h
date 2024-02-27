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
 * @file ModuleReader.h
 * @brief Provides a class that can read data from a Pixie-16 module.
 */

#ifndef MODULEREADER_H
#define MODULEREADER_H

#include <stdint.h>

#include <deque>

#include "BufferArena.h"

/** @namespace DDASReadout */
namespace DDASReadout {
    class ZeroCopyHit;
    
    /**
     * @class ModuleReader
     * @brief A minimal-copy module reader for Pixie-16 modules.
     * @details
     * It tries to provide for minimal-copy readout from the module by 
     * maintining a BufferArena into which data are read. Data are then 
     * parsed into ZeroCopyHits. Pointers to these ZeroCopyHits are placed 
     * into dequeues and  made available to the caller.
     * 
     * @note The zero-copy hits themselves can be recycled.
     *     
     */

    class ModuleReader {
    public:
	typedef std::pair<ModuleReader*, ZeroCopyHit*> HitInfo;
	typedef std::deque<HitInfo> HitList;
    private:
	typedef std::deque<ZeroCopyHit*> HitPool;
    private:
	unsigned    m_nModuleNumber;         //!< Module index in crate.
	unsigned    m_nExpectedEventLength; //!< From e.g. ModEvLen.txt.
	double      m_tsMultiplier; //!< Conversion factor raw timestamp -> ns.
	BufferArena m_freeBuffers;  //!< Storage comes from here.
	HitPool     m_freeHits;     //!< Hit pool.
	double      m_lastStamps[16]; //!< Last timestamp for each channel.
    public:
	uint32_t    m_moduleTypeWord; //!< Id word: bits, revision, MSPS.
    
    public:
	/**
	 * @brief Constructor.
	 * @param module Module number (needed for the read).
	 * @param evtlen Expected event length.
	 * @param moduleType The module type word.
	 * @param timeMultiplier Time calibration multiplier (default=1.0)
	 */
	ModuleReader(
	    unsigned module, unsigned evtlen, uint32_t moduleType,
	    double timeMultiplier
	    );
	/** @brief Destructor. */
	virtual ~ModuleReader();
    private:
	/**
	 * @brief Copy constructor.
	 * @param rhs Reference to the object we copy-construct from.
	 */
	ModuleReader(const ModuleReader& rhs);
	/**
	 * @brief Assignment operator.
	 * @param rhs Reference to the object we assign to the lhs.
	 * @return Reference to lhs object.
	 */
	ModuleReader& operator=(const ModuleReader& rhs);
    public:
	/**
	 * @brief Read a block of data and marshall it into a hit list.
	 *
	 * @param hits The parsed hit list.
	 * @param nWords Maximum read size. If necessary this is reduced to a
	 *     size that is a multiple of the event length. Note that this 
	 *     value and the m_nExpectedEventLength are in uint32_t units.
	 * @return Number of words actually read.
	 */
	size_t read(HitList& hits, size_t nWords);
	/**
	 * @brief Free a hit back to its appropriate hit pool.
	 * @param hit Hit information.
	 */
	static void freeHit(HitInfo& hit);
	/**
	 * @brief Get the module number.
	 * @return The module number.
	 */
	unsigned module() const { return m_nModuleNumber; }
	/**
	 * @brief Reset module last timestamps.
	 */
	void reset();
    private:
	/**
	 * @brief Creates a hit list that contains the events in a buffer read
	 * from the system. Complains if any event is not the correct size.
	 * @param[out] hits Reference the hit list into which these hits will 
	 *     be appended.
	 * @param[inout] pBuffer The buffer containing the events. Note that 
	 *     since hits are zero copy the buffer's reference count will be 
	 *     incremented once for each event found.
	 * @param[in] nUsedWords Number of words read into the buffer.
	 * @throw std::length_error If any of the hits in the buffer is not 
	 *     the correct size (as defined by m_nExpectedEventLength).
	 */
	void parseHits(
	    HitList& hits, ReferenceCountedBuffer& pBuffer, size_t nUsedWords
	    );
	/**
	 * @brief Allocate a new ZeroCopyHit.
	 * @return Pointer to a new hit. If possible, this comes from the hit 
	 * pool. If not, a new one is created and, when it's finally freed, it 
	 * will go back to the hit pool. The goal is that in the end the hit 
	 * pool will be large enough to satisfy all request without dynamic 
	 * memory allocation.
	 */
	ZeroCopyHit* allocateHit();
	/**
	 * @brief Determine whether a parsed hit has a good timestamp.
	 * @param pHit Pointer to the hit.
	 */
	void checkOrder(ZeroCopyHit* pHit);
    };
 
} // Namespace.

#endif
