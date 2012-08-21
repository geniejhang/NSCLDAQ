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
    m_nNewest(0)
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
    ** Using 2* the build window above forces
    ** the builds to be batched which hopefully run the output stages
    ** more efficiently:
     */
    if ((m_nNewest - m_nOldest) > m_nBuildWindow*2) {
        std::vector<EVB::pFragment> sortedFragments;
        while (!(queuesEmpty()) &&
	       ((m_nNewest - m_nOldest) > m_nBuildWindow)) {

		 sortedFragments.push_back(popOldest());
    
        }
        observe(sortedFragments);
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
    std::vector<EVB::pFragment> fragments;
    while(!queuesEmpty()) {
        fragments.push_back(popOldest());
    }
    observe(fragments);
    
    // reset newest/oldest to initial
    
    m_nNewest = 0;
    m_nOldest = UINT64_MAX;
}

/**
 * getStatistics
 *
 *   Return information about the fragment statistics.
 *   This can be used to drive a GUI that monitors the status
 *   of the software.
 *
 *   @return CFragmentHandler::InputStatistics
 *   @retval struct that has the following key fields:
 *   - s_oldestFragment - The timestamp of the oldest queued fragment.
 *   - s_newestFragment - The timestamp of the newest queued fragment.
 *   - s_totalQueuedFragments - Total number of queued fragments.
 *   - s_queueStats     - vector of individual queue statistics.
 *                        each element is a struct of type
 *                        CFragmentHandler::QueueStatistics which has the fields:
 *                        # s_queueId - Source id of the queue.
 *                        # s_queueDepth - number of fragments in the queue.
 *                        # s_oldestElement - Timestamp at head of queue.
 */
CFragmentHandler::InputStatistics
CFragmentHandler::getStatistics()
{
    InputStatistics result;
    
    // get the individual chunks:
    
    result.s_oldestFragment = m_nOldest;
    result.s_newestFragment = m_nNewest;
    
    QueueStatGetter statGetter;
    QueueStatGetter& rstatGetter(statGetter);
    
    for_each(m_FragmentQueues.begin(), m_FragmentQueues.end(), rstatGetter);
    
    result.s_totalQueuedFragments = statGetter.totalFragments();
    result.s_queueStats           = statGetter.queueStats();
    
    
    return result;
}
/*-------------------------------------------------------------------]
 ** Utility methods (private).
 */

/**
 * popOldest
 *
 *   Remove an oldest fragment from the sources queue and update m_nOldest
 *
 *   @return ::EVB::pFragment - pointer to a fragment whose timestamp
 *           matches m_nOldest
 *
 *   @note this is all very brute force.  A much quicker algorithm to find
 *         the oldest fragment would be to retain in addtion to m_nOldest
 *         the queue that it was put in...however we still need to iterate
 *         over the queues to update m_nOldest.  This is a bit
 *         short circuited by:
 *         - Keeping track of it as we search for the first match to m_nOldest
 *         - short circuiting the loop if we find another queue with
 *           an m_nOldest match as there can't be anything older than that
 *           by definition.
 */
::EVB::pFragment
CFragmentHandler::popOldest()
{
    uint64_t nextOldest = m_nNewest;   // Must be older than that.
    ::EVB::pFragment pOldest(0);
    for(Sources::iterator p = m_FragmentQueues.begin();
        p != m_FragmentQueues.end(); p++) {
        ::EVB::pFragment pFrag = p->second.front();
        uint64_t stamp = pFrag->s_header.s_timestamp;
        if(!pOldest && (stamp == m_nOldest)) {
            
            // This is the one.
            
            pOldest  = pFrag;
            p->second.pop();
        }
        // Get pFrag again  in case the test above worked.
        // update nextOldest and break if it matches m_nOldest.
        
        pFrag = p->second.front();
        stamp = pFrag->s_header.s_timestamp;
        if (stamp < nextOldest) nextOldest = stamp;
        if (nextOldest == m_nOldest) break;
        
    }
    m_nOldest = nextOldest;
    return pOldest;
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
    // Delete the fragments in the vector as we're done with them now:

    for(int i =0; i < event.size(); i++) {
      freeFragment(event[i]);
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
 ** Locally defined classers
 */
/**
 * @class QueueStateGetter
 *
 * Event source queue visitor that gathers input statistics.
 * This class is a functional and is intended to be called from
 * a for_each loop over the set of input queues.
 * It gathers the total number of fragments as well
 * as the number of fragments in the queue and the oldest fragment in the queue.
 */

/**
 * Construction sets the total fragment count to zero.
 */
CFragmentHandler::QueueStatGetter::QueueStatGetter() :
  m_nTotalFragments(0)
{}


/**
 * operator()
 *   Called for each queue to accumulate stats for that queue.
 *
 *  @param source - reference to that data source queue.
 */
void
CFragmentHandler::QueueStatGetter::operator()(SourceElementV& source)
{
    SourceQueue&  sourceQ(source.second);
    
    QueueStatistics stats;
    stats.s_queueId       = source.first;
    stats.s_queueDepth    = sourceQ.size();
    stats.s_oldestElement = sourceQ.front()->s_header.s_timestamp;
    
    m_nTotalFragments += stats.s_queueDepth;
    m_Stats.push_back(stats);
}
/**
 * totalFragments()
 * 
 * Return the total number of queued elements.
 *
 * @return uint32_t
 */
uint32_t
CFragmentHandler::QueueStatGetter::totalFragments()
{
    return m_nTotalFragments;
}
/**
 * queueStats
 *
 * Return the queue statistics vector:
 *
 * @return std::vector<QueueStatistics>
 *
 */
std::vector<CFragmentHandler::QueueStatistics>
CFragmentHandler::QueueStatGetter::queueStats()
{
    return m_Stats;
}
