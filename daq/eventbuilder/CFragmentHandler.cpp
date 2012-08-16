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
#include "CFragmentHandler.h"
#include "fragment.h"

#include <string.h>

#include <sstream>
#include <algorithm>


/*---------------------------------------------------------------------
** Static  data:
*/

CFragmentHandler* CFragmentHandler::m_pInstance(0);

static const uint64_t DefaultBuildWindow(1000000); 

/*--------------------------------------------------------------------------
 ** Creationals: Note this is a singleton, constructors are private.
 */

/**
 * CFragmentHandler
 *
 * constructor:
 *   - m_nNewest -> 0, m_nOldest -> maxint64.
 *   - the build window and coincidence windows are set to some
 *     default values -- they should not be left that way.
 *   - m_pInstance -> this.
 */
CFragmentHandler::CFragmentHandler() :
    m_nOldest(UINT64_MAX),
    m_nNewest(0),
    m_nCoincidenceWindow(0)             // Exact matching required.
{
    m_nBuildWindow = DefaultBuildWindow;
    m_pInstance = this;
}

/**
 * getInstance
 *
 * This is the public creational.  If m_pInstance doesn't exist,
 * a new CFragmentHandler is created.
 *
 * @return CFragmentHandler*
 * @retval - A pointer to the unique instance of the CFragmentHandler.
 *           note this may have been created by this call.
 */
CFragmentHandler*
CFragmentHandler::getInstance()
{
    // If needed, construct the object:
    
    if (!m_pInstance) {
        new CFragmentHandler();    // Constructor sets m_pInstance.
    }
    return m_pInstance;
}

/*--------------------------------------------------------------
 ** Object operations.
 */

/**
 * addFragments
 *   Adds a set of fragments to their queues.  The fragments
 *   are passed in as an array of flattened fragments with
 *   a total byte count for the fragments.
 *   - Fragments are put in the right queues.
 *   - Where appropriate, the newest/oldest events are updated.
 *   - If appropriate, events are built.
 *
 *   @param nSize      - Number of bytes worth of event fragment data.
 *   @param pFragments - Pointer to the first fragment.
 *
 *   @note This method is a no-op if nSize is 0.
 *   @throw std::string exception if there's an inconsistency between
 *          the fragment sizes and nSize so that fragments
 *          don't end when nSize is exactly zero.
 */
void
CFragmentHandler::addFragments(size_t nSize, EVB::pFlatFragment pFragments)
{
    while (nSize) {
      EVB::pFragmentHeader pHeader = &(pFragments->s_header);
        size_t fragmentSize = totalFragmentSize(pHeader);
        if((pHeader->s_size + sizeof(EVB::FragmentHeader)) > nSize) {
            std::stringstream s;
            s << "Last fragment has too many bytes: " << nSize
              << " bytes in fragment group but " << fragmentSize
              << " bytes are in the last fragment!";
            throw s.str();
        }
        
        addFragment(pFragments);

        // Point to the next fragment.
        
        char* pNext = reinterpret_cast<char*>(pFragments);
        pNext      += fragmentSize;
        pFragments  = reinterpret_cast<EVB::pFlatFragment>(pNext);
    }
    /**
     * A bit of subtlety here:
     * The builds are done in batches if possible (hence the window*2).
     * We want to be sure that no events overhang the the build window
     * hence the addition of the coincidence window to the stuff that
     * figures out when to stop.
     */
    if ((m_nNewest - m_nOldest) > m_nBuildWindow*2) {
        while (!(queuesEmpty() &&
           (m_nNewest - m_nOldest) > (m_nBuildWindow + m_nCoincidenceWindow))) {
            buildEvent();
        }
    }
}
/**
 * setBuildWindow
 * 
 * Set the build window.  The build window determines how far apart the
 * oldest and newest event can get in time before events are built.
 * 
 * @param windowWidth - Timestamp ticks in the build window.
 */
