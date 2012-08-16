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
#ifndef __CFRAGMENTHANDLER_H
#define __CFRAGMENTHANDLER_H

#ifndef __STL_QUEUE
#include <queue>
#ifndef __STL_QUEUE
#define __STL_QUEUE
#endif
#endif

#ifndef __STL_MAP
#include <map>
#ifndef __STL_MAP
#define __STL_MAP
#endif
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __STL_LIST
#include <list>
#ifndef __STL_LIST
#define __STL_LIST
#endif
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif



// Forward definitions:

namespace EVB {
  struct _Header;
  typedef struct _FragmentHeader FragmentHeader, *pFragmentHeader;

  struct _FlatFragment;
  typedef struct _FlatFragment FlatFragment, *pFlatFragment;
  
  struct _Fragment;
  typedef struct _Fragment Fragment, *pFragment;
};
/**
 * @class CFragmentHandler
 *
 *   Singleton class which handles and dispatches fragments.
 *   The main provisions are for:
 *   - A way to add fragments to the input queues.
 *   - A set of observers that can be handed built events when they are
 *     produced.
 *   - A mechanism to force a 'flush-build'  A flush build is one which builds events
 *     until all queues are empty (think end of run or barriers).
 *   - Mechanisms to set the time tolerance of the build and the time window over which events
 *     are accumulated until a build is done.
 *
 * @note There is an assumption that the timestamps will not roll-over
 *       as they are 64 bits wide and even at 100Mhz that provides
 *       for over 50centuries of continuous operation without rollover.
 *       This puts a burden on event sources with narrower widths to 
 *       maintain the upper bits of the timestamp.u
 */
class CFragmentHandler
{
private:
  typedef std::queue<EVB::pFragment> SourceQueue, *pSourceQueue;
  typedef std::map<uint32_t, SourceQueue> Sources, *pSources;
  typedef std::pair<uint32_t, SourceQueue> SourceElement, *pSourceElement;
  typedef std::pair<const uint32_t, SourceQueue> SourceElementV;

public:

  // Observer base class:

  class Observer {
  public:
    virtual ~Observer() {}	// So we can chain destructors.
  public:
    virtual void operator()(const std::vector<EVB::pFragment>& event) = 0; // Passed built event gather maps.
  };

  // Build observer:
private:

  class Builder {
  private:
    std::vector<EVB::pFragment> m_Event;
    uint64_t               m_nOldestNotBuilt;
    uint64_t               m_nOldestCurrent;
    uint64_t               m_nCoincidenceInterval;

  public:
    Builder(uint64_t interval, uint64_t oldest);
    void                    operator()(SourceElementV& source);
   std::vector<EVB::pFragment>& getEvent();
    uint64_t                getOldest() const;
  };

  

private:
  static CFragmentHandler* m_pInstance;	     //< The unique instance of this class.
private:
  uint64_t          m_nOldest;              //< Oldest fragment seen in terms of ticks.
  uint64_t          m_nNewest;              //< Newest fragment seen in terms of ticks.

  uint64_t          m_nBuildWindow;
  uint64_t          m_nCoincidenceWindow;

  std::list<Observer*> m_Observers;

  Sources              m_FragmentQueues;






  // Canonicals/creationals. Note that since this is a singleton, construction is private.

private:
  CFragmentHandler();
  ~CFragmentHandler();		// No need to be virtual since you can't derive this.

  // These are just plain illegal:

  CFragmentHandler(const CFragmentHandler&);
  CFragmentHandler& operator=(const CFragmentHandler&);
  int operator==(const CFragmentHandler&) const;
  int operator!=(const CFragmentHandler&) const;

  // The only public creational is getInstance:

public:
  CFragmentHandler* getInstance();

  // here are the operations we advertised:

public:
  void addFragments(size_t nSize, EVB::pFlatFragment pFragments);

  void setBuildWindow(uint64_t windowWidth);
  void setCoincidenceWindow(uint64_t timeDifference);

  void addObserver(Observer* pObserver);
  void removeObserver(Observer* pObserver);

  void flush();


  // utility methods:

private:
  void   buildEvent();
  void   observe(const std::vector<EVB::pFragment>& event); // pass built events on down the line.
  void   addFragment(EVB::pFlatFragment pFragment);
  size_t totalFragmentSize(EVB::pFragmentHeader pHeader);
  bool   queuesEmpty();
};


#endif
