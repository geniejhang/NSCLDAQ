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

static const char* versionString = "V2.0";

#include <config.h>
#include "CTheApplication.h"
#include "Globals.h"

#include <COutputThread.h>
#include <CCCUSB.h>

#include <TCLInterpreter.h>
#include <TCLList.h>
#include <TCLException.h>
#include <TCLObject.h>

#include <CBeginRun.h>
#include <CEndRun.h>
#include <CPauseRun.h>
#include <CResumeRun.h>
#include <Exception.h>
#include <DataBuffer.h>
#include <TclServer.h>
#include <buftypes.h>
#include <buffer.h>

#include <vector>

#include <usb.h>
#include <sysexits.h>
#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#ifndef NULL
#define NULL ((void*)0)
#endif

using namespace std;

//  Static member vars:

Tcl_ThreadId      CTheApplication::m_MainThread;
CTheApplication*  CTheApplication::m_pTheApplication(0);
CTCLInterpreter*  CTheApplication::m_pInterpreter;
//   Configuration constants:

static const int    tclServerPort(27000);
static const string daqConfigBasename("daqconfig.tcl");
static const string ctlConfigBasename("controlconfig.tcl");
static const uint32_t bufferCount(32); // Number of buffers that can be inflight.
static const uint32_t bufferSize(4*1024*sizeof(uint16_t)); // 4kword buffers...+pad


// Static member variables and initialization.

bool CTheApplication::m_Exists(false);

/*!
   Construct ourselves.. Note that if m_Exists is true,
   we BUGCHECK.
*/
CTheApplication::CTheApplication()
{
  if (m_Exists) {
    cerr << "Attempted to create more than one instance of the application\n";
    exit(EX_SOFTWARE);
  }
  m_Exists = true;
  m_pInterpreter = static_cast<CTCLInterpreter*>(NULL);
  m_pTheApplication = this;
}
/*!
   Destruction is a no-op since it happens at program exit.
*/
CTheApplication::~CTheApplication()
{
}

/*!
   Thread entry point.  We don't care that much about our command line parameters.
   Note that the configuration files are as follows:
   $HOME/config/daqconfig.tcl     - Data acquisition configuration.
   $HOME/config/controlconfig.tcl - Controllable electronics configuration.
   We will not be returned to after calling startInterpreter().
   startInterpreter will eventually return control to the Tcl event loop which
   will exit directly rather than returning up the call chain.

   \param argc : int
      Number of command line parameters (ignored).
   \param argv : char**
      The command line parameters (ignored).

*/
int CTheApplication::operator()(int argc, char** argv)
{
  m_Argc   = argc;		// In case someone else wants them.
  m_Argv   = argv; 


  cerr << "CC-USB scriptable readout version " << versionString << endl;

  try {				// Last chance exception catching...
    
    createUsbController();
    setConfigFiles();
    initializeBufferPool();
    startOutputThread();
    startTclServer();
    startInterpreter();
  }
  catch (string msg) {
    cerr << "CTheApplication caught a string exception: " << msg << endl;
    throw;
  }
  catch (const char* msg) {
    cerr << "CTheApplication caught a char* excpetion " << msg << endl;
    throw;
  }
  catch (CException& error) {
    cerr << "CTheApplication caught an NCLDAQ exception: " 
	 << error.ReasonText() << " while " << error.WasDoing() << endl;
    throw;
  }
  catch (...) {
    cerr << "CTheApplication thread caught an excpetion of unknown type\n";
    throw;
  }
    return EX_SOFTWARE; // keep compiler happy, startInterpreter should not return.
}


/*
   Start the output thread.  This thread is responsible for 
   reformatting and transferring buffers of data from the CC-USB to 
   spectrodaq.  This thread is continuously running for the life of the program.
   .. therefore we are sloppy with storage management.
*/
void
CTheApplication::startOutputThread()
{
  COutputThread* router = new COutputThread;
  daq_dispatcher.Dispatch(*router);

}

/*
    Start the Tcl interpreter, we use the static AppInit as a trampoline into the
    interpreter configuration and back to the interpreter event loop so the
    default Tcl event loop can be used.
*/
void
CTheApplication::startInterpreter()
{
  Tcl_Main(m_Argc, m_Argv, CTheApplication::AppInit);
}

