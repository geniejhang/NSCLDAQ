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
using namespace std;

#include <sstream>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>

#include <options.h>
#include <config.h>
#include <CExperiment.h>
#include <TCLInterpreter.h>
#include <CTimedTrigger.h>

#include "CReadoutAppSRS.h"
#include "CEventSegmentSRS.h"
#include "CTriggerSRS.h"
// #include "CBusySRS.h" // Not implemented yet
#include "CEndCommandSRS.h"
// #include "CScalerSRS.h" // Not implemented yet
#include <CRunControlPackage.h>

#include <vector>

CTriggerSRS *mytrigger(0);             // Newing them here makes order of construction
CEventSegmentSRS *myeventsegment(0);   // un-controlled - now newed in SetupReadout.
// std::vector<CScalerSRS*> scalerModules; // Not implemented yet


////////////////////////////////////////////////////////////////////////////////////////

/*
** Application frameworks require an 'entry point' object instance.  This
** is created below:
*/

CTCLApplication* gpTCLApplication = new CReadoutAppSRS;

////////////////////////////////////////////////////////////////////////////////////////

/*!
  Setup the Readout This function must define the trigger as well as
  the response of the program to triggers.  A trigger is an object that
  describes when an event happens.  Triggers are objects derived from
  CEventTrigger
 
  \note  This function is incompatible with the pre 10.0 software in that
         for the 10.0 software, there was a default trigger that did useful stuff.
	 The default trigger for this version is a NULL trigger (a trigger that
	 never happens.  You _must_ create a trigger object and register it with the
	 experiment object via its EstablishTrigger member funtion else you'll never
	 get any events.

   The following are trigger classes you can use:
   - CNullTrigger - never fires. This is the default.
   - CTimedTrigger - Really intended for scaler triggers, but maybe there's an application
                     you can think of for a periodic event trigger.
   - CTestTrigger  - Always true. That's intended for my use, but you're welcome to use it
                     if you want a really high event rate.
   - CV262Trigger  - Uses the CAEN V262 as a trigger module.
   - CV977Trigger  - Uses the CAEN V977 as a trigger module.

   \param pExperiment - Pointer to the experiment object.

*/

void
CReadoutAppSRS::SetupReadout(CExperiment* pExperiment)
{
  CReadoutMain::SetupReadout(pExperiment);


  int argc;
  char** argv;
  gpTCLApplication->getProgramArguments(argc, argv);
  struct gengetopt_args_info   parsedArgs;
  cmdline_parser(argc, argv, &parsedArgs);

  // std::cout << "CReadoutAppSRS::SetupReadout - Number of arguments (argc): " << argc << std::endl;
  // for (int i = 0; i < argc; ++i) {
  //   std::cout << "CReadoutAppSRS::SetupReadout - Argument " << i + 1 << ": " << argv[i] << std::endl;
  // }

  // Get SRS configuration file and daqPort from .settings source parameters
  // The default daq port is the SRS daq port, give choice of daqPort to be able to bind to a port for testing and send test datagrams
  std::string configFile, daqPortStr = "6006";
  for (int i = 1; i < argc; ++i) {
    std::string arg(argv[i]);
    if (arg == "configFile") {
      // Check if next argument (i + 1) is within bounds and save it in configFile
      if (i + 1 < argc) {
        configFile = argv[i + 1];
        // break;  // Exit the loop after finding the configFile argument
        if (configFile == "daqPort") {
          std::cerr << "Error: 'configFile' found without following file path." << std::endl;
        }
      } else {
        std::cerr << "Error: 'configFile' found without following file path." << std::endl;
      }
    }
    else if (arg == "daqPort") {
      if (i + 1 < argc) {
        daqPortStr = argv[i + 1];
        if (daqPortStr == "configFile") {
          std::cerr << "Error: 'daqPort' found without following port number." << std::endl;
          daqPortStr = "6006";
        }
      } else {
        std::cerr << "Error: 'daqPort' found without following port number." << std::endl;
        daqPortStr = "6006";
      }
    }
  }

  // Dummy trigger
  mytrigger = new CTriggerSRS();
  pExperiment->EstablishTrigger(mytrigger);

  // Add SRS EventSegment
  myeventsegment = new CEventSegmentSRS(mytrigger, *pExperiment);
  myeventsegment->configure(configFile, daqPortStr);
  pExperiment->AddEventSegment(myeventsegment);

}

