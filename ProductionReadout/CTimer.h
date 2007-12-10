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


//////////////////////////CTimer.h file//////////////////////////////////

#ifndef __CTIMER_H  
#define __CTIMER_H

#ifndef __CTIMEREVENT_H
#include <CTimerEvent.h>
#endif

#ifndef __STL_LIST
#include <list>
#ifndef __STL_LIST
#define __STL_LIST
#endif
#endif

/*!
  Encapsulates a timer.  Timers are entities which can schedule events.
  CTimer maintains a set of handlers which can be periodically scheduled
  called CTimedEvents..

  Timers also maintain an elapsed time which indicates how long they have
  been active since resets.

  Pointers to timers events are handed to us:
  - To retain polymorphism
  - In case part of the CTimedEvent hierarchy has no copy constructor.

  It is, however the client's obligation to manage the lifetime of any
  dynamically allocated  CTimedEvent objects.

  */	
class CTimedEvent;
typedef STD(list)<CTimedEvent*> TimerList; //!< List of timed events.
typedef STD(list)<CTimedEvent*>::iterator TimerListIterator;	//!< Iterator to timed event lists.

class CTimer : public CTimerEvent
{ 
private:
  unsigned int m_nIntervalms; //!< Timer interval in milliseconds.
  unsigned int m_nAccumulatedMs; //!< # milliseconds run pior to start since reset.
  unsigned int m_nStartTimeMs;   //!< # Time of Start.
  unsigned int m_nLatency;   //!< Latency estimate for scheduling.
  unsigned int m_nLastTick;  //!< Time of last tick.

  TimerList m_Events;		//!< STL List containing the managed events. 
  
  
public:
  // Constructors, destructors and other cannonical operations: 
  
  CTimer ();                      //!< Default constructor.
  virtual  ~ CTimer ( ) { } //!< Destructor.
  
private:
  CTimer(const CTimer& rhs); //!< Copy constructor.
  CTimer& operator= (const CTimer& rhs); //!< Assignment
  int         operator==(const CTimer& rhs) const; //!< Comparison for equality.
  int         operator!=(const CTimer& rhs) const;
public:

  // Selectors for class attributes:
public:
  
  unsigned int getIntervalms() const {
    return m_nIntervalms;
  }
  
  unsigned int getElapsedms() const;

  TimerListIterator begin() {
    return m_Events.begin();
  }
  TimerListIterator end() {
    return m_Events.end() ;
  }
  TimerList getTimerList() const {
    return m_Events;
  }
  

  
  // Class operations:
public:
  
  void Start (unsigned int ms,unsigned int latency = 0,  bool Reset=false)  ;
  void Stop ()  ;
  void Reset ()  ;
  unsigned int GetElapsedTime ()  ;
  void EstablishEvent (CTimedEvent& rEvent)  ;
  virtual   void OnTimer ()  ;
  
};

#endif
