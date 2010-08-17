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

static const char* Copyright = "(C) Copyright Michigan State University 2002, All rights reserved";   
//////////////////////////CInterpreterCore.cpp file////////////////////////////////////
#include <config.h>
#include <tcl.h>
#include <tclDecls.h>
#include "TclAuthorizer.h"
#include "CInterpreterCore.h"                  
#include <TCLInterpreter.h>
#include <CTCLStdioCommander.h>
#include "CReadoutMain.h"
#include "CStateVariable.h"
#include "CConstVariableCommand.h"
#include "CConstVariable.h"
#include "CTCLListener.h"
#include "CTKListener.h"
#include <Exception.h>
#include <CopyrightNotice.h>
#include <NSCLException.h>
#include <Iostream.h>
#include <stdio.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

#include <spectrodaq.h>


/*!
  Construct an interpreter core.
  Saves the interpreter startup object and
  constructs the commands.  The commands are registered
  from the RegisterExtensions entry point.
  \param rStartup - reference to the interpreter startup object.
*/
CInterpreterCore::CInterpreterCore (CInterpreterStartup& rStartup) :
  m_rInterpreter(rStartup),
  m_pBegin(new CBeginCommand),
  m_pEnd(new CEndCommand),
  m_pPause(new CPauseCommand),
  m_pResume(new CResumeCommand),
  m_pRunVariable(new CRunVariableCommand),
  m_pStateVariable(new CStateVariableCommand),
  m_pTagBase(new CTagBaseCommand),
  m_pExit(new CExitCommand),
  m_pAuthorizer(0),
  m_pState(0),
  m_pStartTime(0),
  m_pEvents(0),
  m_pWords(0)
{

}
/*!
   Destructor: Release all dynamically produced members.
   In this case each member is a command object.  It is 
   unregistered and deleted.

   */

CInterpreterCore::~CInterpreterCore()
{
  m_pBegin->UnregisterAll();
  delete m_pBegin;

  m_pEnd->UnregisterAll();
  delete m_pEnd;

  m_pPause->UnregisterAll();
  delete m_pPause;

  m_pResume->UnregisterAll();
  delete m_pResume;

  m_pRunVariable->UnregisterAll();
  delete m_pRunVariable;

  m_pStateVariable->UnregisterAll();
  delete m_pStateVariable;

  m_pTagBase->UnregisterAll();
  delete m_pTagBase;

  m_pExit->UnregisterAll();
  delete m_pExit;

  m_Const.UnregisterAll();


   // Delete the const variables:
   
  delete m_pState;  
  delete m_pState;

  // The existence of an authorizer is optional depending
  // on whether or not the usr selected to enable tcl server
  // functionality.
  //  If it exists it is rundown and deleted.
  if(m_pAuthorizer) {
     m_pAuthorizer->UnregisterAll();
     delete m_pAuthorizer;
  }

}

// Functions for class CInterpreterCore

