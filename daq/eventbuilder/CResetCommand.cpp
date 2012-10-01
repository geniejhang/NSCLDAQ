/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#include "CResetCommand.h"
#include "CFragmentHandler.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>

/*----------------------------------------------------------------------------------------
** Canonicals:
*/

/**
 * constructor
 *
 * The base class constructor does all the hard work:
 */
CResetCommand::CResetCommand(CTCLInterpreter& interp, std::string cmd) :
  CTCLObjectProcessor(interp, cmd, true)
{}
/**
 * destructor
 *
 * Base class destructor does all the work.
 */

CResetCommand::~CResetCommand() {}

/*-------------------------------------------------------------------------------------
** command processor.  There are no command parameters.
*/

int
CResetCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  requireExactly(objv, 1);


  CFragmentHandler* pHandler = CFragmentHandler::getInstance();
  pHandler->resetTimestamps();


}

