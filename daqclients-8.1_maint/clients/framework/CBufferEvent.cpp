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


/*!
   \file  CBufferEvent.cpp 
   \class CBufferEvent  abstract template
           Provides an ABC for building
           application level objects to react to
           SpectroDaq Buffers.  This is an abstract,
           templated class which is templated by the
           type of buffer whch can be received.
           Note that depending on how this is constructed,,
           The object can handle alarm events instead
           of data buffers.
           
*/

////////////////////////// FILE_NAME.cpp /////////////////////////////////////////////////////

#include <config.h>

#include "CBufferEvent.h"    				
#include <stdio.h>

#ifdef _HAVE_STD_NAMESPACE
using namespace std;
#endif

// Implementation of the CGenericBufferReactor class
// This class serves as a relay of events reported by the buffer monitor
// to the event class.  The purpose of this is to provide a monolithic model of
// event handling to the physicist/programmer.


/*!
    Construct a Generic Buffer Reactor.  This is the sort of buffer reactor
which is associated with a CBufferEvent.
\param Owner   - Owning event.
*/
CBufferEvent::CGenericBufferReactor::
CGenericBufferReactor(CBufferEvent& Owner)  :
  m_rOwner(Owner)
{}
/*!
   Called when a buffer arrives.  The Event's OnBuffer is called with
   the pointer to the buffer.

   \param rMonitor - reference to the monitor which fired the event.
   \param pBuffer  - `Pointer' to the event buffer.
 */

void
CBufferEvent::CGenericBufferReactor::
OnBuffer(CBufferMonitor& rMonitor,
	 DAQWordBufferPtr pBuffer)
{
  m_rOwner.OnBuffer(pBuffer);	// Relay to event.
  pBuffer.Get()->Release();		// Release the underlying buffer in order
				// to avoid deadlock in spectrodaq.
}
/*!
  Called when the buffer monitor times out, but only if the buffer event
  has been programmed to pass timeouts on to user code.  Again, this call
  is relayed to the Event's OnTimeout function.
  \param rMonitor - Reference to the event monitor (unused).
  */
void
CBufferEvent::CGenericBufferReactor::
OnTimeout(CEventMonitor& rMonitor)
{
  m_rOwner.OnTimeout();
}

// Implementation of CBufferEvent class.

// Constructors and other cannonical operations:

/*!
  Construct an anonymous event. The monitor is a standard buffer monitor,
  the event readctor is a CGenericBufferReactor.

  */
CBufferEvent::CBufferEvent() :
  CEvent(*(new CBufferMonitor),
	 *(new CGenericBufferReactor(*this))),
  m_rMonitor((CBufferMonitor&)CEvent::getMonitor()),
  m_rReactor((CGenericBufferReactor&)CEvent::getReactor())
{
  
}
/*!
   Called to create a named buffer event when the name is a char* string:
   */
CBufferEvent::CBufferEvent(const char* pName) :
 CEvent(pName,
	*(new CBufferMonitor),
	 *(new CGenericBufferReactor(*this))),
  m_rMonitor((CBufferMonitor&)CEvent::getMonitor()),
  m_rReactor((CGenericBufferReactor&)CEvent::getReactor())
{
}
/*!
   Called to create a named buffer event when the name is an stl string.
   */
CBufferEvent::CBufferEvent(const string& rName) :
 CEvent(rName,
	*(new CBufferMonitor),
	 *(new CGenericBufferReactor(*this))),
  m_rMonitor((CBufferMonitor&)CEvent::getMonitor()),
  m_rReactor((CGenericBufferReactor&)CEvent::getReactor())
{
}    
/*!
  Destroy the buffer event:
  */
CBufferEvent::~CBufferEvent()
{
  delete &m_rMonitor;
  delete &m_rReactor;
}
/*!
  Called to add a link.  The link must be added in the context
  of the event's thread or else buffers will not be received. The
  link addition requests, are therefore queued to the list of pending
  link add requests m_AddQueue.  This must be done in a globally synchronized
  way.
  \param url  - The url of the host to link data from.
  \param tag  - The match tag (after mask has been applied).
  \param mask - Receive mask to be anded with the buffer type.
  \param reliability - Can be:
  - COS_RELIABLE - Indicates the link should recieve all buffers.
  - COS_UNRELIABLE - Indicates the link need not receive all buffers.
  */
void
CBufferEvent::AddLink(const string& url,unsigned int tag, 
			 unsigned int mask, int reliability)
{
  CBufferEvent::AddLinkRequest AddQueue;
  
  AddQueue.s_url = url;
  AddQueue.s_tag = tag;
  AddQueue.s_mask = mask;
  AddQueue.s_linktype = reliability;

  // Now queue the connection:

  CApplicationSerializer::getInstance()->Lock();
  m_AddQueue.push_back(AddQueue);
  CApplicationSerializer::getInstance()->UnLock();
}
/*!
   Queues a link deletion. Links are deleted by matching URL, mask and 
   reliabilities.  The deletion must be done in the context of the
   thread receiving buffers.  Therefore, this member just synchronizes
   with the application global mutex, and adds the link description to the
   pending deletion queue.  Next time the buffer receiving thread executes
   (either because a buffer arrives or because the wait times out), new
   links will be made and old links deleted.

   \param url   - The URL describing the server which we will be getting
                  buffers from.
   \param tag   - The buffer tag.
   \param mask  - Mask to be applied to the inbound buffer.
   \param reliability - Can be any of:
      - COS_RELIABLE   - All matching buffers are delivered.
      - COS_UNRELIABLE - Matching buffers are only deliverd if receive has
                         no 'active' buffers.
 */ 
