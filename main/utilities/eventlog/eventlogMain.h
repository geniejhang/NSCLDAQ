#ifndef __EVENTLOGMAIN_H
#define __EVENTLOGMAIN_H

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

#include <CStatusMessage.h>
#include <zmq.hpp>
#include <nsclzmq.h>

// Forward class definitions.

class CRingBuffer;
class CRingItem;
class CRingStateChangeItem;
class CStateClientApi;



/*!
   Class that represents the event log application.
   separating this out in a separate class may make
   possible unit testing of chunks of the application
   with cppunit
*/
class EventLogMain
{
  // Per object data:

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
  
  // Disk space logging:
  
  size_t            m_lastCheckedSize;
  int               m_freeWarnThreshold;
  bool              m_haveWarned;
  int               m_freeSevereThreshold;
  bool              m_haveSevere;
  std::string       m_appname;
  
  std::string       m_logService;
  ZmqSocket*    m_pLogSocket;
  CStatusDefinitions::LogMessage* m_pLogger;
  
  CStateClientApi*  m_pStateApi;
  
  
  // Constructors and canonicals:

public:
  EventLogMain();
  ~EventLogMain();

private:
  EventLogMain(const EventLogMain& rhs);
  EventLogMain& operator=(const EventLogMain& rhs);
  int operator==(const EventLogMain& rhs) const;
  int operator!=(const EventLogMain& rhs) const;

  // Object operations:
public:
  int operator()(int argc, char**argv);

  // Utilities:
private:
  void parseArguments(int argc, char** argv);
  int  openEventSegment(uint32_t runNumber, unsigned int segment);
  void recordData();
  void recordRun(const CRingStateChangeItem& item, CRingItem* pFormatItem);
  void writeItem(int fd, CRingItem&    item);
  std::string defaultRingUrl() const;
  uint64_t    segmentSize(const char* pValue);
  bool  dirOk(std::string dirname) const;
  bool  dataTimeout();
  size_t itemSize(CRingItem& item) const;
  std::string shaFile(int runNumber) const;
  bool isBadItem(CRingItem& item, int runNumber);
  
  // Helpers for logging.
  
  bool shouldLogWarning(double pct);
  bool shouldLogSevere(double pct);
  bool shouldLogSevereClear(double pct);
  bool shouldLogWarnClear(double pct);
  std::string getAggregatorURI();
  CStatusDefinitions::LogMessage* getLogger();
  void log(const char* msg, int severity);
  void log(const char* baseMessage, double free, int severity);
  void log(const char* msg, int errno, int severity);
  
  
  // Promised methods:
  
  void notReadyClose(int fd, int run);
  void writeChecksumFile(int runNumber);
  bool expectStateRequest(
    std::string& msg, const char* stateName,  int timeout
  );
  void stateManagerDie(const char* msg);

  
};



#endif
