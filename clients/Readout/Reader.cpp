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

static const char* Copyright= "(C) Copyright Michigan State University 2002, All rights reserved";/*!
  \file Reader.cpp
  Implements the functionality of the Reader class.  Reader objects
  provide the experiment specific part of the readout skeleton.
  See Reader.h for more iformation.
*/

#include "Reader.h"
#include "Trigger.h"
#include "Busy.h"
#include "ReadoutStateMachine.h"
#include <daqinterface.h>
#include <assert.h>
#include <skeleton.h>
#include <iostream.h>
#include <string>
#include <buftypes.h>
#include <CVMEInterface.h>


/*!
   Construct a Reader object.  Note that tyipcally the Reader object is 
   subclassed to select the appropriate type of trigger or busy module.

*/
CReader::CReader(ReadoutStateMachine& rManager) :
  m_rManager(rManager),
  m_pBuffer(0),
  m_nEvents(0),
  m_nWords(0),
  m_nBufferSize(daq_GetBufferSize()),
  m_pTrigger(0),
  m_pBusy(0)
{}

/*!
    Enable data taking.  This involves:
    - Clearing the user's event hardware.
    - Enabling the trigger.
    - Clearing the busy.

    It is assumed that:
    - The computer has busy set.
    - The m_pBusy is non null (assert guarded).
    - The m_pTrigger is non null (assert guarded).
*/
void
CReader::Enable()
{
  // Ensure we're properly assembled:

  assert(m_pTrigger);
  assert(m_pBusy);

  //  Initialize the user's code:

  ::initevt();
  ::clearevt();
  ::clrscl();

  // Enable the trigger:

  m_pTrigger->Initialize();
  m_pTrigger->Enable();

  // Clear the busy

  m_pBusy->Initialize();
  m_pBusy->Clear();


}

/*!
   Turns off data taking by disableing the trigger.  The busy is set as well.

*/
void
CReader::Disable()
{
  m_pBusy->Set();		// Set computer not accepting.
  m_pTrigger->Disable();	// Disable further event receipt.
}

/*!
   Manages the overall flow of checking for triggers and reading events.
   In order to amortize the call overhead and other checks that are performed
   over multiple potential trigger checks, nPasses through the trigger
   check and readout code will be performed.
   
   If within this function, the buffer fills, it will be flushed out to 
   the routing system.

   \param nPasses int [in] - Number of passes through the trigger check loop.
*/
void
CReader::
ReadSomeEvents(unsigned int nPasses)
{

  // The trigger and busy managers must have been installed:

  assert(m_pTrigger);
  assert(m_pBusy);

  // If necessary, allocate a new >empty<  buffer etc.
  //
  if(!m_pBuffer) {
    m_pBuffer     = m_rManager.GetBuffer();
    m_BufferPtr  = m_rManager.GetBody(m_pBuffer);
    m_nEvents     = 0;
    m_nBufferSize = daq_GetBufferSize() - m_BufferPtr.GetIndex();
    m_nWords      = 0;
  }
  
  try {
    for (unsigned int i = 0; i < nPasses; i++) {
      unsigned int nEventSize;
      if(m_pTrigger->Check()) {	// Event fired.
	m_pTrigger->Clear();
	DAQWordBufferPtr hdr = m_BufferPtr;
	m_BufferPtr+= 2;		// Reserve space for event size (long)
	
	CVMEInterface::Lock();
	nEventSize = ::readevt(m_BufferPtr);
	CVMEInterface::Unlock();
	if(nEventSize > 0) {
	  nEventSize += 2;	// 2 words of event size.
	  *hdr       = (nEventSize & 0xffff); // little endian assumption!!!
	  ++hdr;
	  *hdr       = (nEventSize >> 16) & 0xffff;
	  --hdr;		// >sigh< evidently indexing does not work.

	  m_nWords  += nEventSize;   // Fill in the buffer index.
	  m_nEvents++;
	}
	else {			// Rejected (zero length) event.
	  m_BufferPtr = hdr;	// Retract buffer ptr on rejected event.
	}
	//
	// Try to overlap buffer flush management behind inter-event time.

	::clearevt();
	m_pBusy->ModuleClear();
	m_pBusy->Clear();

	// If necessary, flush the buffer:

	if(m_nWords >= m_nBufferSize) {
	  OverFlow(hdr);
	}
      }      	                 // Trigger present.
    }                            // Trigger check loop.
  }
  // The catch blocks below attempt to do our best to put out messages
  // for exceptions thrown by data taking.  The exceptions themselves are
  // propagated back up the call stack, but at least we'll give a message
  // before dying.

  catch(string& rsMessage) {
    cerr << __FILE__ << __LINE__ << 
           "A string exception was caught during readout: \n";
    cerr << rsMessage << endl;
    cerr << "Propagating exception back to caller\n";
    throw;

  }
  catch(char* pszMessage) {
    cerr << __FILE__ << __LINE__ << 
         "A C-string exception was caught during readout:\n";
    cerr << pszMessage << endl;
    cerr << "Propagating exception back to caller\n";
    throw;
  }
  catch(...) {
    cerr << __FILE__ << __LINE__ <<
            "An exception was caught during readout:\n";
    cerr << "Don't know how to convert this to an error message\n";
    cerr << "Propagating exception back to the caller\n";
    throw;
  }
}

