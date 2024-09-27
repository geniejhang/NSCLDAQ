#ifndef CEVENTSEGMENTSRS_H
#define CEVENTSEGMENTSRS_H

#include <CExperiment.h>
#include <CEventSegment.h>
#include <CRingItem.h>
#include <CRingBuffer.h>
#include <UDPBrokerDerived.h>
#include <vector>
#include <deque>
#include <fstream>
#include <string>
#include <iostream>
#include <thread>

#include "CTcpClient.h"

using namespace std;

class CTriggerSRS;
class CExperiment;


class CEventSegmentSRS : public CEventSegment
{
private:

  CTcpClient *m_clientTcp;
  UDPBrokerDerived *m_clientUdp;
  CTriggerSRS *mytrigger;
    
  bool m_systemInitialized;
  CExperiment*  m_pExperiment;
   
  // Statistics:
    
  size_t m_nCumulativeBytes;
  size_t m_nBytesPerRun;

  std::thread m_clientUdpThread;

  std::vector<std::string> m_activeFecs;
  std::vector<int> m_activeFecsId;

  int m_triggerIn;
  int m_invTrigger;
  int m_extClock;
  double m_clockPeriod;

  bool parseResponse(std::string response);
    
public:
  CEventSegmentSRS(CTriggerSRS *trig, CExperiment& exp);
  CEventSegmentSRS();                  // For unit testing only!!
  ~CEventSegmentSRS();


  virtual void initialize();
  virtual size_t read(void* rBuffer, size_t maxwords);
  virtual void disable();
  virtual void clear();
  // manage explicit run start and resume operations
  virtual void onBegin();
  virtual void onResume();  
  virtual void onPause();  
  // virtual void onEnd(CExperiment* pExperiment);
  virtual void onEnd();

  void boot();       

  void configure(std::string configFile, std::string daqPortStr , std::string mapStr);         

};
#endif
