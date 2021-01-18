#ifndef __EVTCREATORMAIN_H
#define __EVTCREATORMAIN_H

/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2021.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Giordano Cerizza
	     NSCL/FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

// Headers:

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

// Forward class definitions.

class CDataSink;
class CDataSinkFactory;
class CRingItem;
class CRingStateChangeItem;

/*!
   Class that represents the event generator application.
*/
class EvtCreatorMain
{
  int m_tstart;
  int m_tdiff;
  CDataSink* m_pSink;
  int m_nParams;
  int m_nEvents;
  std::string m_url;
  
  /*
  CRingBuffer*      m_pRing;
  std::string       m_eventDirectory;
  uint64_t          m_segmentSize;
  bool              m_exitOnEndRun;
  unsigned          m_nSourceCount;
  bool              m_fRunNumberOverride;
  uint32_t          m_nOverrideRunNumber;
  bool              m_fChecksum;
  void*             m_pChecksumContext;  
  uint32_t          m_nBeginsSeen;
  bool              m_fChangeRunOk;
  std::string       m_prefix;
  */

  // Constructors and canonicals:
  
 public:
  EvtCreatorMain();
  ~EvtCreatorMain();
  
 private:
  EvtCreatorMain(const EvtCreatorMain& rhs);
  EvtCreatorMain& operator=(const EvtCreatorMain& rhs);
  int operator==(const EvtCreatorMain& rhs) const;
  int operator!=(const EvtCreatorMain& rhs) const;
  
  // Object operations:
 public:
  int operator()(int argc, char**argv);
  
  // Utilities:
 private:
  void parseArguments(int argc, char** argv);
  void beginRun(CDataSink& sink);
  void endRun(CDataSink& sink);
  void event(CDataSink& sink);
  void runCreator();  
  /*
  int  openEventSegment(uint32_t runNumber, unsigned int segment);
  void recordData();
  void recordRun(const CRingStateChangeItem& item, CRingItem* pFormatItem);
  void writeItem(int fd, CRingItem&    item);
  std::string defaultRingUrl() const;
  uint64_t    segmentSize(const char* pValue) const;
  bool  dirOk(std::string dirname) const;
  bool  dataTimeout();
  size_t itemSize(CRingItem& item) const;
  std::string shaFile(int runNumber) const;
  bool isBadItem(CRingItem& item, int runNumber);
  */
};



#endif
