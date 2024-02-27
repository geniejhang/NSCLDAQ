/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2013.

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
 * @file CSyncCommand.cpp
 * @brief Implement the ddas_sync command.
 */

#include "CSyncCommand.h"

#include <stdexcept>

#include <TCLInterpreter.h>
#include <TCLObject.h>

#include "CMyEventSegment.h"
#include <CDDASException.h>

/**
 * @details
 * Base class registers the command. We need to save the event segment pointer.
 */
CSyncCommand::CSyncCommand(CTCLInterpreter& interp, CMyEventSegment* pSeg) :
    CTCLObjectProcessor(interp, "ddas_sync", true), m_pSegment(pSeg)
{}

/**
 * @details
 * Chain to superclass for now.
 */
CSyncCommand::~CSyncCommand() {}

int
CSyncCommand::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
    )
{
    // Exceptions map to TCL_ERROR returns with a string that describes the
    // exception:    
    try {
        requireExactly(
            objv, 1, "ddas_sync command takes no parameters"
        ); // can throw std::string
        
        m_pSegment->synchronize(); // can throw CDDASException
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (CDDASException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    
    return TCL_OK;
}
