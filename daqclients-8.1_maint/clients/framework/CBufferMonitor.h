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
  \class: CBufferMonitor


  Monitor to encapsulate a DAQ<type>Buffer.  The monitor can block 
  until the buffer receives data and indicates an event when this happens.
  Timeouts are also allowed on the receipt.
  
  Author:
    Jason Venema
    NSCL
    Michigan State University
    East Lansing, MI 48824-1321
    mailto:venemaja@msu.edu
*/

#ifndef __CBUFFERMONITOR_H     //Required for current class
#define __CBUFFERMONITOR_H

//
// Include files:
//
                               //Required for base classes
#ifndef __CEVENTMONITOR_H     //CEventMonitor
#include "CEventMonitor.h"
#endif

#ifndef __CCLASSIFIEDOBJECTREGISTRY
#include <CClassifiedObjectRegistry.h>
#endif

#ifndef __CNOSUCHLINKEXCEPTION_H
#include <CNoSuchLinkException.h>
#endif

#ifndef __CLINKFAILEDEXCEPTION_H
#include <CLinkFailedException.h>
#endif

#ifndef SPECTRODAQ_H
#include <spectrodaq.h>
#endif

//
// Forward class declaration
class CBufferMonitor;

struct LinkInfo {
  int Tag;     // the tag associated with the link
  int Mask;    // the mask associated with the link
  string URL;  // the URL defining the source system
  int linkid;  // identifies the link to the spectrodaq link manager object.
  int operator== (const LinkInfo& l) const {
    return ((Tag == l.Tag) &&
	    (Mask == l.Mask) &&
	    (URL == l.URL) &&
	    (linkid == l.linkid));
  }
};

typedef STD(list)<struct LinkInfo>::iterator LinkIterator;


// Helper classes: These are predicates which are used
// to match partial LinkInfo data structures.
//
class MatchURL {
  string m_sURL;
 public:
  MatchURL(string& rURL) :
    m_sURL(rURL) { };
  bool operator() (struct LinkInfo l) {
    return (l.URL.find(m_sURL) == 0);
  }
};

class MatchAll {
  string m_sURL;
  int m_nTag;
  int m_nMask;
 public:
  MatchAll(string& rURL, int nTag, int nMask) :
    m_sURL(rURL),
    m_nTag(nTag),
    m_nMask(nMask) { }
  virtual bool operator() (struct LinkInfo l) {
    return ((l.URL.find(m_sURL)== 0) &&
	    (l.Tag == m_nTag) &&
	    (l.Mask == m_nMask));
  }
};

class CBufferMonitor : public CEventMonitor
{
  DAQWordBuffer m_Buffer;    /*!  Encapsulated buffer */
  STD(list)<LinkInfo> m_lLinks;   /*!  List of links. */
  DAQLinkMgr daq_link_mgr;   /*!  A link manager */
  int          m_nTag;
  int          m_nMask;

 public: 


  // Default constructor
  
  CBufferMonitor(bool am_fTimedWait = true) :
    m_Buffer(0),
    CEventMonitor(am_fTimedWait),
    m_nTag(COS_MAXBUFTAG),
    m_nMask(COS_ALLBITS)
  {
    STD(list)<LinkInfo> empty;
    m_lLinks = empty;
    AppendClassInfo();
  }

  CBufferMonitor (const string& rName, 
		     bool am_fTimedWait = true) :
    CEventMonitor(rName, am_fTimedWait),
       m_nTag(COS_MAXBUFTAG),
       m_nMask(COS_ALLBITS)
    {
      STD(list)<LinkInfo> empty;
      m_lLinks = empty;
      AppendClassInfo();
    }
  
  CBufferMonitor (const char* pName, 
		     bool am_fTimedWait = true) :
    CEventMonitor(pName, am_fTimedWait),
       m_nTag(COS_MAXBUFTAG),
       m_nMask(COS_ALLBITS)

    {
      STD(list)<LinkInfo> empty;
      m_lLinks = empty;
      AppendClassInfo();
    }

  // Destructor
  ~CBufferMonitor ( ) { }

  //
  // Copy constructor and assignment may not be allowed
  //
 private:
  CBufferMonitor(const CBufferMonitor& aCBufferMonitor);
  CBufferMonitor operator= (const CBufferMonitor& aCBufferMonitor);

  // Selectors:
 public:
  
  DAQWordBuffer& getBuffer() 
    { 
      return m_Buffer;
    }  
  
  STD(list)<LinkInfo> getLinks() const
    {
      return m_lLinks;
    }

  DAQLinkMgr getLinkMgr() const
    {
      return daq_link_mgr;
    }
  
  // Attribute mutators:
 protected:
  
  void setBuffer (const DAQWordBuffer am_Buffer)
    { 
      m_Buffer = am_Buffer;
    }
  
  void setLinks (const STD(list)<LinkInfo> am_lLinks)
    { 
      m_lLinks = am_lLinks;
    }

  void setLinkMgr (const DAQLinkMgr am_daq_link_mgr)
    {
      daq_link_mgr = am_daq_link_mgr;
    }
  
  // Class operations:
 public:

  virtual CEventMonitor::result operator() ();
  virtual int AddLink (const string& URL, int tag=COS_MAXBUFTAG,
		       int mask=COS_ALLBITS, bool fReliable=true);
  void RemoveLink (int linkid);
  void RemoveLink (LinkIterator link);
  template<typename LinkMatchPredicate>
    LinkIterator FindLink (LinkMatchPredicate& rPredicate, 
			   LinkIterator startat);
  LinkIterator beginLinks ();
  LinkIterator endLinks ();
  DAQWordBufferPtr getBufferPointer (int nOffset = 0);
  void SetBufferTag (int tag=COS_ALLBITS);
  void SetBufferMask (int nMask);
  string DescribeSelf ();
};

typedef CBufferMonitor CWordBufferMonitor;

// This must be here since it's a template implementation:

/*!
  \fn LinkIterator CBufferMonitor::FindLink (LinkMatchPredicate& rPredicate,
                                             LinkIterator startat)

 Operation Type:
    Selector
 
 Purpose:
    Locates the first link that satisfies a given
    predicate. Predefined predicates include:
    MatchURL - matches URL only
    MatchAll - Matches URL, tag and mask.
    A LinkMatchPredicate is a function object implementing:
    bool operator()(LinkInfo) which returns TRUE if the link
    satisfies the predicate. Returns an iterator 'pointing'
    to the first match, or end() if there is no match.
*/
template<class LinkMatchPredicate>
LinkIterator
CBufferMonitor::FindLink (LinkMatchPredicate& rPredicate, 
			     LinkIterator startat)
{
  for(LinkIterator link = startat; link != m_lLinks.end(); link++) {
    if(rPredicate(*link))
      return link;
  }
  return m_lLinks.end();
}


#endif
