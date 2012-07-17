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
#include "CFragmentHandlerCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <tcl.h>
#include <stdint.h>

#include <string>


/**
 * Construct the object:
 * @param interp - reference to an encpasulated interpreter.
 * @param name   - Name to give to the command.
 * @param registerMe - Optional parameter which if true (default) autoregisters the command.
 * 
 */
CFragmentHandlerCommand::CFragmentHandlerCommand(CTCLInterpreter& interp,
						std::string name,
						bool registerMe) :
  CTCLObjectProcessor(interp, name, registerMe)
{
  
}

/**
 * Destructor
 */
CFragmentHandlerCommand::~CFragmentHandlerCommand() {}

/**
 * Command processor
 * - Ensure a channel name is present.
 * - Drain the message body from the channel (stub).
 *
 * @param interp - reference to the encapsulated interpreter.
 * @param objv   - reference to a vetor of encpasulated Tcl_Obj*'s.
 *
 * @return int
 * @retval TCL_OK - success.
 * @retval TCL_ERROR -Failure.
 *
 * @note: TODO:  Deal with running on a big endian system:
 */
int
CFragmentHandlerCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  // objv must have the command name and a socket name:

  if (objv.size() != 2) {
    interp.setResult(std::string("Incorrect number of parameters"));
    return TCL_ERROR;
  }
  // Translate the channel name to a Tcl_Channel: 

  objv[1].Bind(interp);
  std::string channelName = objv[1];

  Tcl_Channel pChannel = Tcl_GetChannel(interp.getInterpreter(), channelName.c_str(), NULL);
  if (pChannel == NULL) {
    interp.setResult(std::string("Tcl does not know about this channel name"));
    return TCL_ERROR;
  }
  // Read the size of the body:

  Tcl_Obj* msgLength = Tcl_NewObj();
  Tcl_Obj* msgBody   = Tcl_NewObj();

  Tcl_IncrRefCount(msgLength);
  Tcl_IncrRefCount(msgBody);


  // Read the message length and get it from the byte array.
  // The protocol requires data in low endian order 
  // The channnel is assumed to be blocking mode so if we don't get the full
  // size it's an error:
  //
  int n = Tcl_ReadChars(pChannel, msgLength, sizeof(uint32_t), 0);
  if (n != sizeof(uint32_t)) {
    interp.setResult(std::string("Message length read failed"));
    Tcl_DecrRefCount(msgLength);
    Tcl_DecrRefCount(msgBody);
    return TCL_ERROR;
  }
  uint32_t* pMsgLength = reinterpret_cast<uint32_t*>(Tcl_GetByteArrayFromObj(msgLength, NULL));

  // A msg length of 0 is fine that means nothing to do otherwise, read the full message from
  // the pipe...again it's an error not to be able to get the full message:

  if (*pMsgLength > 0) {
    n = Tcl_ReadChars(pChannel, msgBody, *pMsgLength, 0);
    if (n != *pMsgLength) {
      Tcl_DecrRefCount(msgLength);
      Tcl_DecrRefCount(msgBody);
      interp.setResult("Message body could not be completely read");
      return TCL_ERROR;
    }
    //  TODO: Dispatch the data to the input queue manager:


  }
  Tcl_DecrRefCount(msgLength);
  Tcl_DecrRefCount(msgBody);

      
  
  return TCL_OK;
}
