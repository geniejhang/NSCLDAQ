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



//  StateMachine.cpp
// Base class which executes a finite state
// automaton (FSA).  An FSA consists of a finite
// set of states and events.  For each state, an
// each event either defines a legal transition
//  to a new state, a non-transition or an
//  illegal transition.
//  This class allows FSA's to be set up
//   and interpreted.
//   States are derived from the State class
//   and inserted into the state machine 
//   along with the allowed transitions prior
//   to executing the Run() member.  The Run()
//   member enters the initial state, which processes
//   stimulii by returning to the caller.  Each transition
//   to a *new* state results in a call to the old state's
//   Leave() member, a call to the new state's Enter()
//   member and one or more calls to the new state's
//   Run() member.
//      This implementation of FSA's  reduces events
//   to those causing legal transitions and those which
//   are illegal.  the Leave() /Enter() calls are only done
//   if the state transition actually resulted in a new state
//   execution.
//   Illegal events call the StateMachine's IllegalTransition
//   virtual function.
//
//     Requires the STL.
     
//
//   Author:
//      Ron Fox
//      NSCL
//      Michigan State University
//      East Lansing, MI 48824-1321
//      mailto:fox@nscl.msu.edu
//
//////////////////////////.cpp file///////////////////////////////////////////

//
// Header Files:
//
#include <config.h>
#include "StateMachine.h"
#include "State.h"
#include "Transition.h"
#include <algorithm>
#include <assert.h>
#include <string.h>
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


static const char* Copyright = 
"StateMachine.cpp: Copyright 1999 NSCL, All rights reserved\n";

//
//  Type definitions:
//

typedef vector<State*>::iterator StateListIterator;
typedef StateMachine::TransitionList::iterator TransitionListIterator;
typedef StateMachine::IdDictionary::iterator   DictionaryIterator;
typedef StateMachine::IdDictionary::const_iterator CDictionaryIterator;
typedef pair<unsigned int, Transition*>  TransitionReference;
//
//   Local classes:
//
class TransitionDumper
{
private:
  const StateMachine* m_pMachine;
  string        m_sCurrentState;
  ostream&      m_Output;
public:
  TransitionDumper(const StateMachine* pMachine, 
		   string StateName, ostream& f) :
    m_pMachine(pMachine),
    m_sCurrentState(StateName),
    m_Output(f) {}
  void operator()(const TransitionReference rTransition) {
    m_Output << m_sCurrentState << ' ' 
             << m_pMachine->EventIdToName((rTransition.second)->getEvent()) 
	     << ' '
             << m_pMachine->StateToName((rTransition.second)->getState())
	     << endl;
  }
};

//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    State* Stimulate ( unsigned nEventId )
//  Operation Type:
//     State Transition
State*
StateMachine::Stimulate(unsigned nEventId)
{
//  Takes a stimulus and reports the next
//  state.   Used within DoTransition to
//  determine if the event id is legal.
//  
//  Formal Parameters:
//       unsigned nEventId:
//             Id of the event which is stimulating
//             the state machine.
//   Returns:
//         NULL - The event produces no legal transition.
//         other  -  Pointer to the new state.  Note that this may
//                      be the same as m_CurrentState.

  if(!m_CurrentState) return (State*)0;	// No next state if no current state.

  // With the current state, get it's transition list.
  //

  unsigned    nStateId = GetCurrentState();
  TransitionList& rAllowed = (m_aTransitions[nStateId]);

  //
  //   The transition list is a map which is indexed on the Event Id.
  //   so we can directly lookup the transition:
  //

  TransitionListIterator li = rAllowed.find(nEventId);
  if(li == rAllowed.end()) return (State*)0; // not an allowed transition.
  Transition* pTransition = (*li).second;    // Transition for nEventId.
  nStateId = pTransition->getState();        // Get dest. state ID for 
  
  return m_StateList[nStateId];
}