void
CFragmentHandler::setBuildWindow(uint64_t windowWidth)
{
    m_nBuildWindow = windowWidth;
}
/**
 * setConicidenceWindow
 *
 *  Set the maximum number of ticks fragments can differ by and still live in
 *  the same event..
 *
 *  @param timeDifference - The new coincidence window width.
 */
void
CFragmentHandler::setCoincidenceWindow(uint64_t timeDifference)
{
    m_nCoincidenceWindow = timeDifference;
}
/**
 * addObserver
 *
 * The fragment handler does not really  know how to deal with built
 * events.  Chunks of code (like an output stage) that want to know
 * about events that have been built register 'observer' objects
 * when an event has been built all registered observers are
 * called with a gather vector of pointer to the fragments that make
 * up the event passed as a parameter.
 *
 * This method registers an event built observer.
 *
 * @param pObserver - Pointer to the observer object to register.
 */
void
CFragmentHandler::addObserver(::CFragmentHandler::Observer* pObserver)
{
    m_Observers.push_back(pObserver);
}
/**
 * removeObserver:
 *
 * Removes an observer that was registerd by addObserver
 * This method is a no-op if there are no such observers as
 *
 * @param pObserver - Pointer to the observer to remove.
 */
void
CFragmentHandler::removeObserver(::CFragmentHandler::Observer* pObserver)
{
    std::list<Observer*>::iterator p = find(
        m_Observers.begin(), m_Observers.end(), pObserver);
    if (p != m_Observers.end()) {
        m_Observers.erase(p);
    }
}

/**
 * flush
 * 
 * There come times when it is necessar to just build event fragments
 * until there are none left. This method does that.
 *
 */
void
CFragmentHandler::flush()
{
    while(!queuesEmpty()) {
        buildEvent();
    }
    // reset newest/oldest to initial
    
    m_nNewest = 0;
    m_nOldest = UINT64_MAX;
}

/*-------------------------------------------------------------------]
 ** Utility methods (private).
 */

/**
 * Build an event.  An event is built as a gather vector of
 * pointers to event fragments that have been removed from
 * their fragment queues:
 * - all fragments in all queues whose time is different from
 *   m_nOldest by less than m_nCoincidence window are put in the event.
 * - A running new 'oldest' is computed.
 * - observe() is run to invoke the observers.
 * - The fragments are freed.
 * - life goes on ;-)
 */
void
CFragmentHandler::buildEvent()
{
    Builder b(m_nCoincidenceWindow, m_nOldest);
    Builder& bref(b);
    
    // Let the builder build up the event and adjust the 
    
    for_each(m_FragmentQueues.begin(), m_FragmentQueues.end(), bref);
    
    std::vector<EVB::pFragment>& event(b.getEvent());
    observe(event);
    
    // Kill off the fragments:
    
    for_each(event.begin(), event.end(), freeFragment);
    
    // Update the oldest event (building does not change newest).
    
    m_nOldest = b.getOldest();

}
/**
 * observe
 *
 * Invoke each observer for the event we've been passed.
 *
 * @param event - Gather vector for the event.
 */
void
CFragmentHandler::observe(const std::vector<EVB::pFragment>& event)
{
    std::list<Observer*>::iterator p = m_Observers.begin();
    while(p != m_Observers.end()) {
        Observer* pObserver = *p;
        (*pObserver)(event);
        p++;
    }
}
/**
 * addFragment
 *
 * Add a single event fragment to the appropriate event queue.
 * The fragment is copied so that ownership of the parameter remains
 * with the caller.  allocateFragment is used to create the fragment
 * storage adnfreeFragment should be used to release that storage
 * (and that is done by buildEvent() typically).
 *
 * @param pFragment - Pointer to the flattened fragment.
 * 
 * @note This method can also alter the value of m_nNewest if its
 *       timestamp says it is the newest fragment.
 */
