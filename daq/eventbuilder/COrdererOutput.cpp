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
#include "COrdererOutput.h"
#include <TCLInterpreter.h>
#include <string>
#include "fragment.h"

/*------------------------------------------------------------------------
**  Canonical methods
*/

/**
 * Constructor:
 *    Translates the channel name into a Tcl_Channel.  If that fails throws
 *    an exception string.
 *    Gets an instance of the Fragment handler and registers ourselves as an 
 *    observer so that we'll get fragments.
 *    we also ensure the -encoding is binary and the -translation binary
 *
 * @param interp       - The interpreter used to lookup the channel.
 * @param pChannelName - The channel namestring (e.g. 'stdout').
 */
COrdererOutput::COrdererOutput(CTCLInterpreter& interp, const char* pChannelName)
{
  int mode;
  m_OutputChannel = Tcl_GetChannel(interp.getInterpreter(), pChannelName, &mode);

  /*
    Several things can go wrong:
    - Lookup failed.
    - Mode does not allow write.
  */
  if (!m_OutputChannel) {
    throw std::string("COrdererOutput observer output channel lookup failed");
  }
  if (!(mode & TCL_WRITABLE)) {
    throw std::string("COrdererOutput observrer output channel is not writable");
  }
  // Set the channel options:

  int stat = Tcl_SetChannelOption(interp.getInterpreter(), m_OutputChannel,
				  "-encoding",    "binary");
  if (stat != TCL_OK) {
    throw std::string("COrdererOutput could not set output encoding binary");
  }
  stat =     Tcl_SetChannelOption(interp.getInterpreter(), m_OutputChannel,
				  "-translation", "binary");
  if (stat != TCL_OK) {
    throw std::string("COrdererOutput could not set output translation to binary");
  }

  // Regiseter us as an observer:

  CFragmentHandler* pHandler = CFragmentHandler::getInstance();
  pHandler->addObserver(this);

}
/**
 * Destructor:
 *    Since this will be invalid, we need to remove ourselves from the listener list:
 */
COrdererOutput::~COrdererOutput()
{
  CFragmentHandler* pHandler = CFragmentHandler::getInstance();
  pHandler->removeObserver(this);
}
/*------------------------------------------------------------------------
** Public methods.
*/

/**
 * operator()
 *    Receives a vector of events from the fragment handler.
 *    These events are time ordered, unless there are time order
 *    errors.  The events are output the Tcl_Channel m_OutputChannel.
 *
 * @param event - vector of fragments to output.
 */
void
COrdererOutput::operator()(const std::vector<EVB::pFragment>& event)
{
  for (int i = 0; i < event.size(); i++) {
    EVB::pFragment p = event[i];
    
    int bytesWritten = Tcl_WriteChars(m_OutputChannel, 
				      reinterpret_cast<const char*>(&(p->s_header)), sizeof(EVB::FragmentHeader));
    if (bytesWritten < 0) {
      ThrowErrnoString("COrdererOutput failed to write fragment header to output:\n");
      
    }
    bytesWritten = Tcl_WriteChars(m_OutputChannel, 
				  reinterpret_cast<const char*>(p->s_pBody), p->s_header.s_size);
    if (bytesWritten < 0) {
      ThrowErrnoString("COrdererOuptut failed to write fragment body to output:\n");
    }
  }
  // Flush the output -- the event.size() check prevents a flush if we were
  // given an empty fragment array.

  if(event.size() && (Tcl_Flush(m_OutputChannel) != TCL_OK)) {
    ThrowErrnoString("COrdererOutput failed to flush channel after writing fragments.");
  }
}

/*-----------------------------------------------------------------------------
** Utiltity methods
*/

/**
 * Throw an error string given an error with errno:
 *.
 * @param prefixMessage - prefixes the error message.
 */
void
COrdererOutput::ThrowErrnoString(const char* prefixMessage) const
{
  int error = Tcl_GetErrno();
  std::string msg(prefixMessage);
  msg += Tcl_ErrnoMsg(error);
  
  throw msg;
}