//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    void DoTransition ( unsigned nEventId )
//  Operation Type:
//     State Transition
//
void 
StateMachine::DoTransition(unsigned nEventId) 
{
// Performs the indicated state transtion.
//  Stimulate() is called to determine if the
//  event is legal in this state.  If it isn't then
//  OnIllegalTransition() is called, if it is, and the
//  state returned from Stimulate() is different than the
//  m_CurrentState, the old state's Leave() function is
//  called, and the new state's Enter() function is called.
//  We then return to the caller who presumably calls the
//  current state's Run() member, to handle state processing and
//  acqusition of any events which might stimulate further 
//  transitions
//.
//  Formal Parameters:
//       unsigned nEventId:
//           Event Id of the stimulus we need to
//            process.
// Side Effects:
//     m_CurrentState is modified if the state changes.
//
// Exceptions:  

     State* newstate = Stimulate(nEventId);
     if(!newstate) {
        OnIllegalTransition(GetCurrentState(), nEventId);
     } else {
         if(newstate != m_CurrentState) {
             if(m_CurrentState)               // If current state is null.
                 m_CurrentState->Leave(*this);// Can't leave it. 
             m_CurrentState = newstate;
             m_CurrentState->Enter(*this);    // Initialize the new state. 
          }                                   // caller is expected to Run()
    }                                         // the state. 
  
}
//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    int GetCurrentState (  )
//  Operation Type:
//     Selector
//
unsigned
StateMachine::GetCurrentState() 
{
// Returns the id of the current state.
// This is done by a reverse lookup of
//  m_CurrentState in the m_StateList vector..
//  It is assumed that GetCurrentState() is much
//  less common than id to state lookups (which are
//  done for each state transition e.g, and are done
//  by indexing the m_StateList vector.
//
//  Returns:
//     -1     - There is no current state.
//     other  - The id of the current state, integerized.
//
//  >>>>BUGBUG<<<< Actually this is assumption is incorrect.
//                 The current state is a pointer to state which must be
//                 turned back into a state Id and therefore must be done
//                 each state transition consider:
//                 1. Either keeping m_CurrentState as a state id.
//                 2. Keeping the state list as a map with index the
//                    state pointer and value the state index.
//                 3. Keeping both an m_CurrentState pointer and a
//                    m_nCurrentStateId id value.
//

  unsigned  i;
  unsigned          nStates = m_StateList.size();

  for(i = 0; i < nStates; i++) {
    if(m_StateList[i] == m_CurrentState)
      return (int)i;
  }
  return 0;			// Didn't find the state (BUG).
}

//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    bool AddState ( State* pNewState, const string& rStateName )
//  Operation Type:
//     Mutator.
//
bool
StateMachine::AddState(State* pNewState, const string& rStateName) 
{
// Adds a new state to the m_StateList,
// The state is given a sequential state Id.
//  The state name is saved in the m_StateDictionary
//  map allowing a lookup of the state id by name.
//
//  Formal Parameters:
//      State* pNewState:
//           Pointer to the state to add to the state table.
//           It is the caller's responsibility to delete this state
//           when the StateMachine is destroyed.  Typically,
//           we anticipate that the states will be constructed and
//           added in a derived class constructor, and then deleted
//           in the virtual destructor.
//      string&  name:
//           Reference to a name for the state.  State names must be
//           unique.
// Returns:
//       true:
//            The state was added correctly.
//       false:
//             State could not be added (duplicate state name).

  //  First check to ensure that the state is not defined already.
  //  The user is allowed to make a single State object do double duty
  //  by giving it different names, but not allowed to duplicate the name.

  if(m_StateDictionary.find(rStateName) != m_StateDictionary.end()) 
    return false;

  // Now assign a state id and:
  //   1. insert the state in the state vector (at end).
  //   2. Insert the state into m_StateList
  //   3. Insert state-name to state-id correspondence in m_StateDictionary.
  //
  unsigned             nStateId = m_nNextStateId;
  m_nNextStateId++;
  m_StateDictionary[rStateName] = nStateId;
  m_StateList.push_back(pNewState);

  // Add a transition list  for the state:

  TransitionList tl;
  m_aTransitions.push_back(tl);


  //
  // ensure that the state id is doing the right stuff:
  //
  assert(m_StateList.size() == m_nNextStateId);

  // Finally invoke the state's initializer:
  
  pNewState->OnInitialize(*this);

  return true;

}

