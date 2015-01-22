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

static const char* Copyright = "(C) Copyright Michigan State University 2002, All rights reserved";
//////////////////////////CNSCLPhysicsBuffer.cpp file////////////////////////////////////

#include "CNSCLPhysicsBuffer.h"                  
#include "buftypes.h"
#include <CRangeError.h>
#include <unistd.h>

/*!
   Default constructor.  This is called when declarations of the form e.g.:
   -  CNSCLPhysicsBuffer  object;
   are performed.
*/
CNSCLPhysicsBuffer::CNSCLPhysicsBuffer(unsigned nWords)
   : CNSCLOutputBuffer(nWords)
{
  SetType(DATABF);
  getBuffer().SetTag(CNSCLOutputBuffer::m_EventTag);
} 


/*!
    Creates a buffer pointer,  reserves space
    for a word count and returns it to the caller.
    

*/
DAQWordBufferPtr 
CNSCLPhysicsBuffer::StartEvent()  
{
  m_EventStartPtr = StartEntity(); // Start the buffer entity.
  DAQWordBufferPtr p = m_EventStartPtr;
  p += 2;
  return p;
}  

/*!
    Determines the word count from the
    difference between the input pointer and
    m_EventStartPtr.  This word count is
    inserted in the buffer at *m_EventStartPtr
    and m_EventStartPtr is set to be the same as
    the input pointer.  The entity count is incremented.
 
   \param p - DAQWordBufferPtr which points past
       the last word of the event.
    
    \exception - If the DAQWordBufferPtr indicates that
       the event has overflowed the bufer a CRangeError
       will be thrown.

*/
void 
CNSCLPhysicsBuffer::EndEvent(DAQWordBufferPtr& rPtr)  
{
  if(rPtr.GetIndex() >= getWords()) {
    throw CRangeError(0, getWords(), rPtr.GetIndex(),
	     "CNSCLPhysicsBuffer::EndEvent - Off the end of the buffer");
  }
  unsigned nSize = rPtr.GetIndex() - m_EventStartPtr.GetIndex();
  *m_EventStartPtr = nSize & 0xffff; // little endian assumption.
  ++m_EventStartPtr;
  *m_EventStartPtr = (nSize >> 16) & 0xffff;

  EndEntity(rPtr);		// Add to entity count and update ptr.
}  

/*!
    Take the necessary steps to ensure that
    data put in the buffer for this event does not
    get committed to the buffer.  At present this is
    a no-op. Since neiter the base class's buffer pointer
    nor the entity count get modified until EndEvent is called.

	\param DAQWordBufferPtr& p

	\bug There is unfortunately no known way to invalidate
	      m_EventStartPtr else we would at this point.

*/
void 
CNSCLPhysicsBuffer::RetractEvent(DAQWordBufferPtr& p)  
{
 
}