void
CBufferEvent::DeleteLink(const string& url, unsigned int tag, unsigned int mask,
			    int reliability)
{
  AddLinkRequest DelQel;
 
  DelQel.s_url         = url;
  DelQel.s_tag         = tag;
  DelQel.s_mask        = mask;
  DelQel.s_linktype = reliability;

  // Synchronize and queue the connection:

  CApplicationSerializer::getInstance()->Lock();
  m_DelQueue.push_back(DelQel);
  CApplicationSerializer::getInstance()->UnLock();

}

/*!
   This member function is the default (no-op) action when a buffer
   has been received on the link.

   \param pBuffer - A `pointer' into the DAQBuffer<T>
*/
void
CBufferEvent::OnBuffer(DAQWordBufferPtr& pBuffer)
{
}

/*!
  This member function is the default (no-op) action when waiting for buffers
  has timed out and timeout delivery is enabled.
*/
void
CBufferEvent::OnTimeout()
{
}
/*!
  Called periodically at event thread context to process any ITC's (inter
  thread communication) primitives which are required by the event.  In this
  case, we need to process the two link request queues:
  - m_AddQueue - queue of links to add.
  - m_DelQueue - queue of links to delete.

  since context switches are in theory unpredictable, it's possible to 
  queue a deletion on an add request which has not yet been processed.  
  Therefore, the add queue is processed first and then the delete queue.
  */
void
CBufferEvent::ProcessQueues()
{
  ProcessAddQueue();
  ProcessDelQueue();
}
/*!
  This utility function is called to take all of the elements in the 
  Add queue and create links corresponding to them.
  It should be called only in the context of the executing event thread.
  */
void
CBufferEvent::ProcessAddQueue()
{
  CApplicationSerializer::getInstance()->Lock();
  while(!m_AddQueue.empty()) {
    AddLinkRequest req = m_AddQueue.front(); // Dequeue an item.
    m_rMonitor.AddLink(req.s_url, req.s_tag, req.s_mask, 
		       req.s_linktype == COS_RELIABLE);
    m_AddQueue.pop_front();
    
  }
  CApplicationSerializer::getInstance()->UnLock();
  
}
/*!
   This utility function is called periodically in the context of the
   event thread.  It dequeues each element from the delete link request queue
   and deletes the corresponding link.
   */
void
CBufferEvent::ProcessDelQueue()
{
  CApplicationSerializer::getInstance()->Lock();
  while(!m_DelQueue.empty()) {
    AddLinkRequest req = m_DelQueue.front();

    MatchAll predicate(req.s_url, req.s_tag, req.s_mask);
    LinkIterator it = m_rMonitor.FindLink(predicate, m_rMonitor.beginLinks());

    if(it != m_rMonitor.endLinks()) 
      m_rMonitor.RemoveLink(it);
    m_DelQueue.pop_front();
  }
  CApplicationSerializer::getInstance()->UnLock();
}

/*!
  Called to get a description of this type of event.
  */
string
CBufferEvent::DescribeSelf()
{
  string result(" Buffer event\n");
  list<CBufferEvent::AddLinkRequest>::iterator i;

  result += CEvent::DescribeSelf();
  if(m_AddQueue.empty()) {
    result += "Add queue is emtpy\n";
  }
  else {
    result += "Add Queue contents: \n";
    i = m_AddQueue.begin();
    while(i != m_AddQueue.end()) {
      result += QueueEntryToString(*i);
    }
  }
  if(m_DelQueue.empty()) {
    result += "Delete queue is empty\n";
  }
  else {
    result += "Delete queue contents: \n";
    i = m_DelQueue.begin();
    while(i != m_DelQueue.end()) {
      result += QueueEntryToString(*i);
    }
  }
  return result;
}

/*!
 Represent the contents of a Link request entry (CBufferEvent<T>::AddLinkRequest&) as
 a string.  This is intended to be used by DescribSelf() when dumping the
 queues.
 */
string
CBufferEvent::QueueEntryToString(CBufferEvent::AddLinkRequest& rEntry)
{
  string result("URL: ");
  char   val[10];
  result += rEntry.s_url;
  result += "  tag: ";
  sprintf(val, "%x", rEntry.s_tag);
  result += val;
  result += "  Mask: ";
  sprintf(val, "%x", rEntry.s_mask);
  result += val;
  result += " Link flags: ";
  sprintf(val, "%x", rEntry.s_linktype);
  result += val;
  result += "\n";
}

