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

/*!
  Tcl interpreter for the readout system.

  Encapsulates the extended interpreter as
  sociated with a readout program in the case where
  the user has selected a Tcl rather than a windowed Tk interpreter.

 */	


// Required headers.

#ifndef __CTCLINTERPRETERSHELL_H  
#define __CTCLINTERPRETERSHELL_H
                               
#ifndef __CTCLINTERPRETERSTARTUP_H
#include "CTCLInterpreterStartup.h"
#endif

#ifndef __CINTERPRETERSHELL_H                               
#include "CInterpreterShell.h"
#endif



// Forward class definitions.

class CInterpreterCore;
class CTclInterpreterShell  : public CTCLInterpreterStartup, 
			      public CInterpreterShell        
{ 
private:
  CInterpreterCore* m_pCore;	//!< Pointer to core object.
  bool              m_bMyCore;	//!< True if I had to allocate a core.
public:
	// Constructors, destructors and other cannonical operations: 

  CTclInterpreterShell ();
  CTclInterpreterShell (CInterpreterCore& rCore);
  
  ~CTclInterpreterShell();
private:
  CTclInterpreterShell(const CTclInterpreterShell& rhs);
  CTclInterpreterShell& operator= (const CTclInterpreterShell& rhs);
  int         operator==(const CTclInterpreterShell& rhs) const; 
  int         operator!=(const CTclInterpreterShell& rhs) const {
    return !(operator==(rhs));
  }
public:

  
  // Class operations:
public:  
  virtual   void RegisterExtensions ()  ;
  virtual   CInterpreterStartup* getInterpreter() {
    return this;

  }
  
protected:
  virtual  CInterpreterCore*  ConstructCore();
};

#endif