//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    bool AddEvent ( const string& rEventName )
//  Operation Type:
//     Mutator.
//
bool 
StateMachine::AddEvent(const string& rEventName) 
{
// Adds an event to the list of defined events in the
// m_EventDictionary.  Events have a name and an
// event id.  NameToEventId looks up the event Id
// given the name.  EventIdToName looks up the Name
// given the event id. Event names and Id's must be unique.
// Event Id's are sequentially assigned.  The Event Name
//  is user supplied.
//   
//   Formal Parameters:
//      string&   rEventName:
//            Reference to the name of the new event.
//

  // Action is almost identical to AddState,
  // however, the event is just added to the
  // dictionary.

  if(m_EventDictionary.find(rEventName) != m_EventDictionary.end())
    return false;

  unsigned nEventId = m_nNextEventId++;
  m_EventDictionary[rEventName] = nEventId;

  assert(m_EventDictionary.size() == m_nNextEventId);

  return true;
}

//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    bool DefineTransition ( unsigned nOldStateId, 
//                              unsigned nEventId, 
//                              unsigned nNewStateId )
//    bool DefineTransition (string& OldStateName,
//                             string& EventName,
//                             string& NewStateName)
//  Operation Type:
//     mutator
//
bool 
StateMachine::DefineTransition(unsigned nOldStateId, 
                               unsigned nEventId, 
                               unsigned nNewStateId) 
{
// Defines a state transition.
// A state transition is defined by the triple of:
// Current state, event, new state
// Current state and newstate may be the same.
//
// Formal Parameters:
//     unsigned nOldStateId:
//          The state id of the current state when the
//           event is received.
//     unsigned nEventId:
//          The Id of the event which causes this transition.
//     unsigned nNewStateId:
//          The ID of the state which will be entered as a 
//          result of this event.
//     string& OldStateName:
//        Name of the current state when event is received.
//     string& EventName:
//        Name of the transition event.
//     string&
//        Name of the resultant state.
//
//   
// Returns:
//      false   - If the transition can't be entered usually
//                     due to an invalid event or state id.
//      true  - If the transition was entered.
//

  // ensure that the Ids are legitimate:

  if(nOldStateId >= m_nNextStateId) return false;
  if(nNewStateId >= m_nNextStateId) return false;
  if(nEventId    >= m_nNextEventId) return false;

  //  Now form a transition and enter it into the transition list.

  TransitionList& rtl = m_aTransitions[nOldStateId];
  if(rtl.find(nEventId) != rtl.end())
    return false;		// There's already a transition for this event

  Transition* pTransition = new Transition(nEventId, nNewStateId);
  if(!pTransition) 
    return false;
  rtl[nEventId] = pTransition;

  return true;
}
bool
StateMachine::DefineTransition(const string& OldStateName,
			       const string& EventName,
			       const string& NewStateName)
{
  // This just translates all of the names to id's and calls the previous /
  // member:

  int OldId  = NameToState(OldStateName);
  int EventId= NameToEventId(EventName);
  int NewId  = NameToState(NewStateName);

  // Ensure the id's are valid:

  if((OldId   < 0) ||
     (EventId < 0) ||
     (NewId   < 0)) return false;

  return DefineTransition((unsigned)OldId,
			   (unsigned)EventId,
			   (unsigned)NewId);
}