/*!
   Flush an event buffer out to the routing system.  This involves:
   - Shrinking the buffer down to 4K words..
   - Filling in the buffer header. 
   - Routing the buffer.
   - Deleting the buffer.
   - Resetting the member variables to indicate that there is no current
     buffer.

  Note that the following are expected to be correct:
  - m_nEvents - Number of events (entities) in the shrunk buffer.
  - m_nWords  - Number of valid words in the shrunken buffer.

  They will be placed into the buffer header.
*/
void
CReader::FlushBuffer()
{
  if(!m_pBuffer) return;	// No buffer to flush.

  // Shrink the buffer down to the daq buffer size:

  m_pBuffer->Resize(daq_GetBufferSize(),true);
  
  // Fill in the buffer heaer:

  m_rManager.NextSequence();	// Increment sequence
  m_rManager.FormatHeader(m_pBuffer, m_nWords, DATABF, m_nEvents);

  // Route and delete the buffer.

  m_pBuffer->Route();
  delete m_pBuffer;

  // Reset the member variables to force a new allocation on the next call
  // to ReadSomeEvents:

  m_pBuffer     = (DAQWordBuffer*)NULL;
  m_nEvents     = 0;
  m_nWords      = 0;
  m_nBufferSize = 0;
}

/*!
   Handles buffer overflows into the safe zone.  All buffers are allocated
   to be twice the size of a daq buffer.  Therefore as long as the worst 
   case event is not larger than a buffer (required by the NSCL daq system), 
   event buffer overflows are allowed and expected.

   When a buffer overflow occurs:
   - A new buffer is allocated.
   - The event that caused the overflow is copied into the new buffer.
   - The old buffer is flushed.
   - Member data are modified so that the new buffer will have new data taken
     into it.
*/
void
CReader::OverFlow(DAQWordBufferPtr& rLastEventPtr)
{
  
  //  Allocate the buffer and point to where the first event belongs:


  DAQWordBuffer* pNewBuffer = m_rManager.GetBuffer();
  DAQWordBufferPtr EventPtr = m_rManager.GetBody(pNewBuffer);
  //
  // Retract the last event from the buffer:

  unsigned int nWords = *rLastEventPtr | (rLastEventPtr[1] << 16); // Little endian assumption.
  
  unsigned int nNewSize = nWords;

  m_nWords -= nWords;
  m_nEvents--;

  // Copy the last event in the old buffer (pointed to by rLastEventPtr)
  // into the new buffer.  The only assumption is that the first word of
  // the event is a self inclusive size.  Note that pre-increments are used
  // because in general they will be faster for objects since they avoid
  // copy construction.
  
  while(nWords) {
    int value = *rLastEventPtr;
    *EventPtr = value;
    ++EventPtr;
    ++rLastEventPtr;
    nWords--;
  }
  // Flush the existing buffer:

  FlushBuffer();

  // Adjust the member data so that the new buffer will be used from now on:

  m_pBuffer     = pNewBuffer;
  m_BufferPtr   = EventPtr;
  m_nEvents     = 1;		// We've just put the first event in the buffer
  m_nWords      = nNewSize;
  m_nBufferSize = daq_GetBufferSize() -
                  m_rManager.GetBody(pNewBuffer).GetIndex();
  
}
