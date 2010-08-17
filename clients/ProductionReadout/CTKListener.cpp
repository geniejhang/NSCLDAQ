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


static const char* Copyright= "(C) Copyright Michigan State University 2002, All rights reserved";/*
** Implementation of Tcl Server connection.
** 
*/
#include <config.h>
#include <tk.h>
#include <malloc.h>
#include <string.h>
#include <Iostream.h>
#include "CTKServerInstance.h"
#include "TclAuthorizer.h"
#include <string>
#ifdef WIN32
#include <winsock.h>
#endif

#include "CTKListener.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif



static CTclAuthorizer* pAuthenticator = 0;

void
CTKListener::Server_Accept(ClientData cd, Tcl_Channel client, 
			   char* pHostname,
			   int nHostport)
{
  Tcl_Interp* pInterp     = (Tcl_Interp*)cd;

  // First be sure the client has any business connecting.

  if(!pAuthenticator) {		// This must be a breakin if no authenticator.
    Tcl_Close(pInterp, client);
    cerr << ">> No authenticator to check connection from "
         << pHostname << " On Port " << nHostport << endl;
    return;
  }
  if(!pAuthenticator->Authenticate(pHostname)) {
    cerr << ">> Rejected unauthorized connection from "
	 << pHostname << " on Port " << nHostport << endl;
    Tcl_Close(pInterp, client);
    return;
  }

  // Honor the connection

  ServerContext context;
  pServerContext pContext = &context;


  printf("Accepting connection from %s on port %d\n", pHostname, nHostport);
  pContext->pInterp = pInterp;
  Tcl_DStringInit(&(pContext->RemoteHost));
  Tcl_DStringAppend(&(pContext->RemoteHost), pHostname, -1);
  pContext->RemotePort = nHostport;
  pContext->DialogChannel = client;
  Tcl_DStringInit(&(pContext->command));

  new CTKServerInstance(context);
  
  Tcl_DStringFree(&(context.RemoteHost));
  Tcl_DStringFree(&(context.command));

}


void 
CTKListener::Server_Init(Tcl_Interp* pInterp, int SERVERPORT,
			 CTclAuthorizer* pAuth) 
{
  // Initialize the tcl/tcp server component:


  pAuthenticator = pAuth;


  // Open the server for business.

  Tcl_OpenTcpServer(pInterp, SERVERPORT, NULL, Server_Accept,
		    pInterp);

}