//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    bool ReadTransitionTable ( istream& fStream=cin )
//  Operation Type:
//     Initializer.
//
bool 
StateMachine::ReadTransitionTable(istream& fStream) 
{
//  Reads a state transition table from file.
//  This function is a convenience function
//   to allow state transition tables to be
//   read in from an ordinary text file.
//   Each line of the file is either blank
//   or contains a transition in the form:
//
//    OldStateName Event NewStateName     comments
//   
//     Events which have not yet been defined are 
//     created and then added to the EventDictionary.
//     It would be nice to do that for states, but impossible
//     since we don't know what class executes the state.
//
//  Formal Parameters:
//       istream&   fTable:
//          Input stream open on the state table file.
//          defaults to cin.
//  Returns:
//         true  - If there were no errors.
//         false - otherwise.
//  >>>BUGBUGBUG<<<
//        Error reporting on the input syntax could be
//        better than the current nonexistent state.
//
// Exceptions:  

  while(!fStream.eof()) {
    string OldState;
    string NewState;
    string Event;
    fStream >> OldState >> Event >> NewState;

    if(OldState == string("")) continue;
    if(strlen(OldState.data())) {
      if(NameToEventId(Event) == -1) { // Add the event:
	AddEvent(Event);
      }
      if(!DefineTransition(OldState, Event, NewState)) {
	return false;
      }
    }
  }
  return true;
    
}

//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    void DumpTransitionTable ( ostream& fOutput=cout )
//  Operation Type:
//     
//
void 
StateMachine::DumpTransitionTable(ostream& fOutput)
{
// Dumps the current state transition table
//  in a format which can be read in with
//  ReadTransitionTable()
//
// Formal Parameters:
//      ostream& fOutput = cout:
//            A reference to a stream on which
//           the transition table will be dumped.
//

  for(unsigned i = 0; i < m_aTransitions.size(); i++) {
    TransitionDumper      DumpTransition(this, StateToName(i), fOutput);
    const TransitionList& rtl = m_aTransitions[i];
    for_each(rtl.begin(), rtl.end(), DumpTransition);
  }

}

//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    string StateToName ( unsigned nStateId )
//  Operation Type:
//     selector
//
string 
StateMachine::StateToName(unsigned nStateId)  const
{
//  Returns the name of a state given the ID.
//  An empty string is returned if the id is unknown.
//
// Formal Paramters:
//       unsigned nState:
//             Id of state to lookup.
// Returns:
//       Name of string or an empty
//       string if there's no match.
//

  string Name;			// Start out with empty string.

  for(CDictionaryIterator i = m_StateDictionary.begin(); 
      i != m_StateDictionary.end(); i++) {
    if(nStateId == (*i).second) {
      Name = (*i).first;
      break;
    }
  }
  // At this point Name is either empty if the loop exited normally, or 
  // contains the name of the state found if the break was executed.

  return Name;

}

//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    int NameToState (const string& rName )
//  Operation Type:
//     selector
//
int 
StateMachine::NameToState(const string& rName)  const
{
// Returns the id of a state given it's name.
//
// Formal parameters:
//     string& rName:
//         reference to the name of the state
//        to lookup.
// Returns:
//      -1  - No such state.
//    other- Id of the state with that name.
//

  CDictionaryIterator i = m_StateDictionary.find(rName);
  if(i != m_StateDictionary.end()) {
    return (int)(*i).second;
  }
  else {
    return -1;
  }

}

//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    string EventIdToName ( unsigned nEvent )
//  Operation Type:
//     selector
//
string 
StateMachine::EventIdToName(unsigned nEvent)  const
{
//  Returns the name of an event given the 
//  event id.
//
// Formal Parameters:
//      unsigned nEvent:
//         ID of the event to lookup.
// Returns:
//      empty string if no such event.
//      name of the event otherwise.

  string Name;

  CDictionaryIterator i;
  for(i = m_EventDictionary.begin(); i != m_EventDictionary.end(); i++) {
    if(nEvent == (*i).second) {
      Name = (*i).first;
      break;
    }
  }
  return Name;

}

