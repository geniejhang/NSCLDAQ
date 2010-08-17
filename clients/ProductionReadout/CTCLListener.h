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

   
//////////////////////////CTCLListener.h file//////////////////////////////////

#ifndef __CTCLLISTENER_H  
#define __CTCLLISTENER_H

#ifndef __CSERVERCONNECTIONEVENT_H
#include <CServerConnectionEvent.h>
#endif


// Forward definitions:

class CTclAuthorizer;
                           
/*!
   Listens for connections for the TclServer component of the
   readout software.  When connections are requested,
   creates and starts a new CTCLServer object that processes
   the tcl commands poked into it socket.
   
   
 */		
class CTCLListener : public CServerConnectionEvent     
{ 
private:
  CTclAuthorizer* m_pAuthorizer;

public:
	// Constructors, destructors and other cannonical operations: 

    CTCLListener (int nPort, CTclAuthorizer* pAuth); //!< Construct using port value.
    virtual  ~ CTCLListener ( ) {
    } 						 //!< Destructor.

	// Some cannonical functions are not allowed:
private:
    CTCLListener(const CTCLListener& rhs); //!< Copy constructor.
    CTCLListener& operator= (const CTCLListener& rhs); //!< Assignment
    int         operator==(const CTCLListener& rhs) const; //!< Comparison for equality.
    int         operator!=(const CTCLListener& rhs) const;
public:
	// Selectors for class attributes:
public:

	// Mutators:
protected:  

	// Class operations:

   virtual   void OnConnection (CSocket* pPeer)  ;
   virtual   bool Authenticate (CSocket* pPeer)  ;
 
};

#endif
