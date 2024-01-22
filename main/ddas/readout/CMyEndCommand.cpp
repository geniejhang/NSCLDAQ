/**
 * @file CMyEndCommand.cpp
 * @brief Implement the end run command.
 */

#include "CMyEndCommand.h"

#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <tuple>
#include <chrono>
#include <thread>

#include <TCLInterpreter.h>
#include <TCLObject.h>

#include <config.h>
#include <config_pixie16api.h>
#include <CVMEInterface.h>
#include <RunState.h>
#include <CDDASException.h>
#include "CMyEventSegment.h"

CMyEndCommand::CMyEndCommand(
    CTCLInterpreter& rInterp, CMyEventSegment *myevseg, CExperiment* pexp
    ) : CEndCommand(rInterp) 
{
    myeventsegment = myevseg;
    m_pExp = pexp;
    NumModules = myeventsegment->GetNumberOfModules();
}

CMyEndCommand::~CMyEndCommand()
{}

/**
 * @details
 * Stop run in the Director module (module #0) -- a SYNC interrupt should be 
 * generated to stop run in all modules simultaneously when running 
 * synchronously... We are not running synchronously when in INFINITY_CLOCK 
 * mode! If INFINITY_CLOCK mode is not set, then we need to end the run in 
 * every module!
 *
 * @note: If the end run signal is successfully communicated to the module(s), 
 * the transition to an inactive state cannot fail, only report which module(s)
 * failed to properly end their run. One common cause of this failure is a 
 * very high input rate to one or more channels on that module.
 */
int CMyEndCommand::transitionToInactive() 
{
    cout << "Transitioning Pixies to Inactive" << endl;

    if (::getenv("INFINITY_CLOCK") == nullptr) {
	try {
	    int retval = Pixie16EndRun(0);	
	    if (retval < 0) {
		std::string msg = "Failed to communicate end run operation to"
		    " the director module.";
		throw CDDASException(retval, "Pixie16EndRun", msg);
	    }
	}
	catch (const CDDASException& e) {
	    std::cerr << e.ReasonText() << std::endl;
	    return TCL_ERROR;
	}
    } else {
	for (int i = 0; i < NumModules; ++i) {
	    try {
		int retval = Pixie16EndRun(0);	
		if (retval < 0) {
		    std::string msg = "Failed to communicate end run"
			" operation in module " + i;
		    throw CDDASException(retval, "Pixie16EndRun", msg);
		}
	    }
	    catch (const CDDASException& e) {
		std::cerr << e.ReasonText() << std::endl;
		return TCL_ERROR;
	    }
	}
    }

    for (int i = 0; i < NumModules; ++i) {
	bool runEnded = false;
	int nRetries = 0;
	const int nMaxRetries = 10;
	while (!runEnded && nRetries < nMaxRetries) {
	    int retval = -1;
	    try {
		retval = Pixie16CheckRunStatus(i);
		if (retval < 0) {
		    std::string msg = "Failed to check run status"
			" trying again...";
		    throw CDDASException(retval, "Pixie16CheckRunStatus", msg);
		}
	    }
	    catch (const CDDASException& e) {
		std::cerr << e.ReasonText() << std::endl;
	    }    
	    runEnded = (retval == 0);
	    nRetries ++;
	    this_thread::sleep_for(chrono::milliseconds(100));
	}
	if (nRetries == nMaxRetries) {
	    std::cout << "Failed to end run in module " << i << std::endl;
	}
    }

    return 0;
}

/**
 * @details
 * After reading out the last of the data, write the run statistics to an
 * end-of-run scalers file.
 */
