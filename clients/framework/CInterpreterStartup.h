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

// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//
// Copyright 

#ifndef __CINTERPRETERSTARTUP_H  //Required for current class
#define __CINTERPRETERSTARTUP_H

//
// Include files:
//

                               //Required for base classes
#ifndef __CEVENTLOOP_H     //CEventLoop
#include "CEventLoop.h"
#endif

class CTCLInterpreter;		// Forward definition of interpreter class.
class CTCLSynchronizeCommand;	// Forward definition of sync command.

/*!
Encapsulates interfaces for starting up 
TCL based interpreter event loops.  The
TCL interpreter executes within a thread.
Adding a command  to the interpreter should
be done by subclassing  CDAQTCLProcessor,
instantiating an object for that class, and registring
it on the current interpreter.  It is important that DAQTCLProcessor
objects be used rather than TCLProcessor objects since  DAQTCLProcessor
is thread-aware and will therefore synchronize its action through the application's
global mutex.

 */
class CInterpreterStartup  : public CEventLoop        
{
private:			// Member data (attributes).
  CTCLInterpreter*         m_pInterp;
  CTCLSynchronizeCommand*  m_pSyncCommand;
private:
	// Private member functions.

  /*!
    This pure virtual member function is expected
    to start the interpreter and call the other member
    functions; it is the entry point of the thread.
    */
  int operator() (int argc, char** argv)   = 0; 

protected:
	// Protected functions:

  virtual   void OnInitialize (int argc, char** Argv);
  virtual   void RegisterExtensions ();

public:
  CInterpreterStartup ();
  virtual ~CInterpreterStartup ( );
  
  //! Copy Constructor is forbidden, private, unimplemented
private:
  CInterpreterStartup (const CInterpreterStartup& aCInterpreterStartup );
  //! Assignment is forbidden, private, unimplemented.
  CInterpreterStartup& operator=
                 (const CInterpreterStartup& aCInterpreterStartup);
  //! Operator== Equality Operator forbidden, private, unimplemented
  int operator== (const CInterpreterStartup& aCInterpreterStartup) const;

public:
  // Selectors:

  //! Return a pointer to the interpreter object.
  CTCLInterpreter* getInterpreter()
  {
    return m_pInterp;
  }
  /*! Get a reference to the interpreter object: Note that this can fail if there is
    not yet an interpreter (m_pInterp is NULL in that case).
    */
  CTCLInterpreter& Interp();

protected:
  // Mutators.

  //! Set the interpreter object:
  void setInterpreter(CTCLInterpreter* pInterp)
  {
    m_pInterp = pInterp;
  }
};

#endif



