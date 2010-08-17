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

static const char* Copyright = "(C) Copyright Michigan State University 2002, All rights reserved";//////////////////////////CTCLListener.cpp file////////////////////////////////////
#include <config.h>
#include "CTCLListener.h"                 
#include "CTCLServer.h"
#include "TclAuthorizer.h"
#include "CReadoutMain.h"
#include <CInterpreterStartup.h>
#include "CInterpreterShell.h"
#include "CInterpreterCore.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

#include <string>
#include <stdio.h>

// Simple stupid function to turn an integer into a string.  This is required for the
// constructor of CServerConnectionEveent... our base class.

inline
string itoa(int n)
{
	char buf[1000];
	sprintf(buf, "%d", n);
	return string(buf);
}

	//Default constructor alternative to compiler provided default constructor
	//Association object data member pointers initialized to null association object 
/*!
   Default constructor.  This is called when declarations of the form e.g.:
   -  CTCLListener  object;
   are performed.
*/
CTCLListener::CTCLListener (int nPort, CTclAuthorizer* pAuth) :
	CServerConnectionEvent(string("TCLListener"),
			       itoa(nPort)),
	m_pAuthorizer(pAuth)
 
{
  CReadoutMain*        pMain    = CReadoutMain::getInstance();
  CInterpreterShell*   pStartup = pMain->getInterpreter();
  CInterpreterCore*    pCore    = pStartup->getInterpreterCore();
  CInterpreterStartup* pIStartup = pCore->getStartup();
  CTCLInterpreter*     pInterp  = pIStartup->getInterpreter();


} 

// Functions for class CTCLListener

/*!
    Processes connection requests:
    - Authenticate the connection to determine
      that it is acceptable.
    - If the connection authenticates, then create a CTCLServer 
       to process it.

	\param CSocket* pPeer

*/
void 
CTCLListener::OnConnection(CSocket* pPeer)  
{
	if(!Authenticate(pPeer)) {   // Unauthorized connection.
		pPeer->Shutdown();
		delete pPeer;
	} else {                            // Authorized connection.
		CTCLServer* pServer = new CTCLServer(pPeer);
		pServer->Enable();    // Start the server.
	}
}  

/*!
    Return True if the supplied socket is authorized
    to connect to this server.

	\param CSocket* pPeer

*/
bool 
CTCLListener::Authenticate(CSocket* pPeer)  
{
  // authenticate against the host list maintained by the
  // serverauth commann in m_pAuthorizer:
  //
  unsigned short port;
  string peer;
  pPeer->getPeer(port, peer);

  return m_pAuthorizer->Authenticate(peer);
  
}