/*!
    Register the command extensions associated with a Readout
    interpreter.  These extensions support:
    - Run control
    - State variables
    - Run variables

	\param CTCLInterpreter* pInterp

*/
void 
CInterpreterCore::RegisterExtensions()  
{

  // Put out the copyright information:

  CopyrightNotice::Notice(cerr, "pReadout", "1.0", "2002");
  CopyrightNotice::AuthorCredit(cerr, "pReadout", 
				"Ron Fox", NULL);

  // From the Interpreter core, we need the actual interpreter object:

  CTCLInterpreter* pTcl = m_rInterpreter.getInterpreter();

  // We'll also need the main object and the experiment:

  CReadoutMain* pMain = CReadoutMain::getInstance();
  CExperiment*  pExperiment = pMain->getExperiment();
  

  // Now create and register the command executor modules:

  // Begin:

  m_pBegin->Bind(pTcl);
  m_pBegin->Register();

  // End:
  
  m_pEnd->Bind(pTcl);
  m_pEnd->Register();

  // Pause:

  m_pPause->Bind(pTcl);
  m_pPause->Register();

  // Resume:

  m_pResume->Bind(pTcl);
  m_pResume->Register();

  // Run variable manager:

  m_pRunVariable->Bind(pTcl);
  m_pRunVariable->Register();

  // State variable manager:


  m_pStateVariable->Bind(pTcl);
  m_pStateVariable->Register();

  // Tag base management:

  m_pTagBase->Bind(pTcl);
  m_pTagBase->Register();

  // Safe exit:


  m_pExit->Bind(pTcl);
  m_pExit->Register();

  // const command:

  m_Const.Bind(pTcl);
  m_Const.Register();


  // Finally: Register the run state variables and initialize them:

  CStateVariableCommand& rState(*m_pStateVariable);

  rState.Create("title");
  rState.Create("run");
  rState.Create("frequency");
  rState.Create("experiment");

  // The iteration below initializes these variables to reasonable values.
  
  StateVariableIterator i = rState.begin();
  while(i != rState.end()) {
    if(i->first == "run") {
      i->second->Set("0");
    }
    if(i->first == "frequency") {
      i->second->Set("10");
    }
   if(i->first == "title") {
      i->second->Set("Set a new title please");
   }
   if(i->first == "experiment") {
      i->second->Set("Set a new experiment description please");
   }
    i++;
  }
  
  // Execute a little scriptlet to set the correct value of the tkloaded
  // const:
  
  string command("const tkloaded ");
  if(pMain->getWindowed()) {
	command += "true";
	pTcl->Eval("proc __Wake {} { after 10 __Wake }; __Wake");
  }
  else {
	command += "false";
  }
  pTcl->Eval(command.c_str());
  
  // Constants that reflect run state are also defined:
  //   "state" - the run state e.g. "active"
  //   "starttime" - When the most recently started run began.
  //   "events"     - Number of events taken this run.
  //   "words"      - Number of data words taken this run.
  
  m_pState     = new CConstVariable(pTcl, string("state"), string("Inactive"));
  m_Const.Enter(m_pState);

  m_pStartTime = new CConstVariable(pTcl, string("starttime"), 
				   string("-never-"));
  m_Const.Enter(m_pStartTime);
  
  m_pEvents    = new CConstVariable(pTcl, string("events"), string("0"));
  m_Const.Enter(m_pEvents);
  m_nEvents=0; 
  m_pEvents->Link(&m_nEvents, TCL_LINK_INT);
  
  m_pWords    = new CConstVariable(pTcl, string("words"), string("0"));
  m_Const.Enter(m_pWords);
  m_nWords = 0;
  m_pWords->Link(&m_nWords, TCL_LINK_INT);
  



  // If the server is turned on, then start the listener thread:
  

  try {
    if(pMain->getServer()) {
       // Add authorization commands.  Note that if this is
       // done in the listener it races against Tcl startup.
       //
      m_pAuthorizer = new CTclAuthorizer(pTcl->getInterpreter());
      m_pAuthorizer->AddHost(string("localhost"));

      // With Tcl 8.5 we need to put the tclserver in our thread and run an event
      // loop if we are not windowed:
      //

      CTKListener::Server_Init(pTcl->getInterpreter(),
			       pMain->getPort(),
			       m_pAuthorizer);



      if(!pMain->getWindowed()) {
	// Need to create and register an event handler for stdio so that we can
	// run the event loop but stay live to commands:

	new CTCLStdioCommander(pTcl);
      }
    }
  }
  catch (NSCLException& rExcept) {
    cerr << "Caught spectrodaq exception" << rExcept.GetErrorString() << endl;
    cerr << "While: " << rExcept.GetContextString() << endl;
    cerr << "in pServer->Enable() \n";
  }
  catch (CException& rExcept) {
    cerr << "Caught framework exception " << rExcept.ReasonText() << endl;
    cerr << " while: " << rExcept.WasDoing() << endl;
    cerr << " in pServer->Enable() \n";
  }
  catch(string rString) {
    cerr << "Caught string exception " << rString << endl;
    cerr << "in pServer->Enable()\n";
  }
  catch(char* pString) {
    cerr << "Caught char* exception: " << pString << endl;
    cerr << " in pserver->Enable()\n";
  }
  catch(...) {
    cerr << "Unrecognized exception fired in pserver->Enable()\n";
  }
  
  // Now we let the experiment register its experiment specific stuff:


  pMain->AddUserCommands(*pExperiment, m_rInterpreter, *this);
  pMain->SetupStateVariables(*pExperiment, m_rInterpreter, *this);
  pMain->SetupRunVariables(*pExperiment, m_rInterpreter, *this);

}

/*!
   Set a current value for the number of events acquired.
  \param nValue unsigned int [in]  New value for the "events" const.
*/
void
CInterpreterCore::setEvents(unsigned int nValue)
{
  m_nEvents = nValue;
}
/*!
   Set the value of the "words" const.  This const tells you how many words of data
   have been acquired in the current run.
   \param nValue unsigned int [in] New value.
*/
void
CInterpreterCore::setWords(unsigned int nValue)
{
  m_nWords = nValue;
}



