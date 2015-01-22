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

//
//
// Requires the STL.
//
//  StateMachine.h:
//
//    This file defines the StateMachine class.
//
// Author:
//    Ron Fox
//    NSCL
//    Michigan State University
//    East Lansing, MI 48824-1321
//    mailto:fox@nscl.msu.edu
//
//  Copyright 1998 NSCL, All Rights Reserved.
//
/////////////////////////////////////////////////////////////

#ifndef __STATEMACHINE_H  //Required for current class
#define __STATEMACHINE_H
                   
#ifndef _STL_MAP_H            
#include <map>   
#ifndef _STL_MAP_H
#define _STL_MAP_H
#endif
#endif

#ifndef _STL_STRING_H
#include <string>
#define _STL_STRING_H
#endif

#ifndef _STL_VECTOR_H
#include <vector>
#ifndef _STL_VECTOR_H
#define _STL_VECTOR_H
#endif
#endif

#ifndef _CPP_IOSTREAM_H
#include <Iostream.h>
#define _CPP_IOSTREAM_H
#endif

class State;			// We only reference these in the header
class Transition;		// through pointers and references.

#ifdef _NEED_BOOL_T
#ifndef _BOOL_T
typedef int bool_t;		// In case bool_t not defined by compiler.
#ifndef TRUE
#define TRUE -1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#define _BOOL_T
#endif
#endif
class StateMachine      
{
				// Class attributes.
public:
  typedef STD(map)<STD(string), unsigned > IdDictionary;
  typedef STD(map)<unsigned, Transition* > TransitionList;
private:
  IdDictionary m_StateDictionary;  // Lookup dictionary of state names 
				   // to state ID's.			    
  IdDictionary  m_EventDictionary;// Dictionary of Event name -> Event ids.
  unsigned      m_nNextEventId;	// Number of the next event id.  
				// Event Id's number
				// from zero.
  unsigned      m_nNextStateId;	// Next state id (number from zero).
  State* m_CurrentState;
  STD(vector)<State*>          m_StateList;    // List of state objects.
  STD(vector)<TransitionList>  m_aTransitions; // List of transitions for each state
  
public:
			//Default constructor

  StateMachine () :
   m_nNextEventId(0),
   m_nNextStateId(0),
   m_CurrentState(0) {}
  
  virtual  ~ StateMachine ( );       //Destructor

			// Copy constructor: Not allowed.
private:
  StateMachine (const StateMachine& aStateMachine ) ;

			//Operator= Assignment Operator: Not allowed.
                        
			//Operator== Equality Operator: Inappropriate.

  int operator== (const StateMachine& aStateMachine);

				// Selectors
				// and mutators:
protected:

  const IdDictionary&  getStateDictionary() const
  {
    return m_StateDictionary;
  }
  const IdDictionary& getEventDictionary() const
  {
    return m_EventDictionary;
  }
  unsigned getNextEventId() const
  {
    return m_nNextEventId;
  }
  unsigned getNextStateId() const
  {
    return m_nNextStateId;
  }
  void setCurrentState(State* pNewState)
  {
    m_CurrentState = pNewState;
  }
public:
  State*      GetCurrentStatePtr() { return m_CurrentState; }
  unsigned    GetCurrentState ()  ;
  STD(string) StateToName (unsigned nStateId)  const;
  int         NameToState (const STD(string)& rName)  const;
  STD(string) EventIdToName (unsigned nEvent)  const;
  int         NameToEventId (const STD(string)& rName)  const;
  State*      replaceState(STD(string) name,
			   State*      newState);

				// Operations:
public:

  State* Stimulate (unsigned nEventId)  ;
  bool AddState (State* pNewState, const STD(string)& rStateName)  ;
  bool AddEvent (const STD(string)& rEventName)  ;
  bool DefineTransition (unsigned nOldStateId, // Define transition using
			   unsigned nEventId,    // IDs.
			   unsigned nNewStateId)  ;
  bool DefineTransition(const STD(string)& OldStateName,	 // Def Transitions 
			  const STD(string)& EventName,     //  using Names.
			  const STD(string)& NewStateName);
  bool ReadTransitionTable (STD(istream)& fStream=STD(cin))  ;
  void DumpTransitionTable   (STD(ostream)& fOutput=STD(cout));

  //
  //  Overridable members:
  //
public:
  virtual   void DoTransition (unsigned nEventId)  ;
  virtual   void OnInitialize ()  ;
  virtual   void  OnIllegalTransition (unsigned nCurrentState, unsigned nEvent)  ;
  virtual   void OnCleanup (unsigned nState)  ;
  virtual   void Run (unsigned nInitial)  ;

};

#endif
