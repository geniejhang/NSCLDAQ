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

/**
 * This file is mostly boilerplate that provides a compiled package for C++
 * for clients of the TclPlus library.
 *
 */

#include <tcl.h>
#include <TCLInterpreter.h>
#include "CFragmentHandlerCommand.h"
#include "CInputStatsCommand.h"


static const char* version = "1.0"; // package version string.

/**
 * Package entry point.  The package is named
 * EvbOrderer that determines the entry point name:
 *
 *
 */
extern "C"
int Eventbuilder_Init(Tcl_Interp* pInterp)
{
  Tcl_PkgProvide(pInterp, "EvbOrderer", version);
  
  // Wrap pInterp in a CTCLInterpretr object and create the command extensions:

  CTCLInterpreter* pInterpObject = new CTCLInterpreter(pInterp);

  new CFragmentHandlerCommand(*pInterpObject, "EVB::handleFragment");
  new CInputStatsCommand(*pInterpObject, "EVB::inputStats");

  return TCL_OK;
}
int gpTCLApplication = 0;