/*
   Create the USB controller.  The usb controller will be the first
   one available (should be the only one).  It is a failable error for
   there not to be any controllers.
*/
void
CTheApplication::createUsbController()
{
  vector<struct usb_device*> controllers = CCCUSB::enumerate();
  if (controllers.size() == 0) {
    cerr << "There appear to be no CC-USB controllers so I can't run\n";
    exit(EX_CONFIG);
  }
  Globals::pUSBController = new CCCUSB(controllers[0]);

}
/* 
  Set the configuration files to the global storage
*/
void
CTheApplication::setConfigFiles()
{
  Globals::configurationFilename = makeConfigFile(daqConfigBasename);
  Globals::controlConfigFilename = makeConfigFile(ctlConfigBasename);

}



/*
   Initialize the interpreter.  This invoves:
   - Wrapping the interpreter into a CTCLInterpreter Object.
   - Creating the commands that extend the interpreter.
   - Returning TCL_OK so that the interpreter will start running the main loop.

*/
int
CTheApplication::AppInit(Tcl_Interp* interp)
{
  Tcl_Init(interp);
  CTCLInterpreter* pInterp = new CTCLInterpreter(interp);

  m_MainThread = Tcl_GetCurrentThread(); // Interpreter's thread...

  new CBeginRun(*pInterp);
  new CEndRun(*pInterp);
  new CPauseRun(*pInterp);
  new CResumeRun(*pInterp);
  m_pInterpreter = pInterp;


  // Look for readoutRC.tcl in the config directory.  If it exists, run it.

  string initScript = makeConfigFile(string("readoutRC.tcl"));
  try {
    if (access(initScript.c_str(), R_OK) == 0) {
      pInterp->EvalFile(initScript.c_str());
    }
  }
  catch (CTCLException except) {
    cerr << "Failed to run initialization file.\n";
    cerr << except.ReasonText() << endl;
  }

  return TCL_OK;
}

/*
   Make a configuration filename:  This is done by taking a basename
   and prepending the home directory and config subdir to its path:

*/
string
CTheApplication::makeConfigFile(string baseName)
{
  string home(getenv("HOME"));
  string pathsep("/");
  string config("config");
  string dir;
  
  // The user can define a CONFIGDIR env variable to move the configuration dir.

  if (getenv("CONFIGDIR")) {
    dir =getenv("CONFIGDIR");
  } else {
    dir = home + pathsep + config;
  }


  string result = dir +  pathsep + baseName;
  return result;

}

// Create the application instance so that Spectrodaq can get us going.k

static CTheApplication theApp;


void* gpTCLApplication(0);

int main(int argc, char** argv, char** env)
{
  return spectrodaq_main(argc, argv, env);
}
/*
   Create the buffer pool.  The following are configurable parameters at the
   top of this file;
   - bufferCount  - Number of buffers to create.
   - bufferSize   - Size (in bytes) of the buffer (payload).

*/
void
CTheApplication::initializeBufferPool()
{
  for(uint i =0; i < bufferCount; i++) {
    DataBuffer* p = createDataBuffer(bufferSize);
    gFreeBuffers.queue(p);
  }
}
/* 
   Start the Tcl server.  It will listen on port tclServerPort, seee above..
   Again, the tcl server runs the lifetime of the program so we are 
   sloppy about storage management.
*/
void
CTheApplication::startTclServer()
{
  TclServer* pServer = new TclServer;
  pServer->start(tclServerPort, Globals::controlConfigFilename.c_str(),
		   *Globals::pUSBController);
}
/*!
 *  Return the thread of the main interpreter.
 *  Can be used in e.g. Tcl_ThreadQueueEvent.
 */
Tcl_ThreadId
CTheApplication::mainThread()
{
  return m_MainThread;
}


