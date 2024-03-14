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
 * @file HitManager.cpp
 * @brief Implements the CHitManager class.
 */

#include "HitManager.h"

#include <algorithm>

#include <ZeroCopyHit.h>
#include <RawChannel.h>

HitManager::HitManager(uint64_t window) : m_nWindow(window)
{}

/**
 * @details
 * On return the deque will be empty.
 */
void
HitManager::addHits(std::deque<DDASReadout::ZeroCopyHit*>& newHits)
{
    sortHits(newHits);  // First sort the incoming hits.
    mergeHits(newHits); // Then merge them into the newHits deque.
}

/**
 * @details
 * Returns false if < 2 hits since no window can be constructed.
 */
bool
HitManager::haveHit()
{
    if (m_sortedHits.size() < 2)
	return false;
    
    auto pfront = m_sortedHits.front();
    auto pback  = m_sortedHits.back();
    
    return ((pback->s_time - pfront->s_time) > m_nWindow);
}

/** 
 * @details
 * On exit, if a hit is returned it has been popped off the deque.
 */
DDASReadout::ZeroCopyHit*
HitManager::nextHit()
{
    DDASReadout::ZeroCopyHit* result;
    if (m_sortedHits.empty()) {
        result = nullptr;
    } else {
        result = m_sortedHits.front();
        m_sortedHits.pop_front();
    }
    
    return result;
}

///////////////////////////////////////////////////////////////////////////////
//  Private members.
//

/** 
 * @brief Static local comparison function to provide < when we have pointers
 * to hits.
 * @return bool
 * @retval true If the timestamp of p1 is < the timestamp of p2.
 * @retval false Otherwise.
 */
static bool
hitCompare(DDASReadout::ZeroCopyHit* p1, DDASReadout::ZeroCopyHit* p2)
{
    return *p1 < *p2;
}

void
HitManager::sortHits(std::deque<DDASReadout::ZeroCopyHit*>& newHits)
{
    std::sort(newHits.begin(), newHits.end(), hitCompare);
}

/**
 * @details
 * We assume that, other than when starting up, the time range covered by 
 * the output deque is much larger than that of the new hits (the window 
 * is seconds where at high rates, which is what we care about optimizing, 
 * the data read from a module will be milliseconds?).
 *
 *  Special cases:
 *  - m_sortedHits is empty: assign it to new hits.
 *  - m_sortedHits.last() <= newHits.front(): just insert the new hits at the 
 *    end of the current sorted list.
 *  - m_sortedHits.front() >= newHits.back(): prepend the new hits.
 *  - Otherwise the following steps are performed:
 *  1. Append the new hits to sorted hits.
 *  2. Search backwards in the existing hits until either we come to the first
 *     existing hit or we come to an element who's time is <= to newhit's front.
 *  3. Do an in-place merge of those two ranges of the sorted_list.
 */
void
HitManager::mergeHits(std::deque<DDASReadout::ZeroCopyHit*>& newHits)
{
    if (m_sortedHits.empty()) {
        m_sortedHits = newHits; // Assign
    } else {
        auto oldFront = m_sortedHits.front();
        auto newBack  = newHits.back();
        if (hitCompare(newBack, oldFront)) { // Prepend
            m_sortedHits.insert(
		m_sortedHits.begin(), newHits.begin(), newHits.end()
		);
        } else {
            // Start by appending:            
            auto newPosition = m_sortedHits.insert(
		m_sortedHits.end(), newHits.begin(), newHits.end()
		);
            auto oldEnd = newPosition;
            --oldEnd;
            while (
		!hitCompare(*oldEnd, *newPosition)
		&& (oldEnd != m_sortedHits.begin())
		) {
                --oldEnd;
            }    
            std::inplace_merge(
		oldEnd, newPosition, m_sortedHits.end(), hitCompare
		);
        }
    }
}
