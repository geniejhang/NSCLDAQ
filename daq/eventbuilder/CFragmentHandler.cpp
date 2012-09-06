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
#include <assert.h>

#include <sstream>
#include <algorithm>
#include <functional>

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
    m_fBarrierPending(false)
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
    * Using 2* the build window below forces
    * the builds to be batched which hopefully run the output stages
    * more efficiently. Getting a barrier event requires a build since
    * we  may never hit the timestamp requirement.
    */
    if ((m_nNewest - m_nOldest) > m_nBuildWindow*2) {
      flushQueues();
    }
    // If all live queues have barriers at the front we need
    // to flush too...the type of flush depends on whether there
    // are dead sources:

    if (countPresentBarriers() == m_liveSources.size()) {

      std::vector<EVB::pFragment> barrierFrags;

      // If there are no dead sources its complete

      if (m_liveSources.size() == m_FragmentQueues.size()) {

	generateCompleteBarrier(barrierFrags);
	
      } else {
	// otherwise it's not complete.

	generateMalformedBarrier(barrierFrags);

      }
      observe(barrierFrags);
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
    m_OutputObservers.push_back(pObserver);
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
        m_OutputObservers.begin(), m_OutputObservers.end(), pObserver);
    if (p != m_OutputObservers.end()) {
        m_OutputObservers.erase(p);
    }
}
/**
 * addDataLateObserver
 *
 *  Add an observer to the list of objects that are invoked when 
 *  a data late event occurs See dataLate below for information about what
 *  that means.
 * 
 * @param pObserver - pointer to the  new observer.
 */
void
CFragmentHandler::addDataLateObserver(CFragmentHandler::DataLateObserver* pObserver)
{
  m_DataLateObservers.push_back(pObserver);
}
/**
 * removeDataLateObserver
 *   
 * Removes an existing data late observer from the observer list.
 * note that it is not an error to attempt to remove an observer that does
 * not actually exist.
 *
 * @param pObserver - Pointer to the observer to remove.
 */
void
CFragmentHandler::removeDataLateObserver(CFragmentHandler::DataLateObserver* pObserver)
{
  std::list<DataLateObserver*>::iterator p = find(
					  m_DataLateObservers.begin(), m_DataLateObservers.end(),
					  pObserver);
  if (p != m_DataLateObservers.end()) {
    m_DataLateObservers.erase(p);
  }
}
/**
 * addBarrierObserver
 *
 *  Barrier observers are called when a successful barrier has been 
 *  dispatched.  A successful barrier is one where all input queues
 *  have a barrier by the time timestamps for events following the
 *  barrier have gone past the build interval.
 *
 *  Observers are called passing a vector of pairs where each pair is
 *  a source Id and the type of barrier contributed by that source.
 *  It is really applciation dependent to know if mixed barrier types
 *  are errors, or normal.  No judgement is passed here.
 *
 * @param pObserver - Pointer to a BarrierObserver class which implements
 *                    operator()(std::vector<std::pair<uint32_t, uint32_t> >);
 */
void
CFragmentHandler::addBarrierObserver(CFragmentHandler::BarrierObserver* pObserver)
{
  m_goodBarrierObservers.push_back(pObserver);
}
/**
 * removeBarrierObserver
 *
 *  Remove a barrier observer from the set of observers that get called on a
 *  completed barrier.  It is a no-op to try to remove an observer that is not
 *  in the list.
 *
 * @param pObserver - pointer to the observer to remove.
 */
void
CFragmentHandler::removeBarrierObserver(CFragmentHandler::BarrierObserver* pObserver)
{
  std::list<BarrierObserver*>::iterator p = std::find(m_goodBarrierObservers.begin(),
						      m_goodBarrierObservers.end(), pObserver);
  if (p != m_goodBarrierObservers.end()) {
    m_goodBarrierObservers.erase(p);
  }
}
/**
 * addPartialBarrierObserver
 *
 * In the event a barrier is only partially made (some input queues are missing
 * their barrier fragments, a list of partial barrier observers is invoked.
 * This method adds a new partial barrier observer to the end of the list
 * of observers for this event.
 *
 * @param pObserver - Pointer to the observer to add.
 */