/*!
  Very likely you will want some scalers read out.  This is done by
  creating scalers and adding them to the CExperiment object's
  list of scalers via a call to that object's AddScalerModule.

  By default, the scalers are read periodically every few seconds.  The interval
  between scaler readouts is defined by the Tcl variable frequency.

  You may replace this default trigger by creating a CEventTrigger derived object
  and passing it to the experiment's setScalerTrigger member function.

  \param pExperiment - Pointer to the experiment object.
*/
void
CReadoutAppSRS::SetupScalers(CExperiment* pExperiment) 
{
  CReadoutMain::SetupScalers(pExperiment);	// Establishes the default scaler trigger.

  // Sample: Set up a timed trigger at 16 second intervals.

  timespec t;
  t.tv_sec  = 2;
  t.tv_nsec = 0;
  
  CTimedTrigger* pTrigger = new CTimedTrigger(t);
  pExperiment->setScalerTrigger(pTrigger);
  
  // // Create and add your scaler modules here.
  // int modules;
  // int crateid;
  // modules = myeventsegment->GetNumberOfModules();
  // crateid = myeventsegment->GetCrateID();

  // cout << "setup scalers for " << modules << " modules " << endl;

  // // if(modules > 20) cout << "how do you fit " << modules << " into one crate for scaler readout " << endl;

  // for( int i=0; i<modules; i++){
    
  //   if(i < 20) {
  //     CMyScaler* pModule = new CMyScaler(i,crateid);
  //     scalerModules.push_back(pModule);
  //     pExperiment->AddScalerModule(pModule);
  //   }
  // }

}
/*!
   Add new Tcl Commands here.  See the CTCLObjectProcessor class.  You can create new
   command by deriving a subclass from this abstract base class.  The base class
   will automatically register itself with the interpreter.  If you have some
   procedural commands you registered with Tcl_CreateCommand or Tcl_CreateObjCommand, 
   you can obtain the raw interpreter (Tcl_Interp*) of a CTCLInterpreter by calling
   its getInterp() member.

   \param pInterp - Pointer to the CTCLInterpreter object that encapsulates the
                    Tcl_Interp* of our main interpreter.

*/

void
CReadoutAppSRS::addCommands(CTCLInterpreter* pInterp)
{
  CReadoutMain::addCommands(pInterp); // Add standard commands.
}

/*!
  Setup run variables:  A run variable is a Tcl variable whose value is periodically
  written to to the output event stream.  Run variables are intended to monitor things
  that can change in the middle of a run.  One use of a run variable is to
  monitor control system values.  A helper process can watch a set of control system
  variables, and issue set commands to the production readout program via its
  Tcl server component.  Those run variables then get logged to the event stream.

  Note that the base class may create run variables so see the comments in the function
  body about where to add code:

  See also:

     SetupStateVariables

     \param pInterp - pointer to the TCL interpreter.
*/

void
CReadoutAppSRS::SetupRunVariables(CTCLInterpreter* pInterp)
{
  // Base class will create the standard commands like begin,end,pause,resume
  // runvar/statevar.

  CReadoutMain::SetupRunVariables(pInterp);

  // Add any run variable definitions below.

}

/*!
  Setup state variables: A state variable is a Tcl variable whose value is logged 
  whenever the run transitions to active.  While the run is not halted,
  state variables are write protected.  State variables are intended to 
  log a property of the run.  Examples of state variables created by the
  production readout framework are run and title which hold the run number,
  and the title.

  Note that the base class may create state variables so see the comments in the function
  body about where to add code:

  See also

  SetupRunVariables

  \param pInterp - Pointer to the tcl interpreter.
 
*/
void
CReadoutAppSRS::SetupStateVariables(CTCLInterpreter* pInterp)
{
  CReadoutMain::SetupStateVariables(pInterp);

  // Add any state variable definitions below:

  
}
