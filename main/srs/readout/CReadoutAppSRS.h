/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
             Simon Giraud
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef CREADOUTAPPSRS_H
#define CREADOUTAPPSRS_H

#include <CReadoutMain.h>

class CTCLInterpreter;
class CExperiment;

/*
**  This file is the 'application' class for the production readout software.
**  The application class has several member functions you can override
**  and implement to perform user specific initialization.
**  These are:
**    AddCommands         - Extend the Tcl interpreter with additional commands.
**    SetupRunVariables   - Creates an initial set of run variables.
**    SetupStateVariables - Creates an initial set of state variables.
**    SetupReadout        - Sets up the software's trigger and its response to 
**                          that trigger.
**    SetupScalers        - Sets up the response to the scaler trigger and, if desired,
**                          modifies the scaler trigger from a periodic trigger controlled
**                          by the 'frequency' Tcl variable to something else.
*/

class CReadoutAppSRS : public CReadoutMain
{
private:
  // if you need per instance data add it here:

public:
  // Overrides for the base class members described above.

  virtual void addCommands(CTCLInterpreter* pInterp);
  virtual void SetupRunVariables(CTCLInterpreter* pInterp);
  virtual void SetupStateVariables(CTCLInterpreter* pInterp);
  virtual void SetupReadout(CExperiment* pExperiment);
  virtual void SetupScalers(CExperiment* pExperiment);

private:
  // If you want to define some private member utilities, define them here.

};

#endif
