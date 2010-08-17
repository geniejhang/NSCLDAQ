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


#ifndef __CTKLISTENER_H
#define __CTKLISTENER_H

#include <tk.h>

#ifndef SERVERCONTEXT
#define SERVERCONTEXT
typedef struct _ServerContext {
  Tcl_Interp*   pInterp;
  Tcl_DString   RemoteHost;
  int           RemotePort;
  Tcl_Channel   DialogChannel;
  Tcl_DString   command;
} ServerContext, *pServerContext;
#endif

class CTclAuthorizer;

/*!
   Provides an event driven listener suitable for use in the Tk environment.
*/
class CTKListener 
{
  
  struct StartupInfo {
    Tcl_Interp* pInterp;
    int         port;
 };
 private:
  static void Server_Accept(ClientData cd,
			    Tcl_Channel client,
			    char* pHostname, int nHostport);

 public:
  static void Server_Init(Tcl_Interp* pInterp, 
			  int nPort,
			  CTclAuthorizer* pAuth);
};

#endif
