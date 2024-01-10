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
 * @file CHitManager.h
 * @brief Defines a class to collect and sort hits from modules and outputs 
 * hits within a specified time window.
 */

#ifndef CHITMANAGER_H
#define CHITMANAGER_H

#include <deque>
#include <vector>

#include "ModuleReader.h"

/** @namespace DDASReadout */
namespace DDASReadout {
/**
 * @class CHitManager
 *
 * @details
 * Collects hits from modules and retains them in a sorted deque. 
 * On request, provides hits that were accepted within some sliding
 * time interval. The time interval is defined at construction time
 * and is in units of seconds (1.0E9 timestamp ticks as timestamps are
 * in ns).
 *
 * This module does no storage manager, the receiver of all hits is expected
 * to release any events that have been output.
 */
    class CHitManager {
    private:
	double m_emitWindow; //!< Emission window for dequeing hits in ns.
	/** Sorted hits for all modules in the system. */
	std::deque<ModuleReader::HitInfo> m_sortedHits; 
	bool m_flushing; //!< True if flushing data (e.g. on end run).
    public:
	/** 
	 * @brief Constructor.
	 * @param window The hit emission window in nanoseconds (ns).
	 */
	CHitManager(double window);
	/** @brief Destructor. */
	~CHitManager();

	/**
	 * @brief Add hits from a set of modules.
	 * @param newHits Vector of dequeues of hit information.
	 */
	void addHits(std::vector<std::deque<ModuleReader::HitInfo>>& newHits);
	/** 
	 * @brief Returns true if there's at least one hit that can be output.
	 * @return True if a hit can be output, false otherwise.
	 */
	bool haveHit() const;
	/**
	 * @brief Returns the hit from the front of the deque of sorted hits, 
	 * removing it from the queue.
	 * @return The dequeued hit.
	 */
	ModuleReader::HitInfo getHit();
	/**
	 * @details Clear the sorted hit deque. This means dereferenceing each 
	 * hit as it comes off the dequeue.
	 */
	void clear();
	/**
	 * @brief Set flag to control whether we are flushing data.
	 * @param amI Are we flushing data?
	 */
	void flushing(bool amI) { m_flushing = amI; }
    private:
    
	// Sorting and merging support.

	/**
	 * @brief Merges a vector of sorted dequeues onto the back end of
	 * a deque.
	 * @param[out] result The dequeue into which the elements will be put.
	 * @param[in,out] newHits Vector of input dequeues. These will be 
	 *   emptied by this method.
	 */
	void merge(
	    std::deque<ModuleReader::HitInfo>& result,
	    std::vector<std::deque<ModuleReader::HitInfo>>& newHits
	    );
	/**
	 * @brief Merge new hits into an existing sorted hit list.
	 *  @param result The resulting merged hits.
	 *  @param newHits Sorted input hits.
	 */
	void merge(
	    std::deque<ModuleReader::HitInfo>& result,
	    std::deque<ModuleReader::HitInfo>& newHits
	    );

	/** 
	 * @brief Given two hit info references, returns true if the first one
	 * has a timestamp strictly less than the second.
	 * @param q1 First hit
	 * @param q2 Second hit.
	 * @return bool
	 */
	static bool lessThan(
	    const ModuleReader::HitInfo& q1,
	    const ModuleReader::HitInfo& q2
	    );

	/**
	 * @brief  Given a hit info returns the timestamp of the zero copy hit 
	 * it contains.
	 * @param hit Hit information pair.
	 */
	static double timeStamp(const ModuleReader::HitInfo& hit);
    };

} // Namespace.

#endif