//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    int NameToEventId (const string& rName )
//  Operation Type:
//     selector
//
int 
StateMachine::NameToEventId(const string& rName)  const
{
//   Returns the id of an event given its name.
// Formal Parameters:
//     string& rName:
//        reference to the event to return.
//  Returns:
//        -1 if no match else the id of the
//         named state.

  CDictionaryIterator i = m_EventDictionary.find(rName);
  if(i != m_EventDictionary.end()) {
    return (int)(*i).second;
  }
  else {
    return -1;
  }
}

//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    void OnInitialize (  )
//  Operation Type:
//     Override hook
//
void 
StateMachine::OnInitialize() 
{
// Called from the Run() member function to perform any
// initialization required prior to executing the state machine.

  // Default function does nothing.

}

//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    bool OnIllegalTransition ( unsigned nCurrentState, unsigned nEvent )
//  Operation Type:
//     override hook:
//
void
StateMachine::OnIllegalTransition(unsigned nCurrentState, unsigned nEvent) 
{
// Called from DoTransition when an
// illegal event is submitted to the transition
// engine.
//   The default function emits an error
//   indicating the current state and the attempted
//   event.
// 
// Formal Parameters:
//     unsigned nCurrentState:
//         id of the current state.
//      unsigned nEvent:
//         Attempted event.
//
  
  cerr << "StateMachine - Illegal transition attempted" << endl;
  cerr << "    Current state = " << StateToName(nCurrentState) << endl;
  cerr << "    Event         = " << EventIdToName(nEvent) << endl;

}

//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    void OnCleanup ( unsigned nState )
//  Operation Type:
//     override hook.
//
void 
StateMachine::OnCleanup(unsigned nState) 
{
// Can be called by the current state when 
// it is about to exit.
//  
// Formal Parameters:
//     unsigned  nCurrent:
//         Id of current state.
//

//   Default action is a no-op.

}

//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    void Run ( unsigned nInitial )
//  Operation Type:
//     Execution
//
void 
StateMachine::Run(unsigned nInitial) 
{
// Interprets the current state machine.
//
// Formal Parameters:
//     unsigned nInitial:
//         Id of the  initial state.
// 
// Exceptions:  

  // First make the transtion to the initial state.. if this is illegal, then
  // we exit immediately.
  //  >>>BUGBUG<<< Is there a better way to return failure to the caller?
  //

  if(nInitial > m_StateList.size()) { // Should this be an assert?
    return;
  }

  OnInitialize();		// Initialize ourself.

  m_CurrentState = m_StateList[nInitial];
  m_CurrentState->Enter(*this);
  while(1) {
    unsigned stimulus = m_CurrentState->Run(*this);
    DoTransition(stimulus);	// May or may not change m_CurentState.
  }

}

/////////////////////////////////////////////////////////////////////////
// Function:
//   ~StateMachine()
// Operation Type:
//   Destructor
//
StateMachine::~StateMachine()
{
  // The transition list vector of maps contains pointers to dynamicall
  // allocated transitions which must be destroyed:

  for(int i = 0; i < m_aTransitions.size(); i++) {
    TransitionList& rtl = m_aTransitions[i];
    TransitionListIterator tli;
    for(tli = rtl.begin(); tli != rtl.end(); tli++) {
      delete (*tli).second;		// Destroy the pointer.
    }
  }
}
/*!

    Allow an existing state processor to be replaced.
    The new state processor is initialized, and the
    old state processor returned.  If there is no old
    processor, NULL is returned, and the operation is equivalent to AddState.
    If there are transitions to/from the state they will not be affected.

    \param name   : std::string   
       Name of the state to replace
    \param newState : CState*
       Pointer to the new state.
    \return CState*
    \retval NULL - this is a new state.
    \retval !NULL - Pointer to the previous state by that name.
*/
State*
StateMachine::replaceState(string name,
			   State* newState)
{
  int oldId = NameToState(name);
  if (oldId < 0) {
    // New state:

    AddState(newState, name);
    return NULL;
  }
  else {
    State* pOld = m_StateList[oldId];
    m_StateList[oldId] = newState;
    newState->OnInitialize(*this);
    return pOld;
  }
}