void
CFragmentHandler::addPartialBarrierObserver(CFragmentHandler::PartialBarrierObserver* pObserver)
{
  m_partialBarrierObservers.push_back(pObserver);
}
/**
 * removePartialBarrierObserver
 *
 *  Removes a partial barrier observer.
 *
 * @param pObserver - Partial barrier
 */
void
CFragmentHandler::removePartialBarrierObserver(CFragmentHandler::PartialBarrierObserver* pObserver)
{
  std::list<PartialBarrierObserver*>::iterator p = std::find(m_partialBarrierObservers.begin(),
							     m_partialBarrierObservers.end(), 
							     pObserver);
  if (p != m_partialBarrierObservers.end()) {
    m_partialBarrierObservers.erase(p);
  }
}

/**
 * flush
 * 
 * There come times when it is necessary to just build event fragments
 * until there are none left. This method does that.
 *
 */
void
CFragmentHandler::flush()
{

  flushQueues(true);
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
/**
 * createSourceQueue
 *
 *  Creates a fragment queue for a source id.  This is called to set up 
 *  an  initial set of a-priori queues in order to make initial barrier
 *  handling simpler.  If the queue already exists, this is a no-op.
 *
 * @param[in] id  - source id of the queue.
 */
void
CFragmentHandler::createSourceQueue(uint32_t id)
{
  SourceQueue& queue = m_FragmentQueues[id]; // creates if needed.
  m_liveSources.insert(id);		     // Sources start live.
}

/**
 * markSourceFailed
 *
 * Marks a source as failed.  This just removes it from the live sources
 * set but maintains its input queue.  If there are dead sources,
 * - Barrier synchronization can proceed without them.
 * - All barriers synchronizations are considered incomplete.
 * 
 * @note receipt of a fragment from a source automatically makes it live again. 
 *
 * @param[in] id - Id of the source to mark, as dead.
 */
void
CFragmentHandler::markSourceFailed(uint32_t id)
{
  m_liveSources.erase(id);
  
  // If there's a pending barrier synchronization and all of the missing
  // sources are dead do an incomplete barrier.
  //
  if(m_fBarrierPending) {
    if (countPresentBarriers() == m_liveSources.size()) {
      std::vector<EVB::pFragment> sortedFragments;
      generateMalformedBarrier(sortedFragments);
      observe(sortedFragments);
    }
  }
}

/*---------------------------------------------------------------------
 ** Utility methods (private).
 */

/**
 * flushQueues
 *
 *  Flush the output queues to the observers.  By default, this flushes
 *  queues until the oldest queue element is 'too new' to flush.
 *  If, however completly is true, queues are fluhsed until empty.
 *
 * @param completely - If true, queues are flushed until empty.
 *                     otherwise, the build window and m_nNewest determine
 *                     when the flush stops.
 *                     Once the events are ordered into a vector, the
 *                     observers are called to deal with them
 *                     and storage associated with them deleted.
 */
void
CFragmentHandler::flushQueues(bool completely)
{
  std::vector<EVB::pFragment> sortedFragments;
  while (!(queuesEmpty()) &&
	 (completely | ((m_nNewest - m_nOldest) > m_nBuildWindow))) {
    
    ::EVB::pFragment p = popOldest();
    if (p) {
      sortedFragments.push_back(p);
    } else if (m_fBarrierPending) {
      goodBarrier(sortedFragments); //  most likely good barrier.
    } else {
      assert(0);		// Should never get p == 0 wthout a barrier pending.
    }
    
  }

  // If a complete flush and barrier is still pending we have a malformed barrier:
  // - recurse to process the frags behind the barrier.
  //
  if (m_fBarrierPending && completely) {
    generateMalformedBarrier(sortedFragments);
    observe(sortedFragments);    
    flushQueues(completely);
  } else {
    observe(sortedFragments);
  }

    
}

/**
 * popOldest
 *
 *   Remove an oldest fragment from the sources queue and update m_nOldest
 *
 *   @return ::EVB::pFragment - pointer to a fragment whose timestamp
 *           matches m_nOldest
 *   @retval - Null is returned if there are no non-barrier events in the
 *             queues.
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
      if (!p->second.empty()) {
	::EVB::pFragment pFrag = p->second.front();
	
	// We can only process non-barriers.
	
	if (pFrag->s_header.s_barrier == 0) {
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
	} else {
	  m_fBarrierPending = true;	// There's at least on barrier fragment.
	}
      }
    }
    if (pOldest) {
      m_nOldest = nextOldest;
    }
    return pOldest;
}

/**
 * observe
 *
 * Invoke each observer for the event we've been passed.
 *
 * @param event - Gather vector for the event.
 *
 * @note - the storage associated with the fragments in the vector
 *         is released after all observers have been called.
 *         If observers want to maintain fragments they will therefore need
 *         to copy them.
 */
void
CFragmentHandler::observe(const std::vector<EVB::pFragment>& event)
{
    std::list<Observer*>::iterator p = m_OutputObservers.begin();
    while(p != m_OutputObservers.end()) {
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
 * dataLate
 * 
 *  This method is called by addFragment in the event a fragment timestamp
 *  is older than m_nNewest by more than m_nBuildWindow, indicating that
 *  the data should have been output earlier. At this time,
 *  the data has not yet been put in the data source queue.
 *  We will pass the fragment along with the value of the largest timestamp
 *  to each element of the m_DataLateObservers list.
 *
 * @param fragment - Reference to the fragment that is late.
 */
void 
CFragmentHandler::dataLate(const ::EVB::Fragment& fragment)
{
  std::list<DataLateObserver*>::iterator p = m_DataLateObservers.begin();
  while (p != m_DataLateObservers.end()) {
    DataLateObserver* pObserver = *p;
    (*pObserver)(fragment, m_nNewest);
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
    uint64_t timestamp = pHeader->s_timestamp;
    bool     isBarrier = pHeader->s_barrier != 0;


    memcpy(&(pFrag->s_header), pHeader, sizeof(EVB::FragmentHeader));
    memcpy(pFrag->s_pBody, pFragment->s_body, pFrag->s_header.s_size);

    // If the timestamp is null, assign the newest timestamp to it:

    if (timestamp == NULL_TIMESTAMP) {
      timestamp = m_nNewest;
      pFrag->s_header.s_timestamp = timestamp;
    }
    
    // If the timestamp is late we need to invoke datalate on this fragment:
    // though barrier event timestamps are meaningless:

    if (!isBarrier && (timestamp < m_nNewest) && ((m_nNewest - timestamp) > m_nBuildWindow)) {
      dataLate(*pFrag);
    }
    
    // Get a reference to the fragment queue, creating it if needed:
    
    SourceQueue& destQueue(m_FragmentQueues[pHeader->s_sourceId]);
    destQueue.push(pFrag);
    m_liveSources.insert(pHeader->s_sourceId); // having a fragment makes a source live.
    
    // update newest/oldest if needed -- and not a barrier:
    // 
    // Since data can come out of order across sources it's possible
    // to update oldest as well as newest.
    if (!isBarrier) {
      if (timestamp < m_nOldest) m_nOldest = timestamp;
      if (timestamp > m_nNewest) m_nNewest = timestamp;
    }

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
/**
 * generateBarrier
 *   
 *  Called to remove all barriers from the fronts of source
 *  queues and add them to the output event list.
 *  
 * @param outputList - the output list.  This is an output parameter and
 *                     we do it this way to avoid copy construction of the whole
 *                     vector.
 *
 * @return BarrierSummary
 * @retval A summary of the barrier fragment types for the sources that
 *         contributed and the sources that did not have a barrier rarin' to go.
 *
 */
CFragmentHandler::BarrierSummary
CFragmentHandler::generateBarrier(std::vector<EVB::pFragment>& outputList)
{
  // Iterate through the output queues and add any
  // barrier events to the outputList.


  BarrierSummary result;

  
  for (Sources::iterator p = m_FragmentQueues.begin(); p!= m_FragmentQueues.end(); p++) {
    if (!p->second.empty()) {
      ::EVB::pFragment pFront = p->second.front();
      if (pFront->s_header.s_barrier) {
	outputList.push_back(pFront);
	p->second.pop();
	result.s_typesPresent.push_back(
            std::pair<uint32_t, uint32_t>(p->first, pFront->s_header.s_barrier)
        );
      } else {
	result.s_missingSources.push_back(p->first);
      }
    } else {
      result.s_missingSources.push_back(p->first); // just as missing if the queue is empty.
    }
  }
  m_fBarrierPending = false;
  findOldest();

  return result;
  
}
/**
 * generateMalformedBarrier
 *
 *  Called when we have finished processing output events but there is an 
 *  incomplete barrier. *  this is an error...but we need to flush those
 *  fragments.
 *
 * @param outputList - Output fragment list (see above).
 */
void
CFragmentHandler::generateMalformedBarrier(std::vector<EVB::pFragment>& outputList)
{
  BarrierSummary bs = generateBarrier(outputList);
 
  partialBarrier(bs.s_typesPresent, bs.s_missingSources);

}
/**
 * goodBarrier
 *
 *   Generate a complete barrier and fire the observers associated with them.

 * @param outputList - Output fragment list (see above).
 */
void
CFragmentHandler::goodBarrier(std::vector<EVB::pFragment>& outputList)
{
  BarrierSummary bs = generateBarrier(outputList);

  // If there's a non empty missing sources this is a partial barrier actually.

  if (bs.s_missingSources.empty()) {
    observeGoodBarrier(bs.s_typesPresent);
  } else {
    partialBarrier(bs.s_typesPresent, bs.s_missingSources);
  }
}
/**
 * findOldest
 *
 * When a barrier (even a partial one) we may not have a correct value for
 * m_nOldest.  This method re-determines the oldest fragment by examing all
 * nonempty fragment queue's front element.  The m_nOldest is set to the timesstamp
 * in the oldest fragment or to m_nNewest if all queues are emtpy.
 */
void
CFragmentHandler::findOldest()
{
  m_nOldest = m_nNewest;		// Automatically right if all queues are empty.

  for (Sources::iterator p = m_FragmentQueues.begin(); p != m_FragmentQueues.end(); p++) {
    if (!p->second.empty()) {
      ::EVB::pFragment pf = p->second.front();
      if (pf->s_header.s_timestamp < m_nOldest) {
	m_nOldest = pf->s_header.s_timestamp;
      }
    }
  }
  

}
/**
 * goodBarrier
 *  
 * Fire off all of the good barrier observers.
 *
 * @param types - Vector of pairs. Each pair contains, in order, the data source ID
 *                and the barrier event type committed to output for that data source.
 */
void
CFragmentHandler::observeGoodBarrier(std::vector<std::pair<uint32_t, uint32_t> >& types)
{

  for (std::list<BarrierObserver*>::iterator p = m_goodBarrierObservers.begin();
       p != m_goodBarrierObservers.end(); p++) {
    (*p)->operator()(types);
  }

}
/**
 * partialBarrier:
 * 
 * Fire off all partial barrier observers.
 *
 * @param types - Vector of pairs as described in goodBarrier above.
 * @param missingSources - Vector of source ids that did not have a barrier.
 */
void
CFragmentHandler::partialBarrier(std::vector<std::pair<uint32_t, uint32_t> >& types, 
				 std::vector<uint32_t>& missingSources)
{

  for (std::list<PartialBarrierObserver*>::iterator p = m_partialBarrierObservers.begin();
       p != m_partialBarrierObservers.end(); p++) {
    (*p)->operator()(types, missingSources);
  }

}
/**
 * countPresentBarriers
 *
 *  Counts the number of queues with barriers that are at their heads.
 *  
 * @return size_t - number of fragment queues that have barriers at their head.
 */
size_t
CFragmentHandler::countPresentBarriers() const
{
  size_t count;
  for (Sources::const_iterator p = m_FragmentQueues.begin(); p != m_FragmentQueues.end(); p++) {
    const SourceQueue& queue(p->second);
    
    EVB::pFragment pFront = queue.front();
    if (pFront->s_header.s_barrier) count++;
  }
  return count;
}
