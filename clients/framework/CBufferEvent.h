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

//! \class: CBufferEvent           
//! \file:  CBufferEvent.h
// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//
// Copyright 

#ifndef __CBUFFEREVENT_H  //Required for current class
#define __CBUFFEREVENT_H

//
// Include files:
//

                               //Required for base classes
#ifndef __CEVENT_H     // CEvent
#include "CEvent.h"
#endif

#ifndef __CBUFFERMONITOR_H
#include <CBufferMonitor.h>
#endif

#ifndef __CBUFFERREACTOR_H
#include <CBufferReactor.h>
#endif

#ifndef __STL_LIST
#include <list>
#define __STL_LIST
#endif

#ifndef __STL_STRING
#include <string>
#define __STL_STRING
#endif

#ifndef __SPECTRODAQ_H
#include <spectrodaq.h>
#define __SPECTRODAQ_H
#endif

#ifndef __CAPPLICATIONSERIALIZER_H
#include <CApplicationSerializer.h>
#endif



/*!
Provides an ABC for building
application level objects to react to
SpectroDaq Buffers.  This is an abstract,
templated class which is templated by the
type of buffer whch can be received.
Note that depending on how this is constructed,,
The object can handle alarm events instead
of data buffers.

*/
class CBufferEvent  : public CEvent        
{
  // Private data structures:

  //!  Form of request to add a link to the link manager.
  struct AddLinkRequest {
    string       s_url;		//!< URL of source system.
    unsigned int s_tag;		//!< tag to match against.
    unsigned int s_mask;	//!< Accpetance mask to apply to tags.
    unsigned int s_linktype;	//!< Type of link (COS_RELIABLE e.g.).
  };

  /*! The buffer reactor for CBufferEvent is actually a nested class:
      It relays all of the calls back to the event's virtual functions.
      this allows the presentation of a monolithic model for managing the 
      events.
  */

  class CGenericBufferReactor : public CBufferReactor
  {
    CBufferEvent& m_rOwner;
  public:
    CGenericBufferReactor(CBufferEvent& owner);

    virtual void OnBuffer(CBufferMonitor& rMonitor,
			 DAQWordBufferPtr pBuffer);
    virtual void OnTimeout(CEventMonitor& rMonitor);
  };

  // Private member data:

  // The two request queues must have a synchronized access.

  STD(list)<AddLinkRequest> m_AddQueue; //!< Requests to add links go here.
  STD(list)<AddLinkRequest> m_DelQueue;  //!< Requests to delete links go here.

  CBufferMonitor&   m_rMonitor; //!< Monitors the input links.
  CGenericBufferReactor&   m_rReactor; //!< Reacts to the input links.


public:
  // Constructors and other cannonical operations:

  CBufferEvent();		   //!< Anonymous buffer event.
  CBufferEvent(const char* pName); //!< Named event with char* name.
  CBufferEvent(const string& rName); //!< Named event with string name.
  ~CBufferEvent();		//!< Destroy the event.

  // Copy construction, assignment, comparison are all illegal:

private:
  CBufferEvent(const CBufferEvent& rhs);
  CBufferEvent& operator=(const CBufferEvent& rhs);
  int          operator==(const CBufferEvent& rhs);
public:

  // Selectors:
public:
  STD(list)<AddLinkRequest> getPendingAddQueue() const {
    CApplicationSerializer::getInstance()->Lock();
    STD(list)<AddLinkRequest> result = m_AddQueue;
    CApplicationSerializer::getInstance()->UnLock();
    return result;
  }
  STD(list)<AddLinkRequest> getPendingDeleteQueue() const {
    CApplicationSerializer::getInstance()->Lock();
    STD(list)<AddLinkRequest> result = m_DelQueue;
    CApplicationSerializer::getInstance()->UnLock();
    return result;
  }
  CBufferMonitor& getMonitor() {
    return m_rMonitor;
  }
  CBufferReactor& getReactor() {
    return m_rReactor;
  }

  // Class operations:

  void AddLink(const string& url,unsigned int tag, 
	       unsigned int mask = ALLBITS_MASK, 
	       int reliability = COS_RELIABLE);
  void DeleteLink(const string& url, unsigned int tag, 
		  unsigned int mask = ALLBITS_MASK,
		  int reliability = COS_RELIABLE);

  virtual void OnBuffer(DAQWordBufferPtr& pBuffer);
  virtual void OnTimeout();

  virtual void setBufferTag(int tag) {
    m_rMonitor.SetBufferTag(tag);
  }
  virtual void setBufferMask(int mask) {
    m_rMonitor.SetBufferMask(mask);
  }
  virtual string DescribeSelf();
protected:
  virtual void ProcessQueues();
  void ProcessAddQueue();
  void ProcessDelQueue();
  string QueueEntryToString(AddLinkRequest& rEntry);
};


#endif

