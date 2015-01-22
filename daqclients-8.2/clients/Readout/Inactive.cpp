/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2008

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/



//  Inactive.cpp
// This class executes the Inactive state of
//  the run state machine. 
//    Enter:
//        If prior state was active, then
//        format an end of run buffer
//   Leave:
//       Set prior state to  our state id.
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
#include "Inactive.h"                               
#include "ReadoutStateMachine.h"
#include "skeleton.h"
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


static const char* Copyright= 
"Inactive.cpp: Copyright 1999 NSCL, All rights reserved\n";

// Functions for class Inactive

//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    unsigned Run ( StateMachine& rMachine )
//  Operation Type:
//     Action override..
//
unsigned 
Inactive::Run(StateMachine& rMachine) 
{
// Executes when the state tirggers.
// Blocking reads are performed to 
// get commands.  These commands are
// mapped to events and then returned to
// the caller for processing
//
// Formal Parameters:
//   StateMachine& rMachine:
//       Reference to the executing statemachine.
//       This is required to be a reference to an object
//        of type ReadoutStateMachine or derived from
//        that class.
// Returns:
//      Event decoded from the command.
// Exceptions:  

  ReadoutStateMachine& rRun((ReadoutStateMachine&)rMachine);

  return rRun.GetCommand();	// Get the next command as an event.

}
//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    void Enter ( StateMachine& rMachine )
//  Operation Type:
//     Action Override.
//
void 
Inactive::Enter(StateMachine& rMachine) 
{
//  Performs state entry operations.
//  These are only performed if
//  the prior state was Active.  Otherwise, we're being 
//  entered as part of initial state processing.
//  If prior state was active, the run is ending so:
//   1. Read and submit scaler buffers.
//   2. Submit an End of Run Buffer.
//
// Formal Parameters:
//     StateMachine& rMachine:
//            Reference to the state machine which is
//            executing.  This is actually a reference to 
//            an object of or derived from ReadoutStateMachine
// Exceptions:  

  ReadoutStateMachine& rRun = (ReadoutStateMachine&)rMachine;
  string PriorState;
  string Active("ACTIVE");
  string Paused("PAUSED");
  
  PriorState = rRun.StateToName(rRun.GetPriorState());
  if((PriorState == Active) ||
     (PriorState == Paused)) {
    rRun.EmitScaler();
    rRun.EmitStop();
    ::endrun();
    daq_EndRun();
    rRun.ResetSequence();
    daq_IncrementRunNumber();
  }
 

}
//////////////////////////////////////////////////////////////////////////
//
//  Function:   
//    void Leave ( StateMachine& rMachine )
//  Operation Type:
//     Action override
//
void 
Inactive::Leave(StateMachine& rMachine) 
{
// Leaves the inactive state.  
// Calls SetPriorState in the
//  RunStateMachine so that the
//  caller, and next state knows we were
//  the prior state.
//
// Formal Parameters:
//     StateMachine& rMachine
//            Reference to the executing state
//            machine.  This is actually an object
//            of or derived from RunStateMachine
//
// Exceptions:  

  ReadoutStateMachine& rRun((ReadoutStateMachine&)rMachine);
  
  rRun.SetPriorState();
}