/*!
   Handle events with data from the 'router' thread.
   @param pEvent -Tcl Event of type dataEvent.
   @param flags - event processing flags
   @return int
   @retval 1


*/
int
CTheApplication::dataEventHandler(Tcl_Event* pEvent, int flags)
{
  // Cast to the right event type.
  // figure out the size/type output them,
  // free the payload and return 1.
  //

  dataEvent* pMyEvent = reinterpret_cast<dataEvent*>(pEvent);
  uint16_t*  pPayload = reinterpret_cast<uint16_t*>(pMyEvent->m_pPayload);


  CTheApplication* pApp = CTheApplication::getApplication();
  pApp->onVMUSBData(pPayload[1], pPayload);

  Tcl_Free(reinterpret_cast<char*>(pPayload));

  return 1;
}
/*!
 @return CTheApplication*
 @retval Pointer to the singleton that is the application.
 @retval 0 the application has not yet been constructed.
*/
CTheApplication*
CTheApplication::getApplication()
{
  return m_pTheApplication;
}
/*!
   Object context handling of data from the VM_USB
   @param type    - Data type: 1 data, 11 begin, 12 end.
   @param pBuffer - Pointer to the raw buffer.
*/
void
CTheApplication::onVMUSBData(uint16_t type, void* pBuffer)
{
  bheader* pHeader = reinterpret_cast<bheader*>(pBuffer);
  switch(type) {
  case BEGRUNBF:
    {
      struct ctlbody* pBody = reinterpret_cast<struct ctlbody*>(&(pHeader[1]));
      onBegin(pHeader->run, pBody);
    }
    break;
  case ENDRUNBF: 
    {
      struct ctlbody* pBody = reinterpret_cast<struct ctlbody*>(&(pHeader[1]));
      onEnd(pHeader->run, pBody);
    }
  
    break;
  case DATABF:
    {
      struct phydata* pBody = reinterpret_cast<struct phydata*>(&(pHeader[1]));
      onPhysics(pHeader->nevt, pBody);
    }
    break;
  default:
    break;			// Just ignore all other buffers.
  }
}
/*
 * Common code processing of a control buffer.
 * the only differnce is which proc gets invoked.
 */
void
CTheApplication::dispatchControlBuffer(const char* baseCommand,
				       uint16_t run, 
				       ctlbody* pBody)
{
  char title[81];
  title[80]   = '\0';
  string time = todToTimeString(pBody->tod);
  strncpy(title, pBody->title, 80);
  string titleString(title);
  char runstring[100];
  sprintf(runstring, "%d", run);

  // Construct the command string and eval it in a try/catch-ignore block so that 
  // we dont' have to worry about what happens if onBegin does not exist:
  
  string command = baseCommand;
  command       += ' ';
  command       += runstring;
  command       += " {";
  command       += titleString;
  command       += "} {";
  command       += time;
  command       += "}";

  try {
    m_pInterpreter->GlobalEval(command);
  }
  catch (...) {
  }

}
/*!
 * Process a begin run buffer.  
 * @param run   - Run number of the run.
 * @param pBody - Body of the begin run buffer.
 */
void
CTheApplication::onBegin(uint16_t run, ctlbody* pBody)
{
  dispatchControlBuffer("onBegin",
			run, pBody);

}
/*!
 Process an end of run buffer.
 @param run    - Number of the run that's ending.
 @param pBody  - Body of the end run buffer.
*/
void
CTheApplication::onEnd(uint16_t run, ctlbody* pBody)
{
  dispatchControlBuffer("onEnd",
			run, pBody);


}
/*!
  Process a data buffer.
  @param  count   - Number of events in the buffer.
  @param  pEvents - POinter to the buffer body.
*/
void
CTheApplication::onPhysics(uint16_t count, phydata* pEvents)
{

  uint16_t* pBuffer;		// Will step through the buffer.
  uint32_t* pScalers;		// Will step through scalers for each event.

  pBuffer = reinterpret_cast<uint16_t*>(pEvents);
  vector<string>  eventList;
  for (int i =0; i < count; i++) {
    uint16_t nWords = *pBuffer++;
    int nScalers    = nWords/2;	// Assuming 2 uint16_t's per uint32_t.
    pScalers = reinterpret_cast<uint32_t*>(pBuffer);
    vector<string> scalerList;
    CTCLList       scalerTclList(m_pInterpreter);
    for(int s = 0; s < nScalers; s++) {
      char scalerString[100];
      int   scaler = *pScalers++;
      sprintf(scalerString, "%d", scaler & 0xffffff);
      scalerList.push_back(scalerString);
    }
    // Make the Tcl List for this event.. get it as a string and
    // push that string into eventList.

    scalerTclList.Merge(scalerList);
    eventList.push_back(scalerTclList.getList());

    pBuffer += nWords;
    
  }
  // Turn the eventList into a TclList:

  CTCLList eventTclList(m_pInterpreter);
  eventTclList.Merge(eventList);
  
  // Construct the command::

  string command = "onEvent ";
  command       += "{";
  command       += eventTclList.getList();
  command       += "}";

  try {
    m_pInterpreter->GlobalEval(command);
  }
  catch(...) {
  }
}

/*!
  Return the stringified equivalent of a tod stuct.
  @param Tod  - the tod struct.
  @return string
*/
string
CTheApplication::todToTimeString(bftime& tod)
{
  char time[200];
  sprintf(time, "%d/%d/%d %d:%d:%d",
	  tod.month+1, tod.day, tod.year+1900,
	  tod.hours, tod.min, tod.sec);
  return string(time);
}