int CMyEndCommand::readOutRemainingData() 
{
    // We will poll trying to lock the mutex so that we have a better chance
    // of acquiring it.
    const int nAllowedAttempts = 10;
    int nAttemptsMade = 0;
    while (!CVMEInterface::TryLock(1) && (nAttemptsMade < nAllowedAttempts)) { 
	nAttemptsMade++; 
    }

    if (nAttemptsMade == nAllowedAttempts) {
	// Failed to lock the interface, add an end event back onto the tail of
	// the event stack. We will try again. This is to prevent deadlocks
	// between the CVariableBuffers thread sync and the end run sync.
	rescheduleEndRead();
	return TCL_ERROR;
    }

    int retval = 0;
    int count = 0;
    char filnam[80];
    unsigned long nFIFOWords[24];
    unsigned long *lmdata;
    unsigned int mod_numwordsread;
    unsigned short EndOfRunRead;

    if ((lmdata = (unsigned long *)malloc(sizeof(unsigned long)*131072)) == nullptr) {
        cout << "Failed to allocate memory block lmdata" << endl << flush;
    }

    // Clear counters to 0 (counters keep track of how many words each 
    // module has read)
    for (int i = 0; i < NumModules; i++) {
        nFIFOWords[i] = 0;
    } 

    usleep(100);
    
    // Make sure all modules indeed finish their run successfully.
    for(int i = 0; i < NumModules; i++) {
        // For each module, check to see if a run is still in progress in the
	// ith module. If it is still running, wait a little bit and check
	// again. If after 10 attempts, we stop trying. Run ending failed.
        count = 0;
        do {
	    try {
		retval = Pixie16CheckRunStatus(i);
            
		// retval < 0: Error checking run status.
		// retval == 1: A run is in progress.
		// retval == 0: Run is ended.
		if (retval < 0) {
		    std::string msg = "Failed check run status in module " + i;
		    throw CDDASException(retval, "Pixie16CheckRunStatus", msg);
		}
		else if (retval == 1) {
		    EndOfRunRead = 1;	
		} else {
		    // No run is in progress
		    break;
		}
	    }
	    catch (const CDDASException& e) {
		EndOfRunRead = 1; // We failed, so...
		std::cerr << e.ReasonText() << std::endl;
	    }
            count ++;
            usleep(100);
        } while (count < 10);

        if (count == 10) {
            cout << "End run in module " << i << " failed" << endl << flush;
        }
    }

    // All modules have their run stopped successfully. Now read out the 
    // possible last words from the external FIFO, and get statistics.
    ofstream outputfile;
    outputfile.open("EndofRunScalers.txt", ios::app);

    for(int i = 0; i < NumModules; i++) {
        EndOfRunRead = 1;
	
        // Get final statistics:
	std::vector<unsigned int> statistics(Pixie16GetStatisticsSize(), 0);
	try {
	    int retval = Pixie16ReadStatisticsFromModule(statistics.data(), i);
	    if (retval < 0) {
		std::string msg = "Error accessing scaler statistics"
		    " from module " + i;
		throw CDDASException(
		    retval, "Pixie16ReadStatisticsFromModule", msg
		    );
	    }
	}
	catch (const CDDASException& e) {
	    std::cerr << e.ReasonText() << std::endl;
	}

	// Write the stats to the output file:
        outputfile << "Module " << i << endl;	
        for (int j = 0; j < 16; j++) {
	    double ocr = Pixie16ComputeOutputCountRate(
		statistics.data(), i, j
		);
	    double icr = Pixie16ComputeInputCountRate(
		statistics.data(), i, j
		);
            outputfile << "   Channel " << i << ": "
		       << ocr << " "  << icr << endl;
        }
    }

    outputfile.close();  
    free(lmdata);
    CVMEInterface::Unlock();
    
    return 0;	
}

int CMyEndCommand::endRun() 
{
    RunState* pState = RunState::getInstance();

    // To end a run we must have no mor command parameters
    // and the state must be either active or paused:

    bool okToEnd = (
	(pState->m_state == RunState::active)
	|| (pState->m_state == RunState::paused)
	);

    // In case any of these end run stages fail, they will be rescheduled
    // and picked up again
    if(okToEnd) { 
	// we will poll trying to lock the mutex so that we have a better
	// chance of acquiring it.
	const int nAllowedAttempts = 10;
	int nAttemptsMade = 0;
	while (
	    !CVMEInterface::TryLock(1) && (nAttemptsMade < nAllowedAttempts)
	    ) { 
	    nAttemptsMade++; 
	}

	if (nAttemptsMade == nAllowedAttempts) {
	    // Failed to lock the interface, add an event to the tail of the
	    // event stack to try again later. This is to prevent deadlocks
	    // between the CVariableBuffers thread sync and the end run sync.
	    rescheduleEndTransition();
	} else {
	    // We've acquired the lock, proceed:
	    int deviceEndStatus = transitionToInactive();
	    
	    CVMEInterface::Unlock();
	    
	    int triggerEndStatus;
	    string result;
	    tie(triggerEndStatus, result) = CEndCommand::end();
	    
	    if (deviceEndStatus != TCL_OK || triggerEndStatus != TCL_OK) {
		return TCL_ERROR;
	    }
	}
    }

    return TCL_OK;
}

void CMyEndCommand::rescheduleEndTransition() {
    EndEvent* pEnd = reinterpret_cast<EndEvent*>(Tcl_Alloc(sizeof(EndEvent)));
    pEnd->s_rawEvent.proc = CMyEndCommand::handleEndRun;
    pEnd->s_thisPtr = this;
    Tcl_QueueEvent(reinterpret_cast<Tcl_Event*>(pEnd), TCL_QUEUE_TAIL);
}

void CMyEndCommand::rescheduleEndRead() {
    EndEvent* pEnd = reinterpret_cast<EndEvent*>(Tcl_Alloc(sizeof(EndEvent)));
    pEnd->s_rawEvent.proc = CMyEndCommand::handleReadOutRemainingData;
    pEnd->s_thisPtr = this;
    Tcl_QueueEvent(reinterpret_cast<Tcl_Event*>(pEnd), TCL_QUEUE_TAIL);
}

/**
 * @details
 * Overridden CEndCommand function call operator to be sure that our end run 
 * gets called at the right time. If an end run operation is permitted, attempt
 * to read out the remaining data and end the run. 
 */
int
CMyEndCommand::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
    )
{
    if (objv.size() != 1) {
	return TCL_ERROR;
    }
    int status = endRun();
    if (status == TCL_OK) {
	readOutRemainingData();
    }
    return TCL_OK;
}

/**
 * @details
 * Calls the command's endRun function.
 */
int CMyEndCommand::handleEndRun(Tcl_Event* pEvt, int flags)
{
    EndEvent* pEnd = reinterpret_cast<EndEvent*>(pEvt);
    pEnd->s_thisPtr->endRun();
    
    return 0;
}

/**
 * @details
 * Calls the command's readOutRemainingData function.
 */
int CMyEndCommand::handleReadOutRemainingData(Tcl_Event* pEvt, int flags)
{
    EndEvent* pEnd = reinterpret_cast<EndEvent*>(pEvt);
    pEnd->s_thisPtr->readOutRemainingData();
    
    return 0;
}