void
CFragmentHandler::addFragment(EVB::pFlatFragment pFragment)
{
    // Allocate the fragmentand copy it:
    
    EVB::pFragmentHeader pHeader = &pFragment->s_header;
    EVB::pFragment pFrag         = allocateFragment(pHeader);
    
    memcpy(&(pFrag->s_header), pHeader, sizeof(EVB::FragmentHeader));
    memcpy(pFrag->s_pBody, pFragment->s_body, pFrag->s_header.s_size);
    
    // Get a reference to the fragment queue, creating it if needed:
    
    SourceQueue& destQueue(m_FragmentQueues[pHeader->s_sourceId]);
    destQueue.push(pFrag);
    
    // update newest/oldest if needed:
    // Since data can come out of order across sources it's possible
    // to update oldest as well as newest.
    uint64_t timestamp = pHeader->s_timestamp;
    if (timestamp < m_nOldest) m_nOldest = timestamp;
    if (timestamp > m_nNewest) m_nNewest = timestamp;
    
}
/**
 * totalFragmentSize
 *
 * Computes the total size of a fragment.  This is just the size
 * of the payload + size of the header.
 *
 * @param pHeader - Pointer to an event builder fragment header.
 *
 * @return size_t
 * @retval total size of fragment descsribed by the header.
 */
size_t
CFragmentHandler::totalFragmentSize(EVB::pFragmentHeader pHeader)
{
    return pHeader->s_size + sizeof(EVB::FragmentHeader);
}

/**
 * queuesEmpty
 *   Returns true if all data source queues are empty.
 *
 *  @return bool
 *
 *  @note we don't use for_each as short of tossing an exception
 *        there's no way to short circuit the loop.
 */
bool
CFragmentHandler::queuesEmpty()
{
    for(Sources::iterator p = m_FragmentQueues.begin();
        p != m_FragmentQueues.end(); p++) {
        if (!p->second.empty()) return false;
    }
    return true;
}
/*-----------------------------------------------------------
 ** Locally defnie classers
 */

/**
 *   @class CFragmentHandler::Builder
 *
 *   A visitor of each element of the Sources map.
 *   This builds up an event with each visitation by pulling
 *   off queue elements that are within a coincidence interval.
 *   The oldest fragment not added to the event is also built up
 *   during the iteration.
 */

/**
 * Builder
 *
 *   Constructor:
 *       - Save the coincidence interval.
 *       - Set the oldest stamp to UINT64_MAX.
 *
 *   @param interval - Build coincidence interval.
 *   @param oldest   - The oldest fragment in all queues.
 */
CFragmentHandler::Builder::Builder(uint64_t interval, uint64_t oldest) :
    m_nOldestNotBuilt(UINT64_MAX),
    m_nOldestCurrent(oldest),
    m_nCoincidenceInterval(interval)
{}

/**
 *  operator()
 *
 *     Function call operator.  This is called for each node
 *     of the Sources map.  It removes all queue elements
 *     that are within m_nCoincidenceInterval from the queue
 *     and adds them to the event that is being built up.
 *     If necesary, m_nOldestNotBuilt is updated.
 *
 *    @param source - reference to the map source element (a pair).
 */
void
CFragmentHandler::Builder::operator()(CFragmentHandler::SourceElementV& source)
{
    uint64_t latestAllowed = m_nOldestCurrent + m_nCoincidenceInterval;
    SourceQueue& q(source.second);
    
    while ((!q.empty()) && (q.front()->s_header.s_timestamp < latestAllowed)) {
        m_Event.push_back(q.front());
        q.pop();
        
    }
    // Update oldest if necessary:
    
    if (!q.empty() && (q.front()->s_header.s_timestamp < m_nOldestNotBuilt)) {
        m_nOldestNotBuilt = q.front()->s_header.s_timestamp;
        
    }
}
/**
 * getEvent
 *    Return a reference to the built event:
 *
 * @return std::vector<pFragment>&
 */
std::vector<EVB::pFragment>&
CFragmentHandler::Builder::getEvent()
{
    return m_Event;
}
/**
 * Get the new oldest timestamp.
 *
 * @return uint64_t
 */
uint64_t
CFragmentHandler::Builder::getOldest() const
{
    return m_nOldestNotBuilt;
}
