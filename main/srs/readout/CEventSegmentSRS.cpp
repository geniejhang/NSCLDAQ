#include "CEventSegmentSRS.h"
#include "CTriggerSRS.h"

#include <config.h>
#include <CReadoutMain.h>
#include <CExperiment.h> 

#include <string>
#include <stdio.h>
#include <iomanip>
#include <float.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <signal.h>
#include <cstring>
#include <math.h>
#include <vector>
#include <deque>
#include <algorithm>
#include <iterator>
#include <cstdlib>
#include <string.h>
#include <stdexcept>
#include <future>
#include <regex>



#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif



CEventSegmentSRS::CEventSegmentSRS(CTriggerSRS *trig, CExperiment& exp)
 : m_systemInitialized(false),
   m_pExperiment(&exp),
   m_nCumulativeBytes(0),
   m_nBytesPerRun(0),
   m_clientTcp(new CTcpClient()),
   m_clientUdp(new UDPBrokerDerived())
{
    ios_base::sync_with_stdio(true);
    // Trigger object, useless for now
    mytrigger = trig;
}


CEventSegmentSRS::~CEventSegmentSRS()
{
    m_clientUdp->stop();
    if (m_clientUdpThread.joinable()) {
        m_clientUdpThread.join(); 
    }
    delete m_clientTcp;
    delete m_clientUdp;
}

void CEventSegmentSRS::initialize(){
}


// Overrides CExperiment onBegin() function.
void
CEventSegmentSRS::onBegin()
{

    // std::cout<<"CEventSegmentSRS::onBegin - "<<m_pExperiment->getRunNumber()<<" "<<m_pExperiment->getSourceId()<<std::endl;

    // Start vmmsc acquisition
    bool statusClientTcp = m_clientTcp->sendCommand("start");
    std::string response = m_clientTcp->receiveResponse();
    std::cout <<"onBegin response from VMMSC: "<< response << std::endl;
    if (!parseResponse(response)) {
        std::cerr << "VMMSC ACQ ON failed"<< std::endl;
        return;
    }
    // Provide trigger and clock info to set the hit timestamp 
    m_clientUdp->setTriggerMode(m_triggerIn, m_invTrigger);
    m_clientUdp->setClockMode(m_extClock);
    m_clientUdp->setClockPeriod(m_clockPeriod);

    //Give run number to put ring state change item into sinks
    //Could also give run title...
    m_clientUdp->setRunNumber(m_pExperiment->getRunNumber());
    // m_clientUdp->setSourceId(m_pExperiment->getSourceId());

    m_clientUdp->begin();
 
}


void
CEventSegmentSRS::onResume()
{
    std::cout << "CEventSegmentSRS::onResume " << std::endl;
    m_clientUdp->resume(); 
}


void
CEventSegmentSRS::onPause()
{
    std::cout << "CEventSegmentSRS::onPause " << std::endl;
    m_clientUdp->pause();
}


void CEventSegmentSRS::onEnd() 
{
    bool statusClientTcp = m_clientTcp->sendCommand("stop");
    std::string response = m_clientTcp->receiveResponse();
    std::cout <<"CEventSegmentSRS::onEnd response from VMMSC: "<< response << std::endl;
    if (!parseResponse(response)) {
        std::cerr << "VMMSC ACQ OFF failed"<< std::endl;
        return;
    }
    m_clientUdp->end();
    // std::cout<<"CEventSegmentSRS::onEnd - "<<m_pExperiment->getRunNumber()<<" "<<m_pExperiment->getSourceId()<<std::endl;
}


// Start udpBroker, which is listenning to SRS daq port, on a detached thread.
// Listen to udp within a while loop, skipped or not given certain flags set by run state changes.
void CEventSegmentSRS::configure(std::string configFile, std::string daqPortStr)
{
    std::string startCmd = "connect - " + configFile;

    // std::cout<<"CEventSegmentSRS::configure - before connect clientTcp"<<std::endl;

    // Connect to tcp server, hard code address and port here
    m_clientTcp->setAddressAndPort("127.0.0.1", 8585);
    m_clientTcp->connectToServer();
    //startCmd tells vmmsc to open FEC daq connections (equivalent to "open/check communication" button)
    m_clientTcp->sendCommand(startCmd);
    std::string response = m_clientTcp->receiveResponse();
    if (!parseResponse(response)) {
        std::cerr << "connection to Tcp server failed"<< std::endl;
        return;
    }
    // std::cout<<"CEventSegmentSRS::configure - response: "<<response<<std::endl;

    //get the active FEC ids and pass them to udpBroker (m_clientUdp) so that a ring buffer per FEC is created
    // std::cout<<"Active fec: "<<m_activeFecs.size()<<std::endl;
    // for(auto fec: m_activeFecs){
    //     std::cout<<"Active fec: "<<fec<<std::endl;
    // }
    // for(auto fec: m_activeFecsId){
    //     std::cout<<"Active fec # : "<<fec<<std::endl;
    // }

    int daqPort = stoi(daqPortStr);
    // The ring buffer base name could/should be defined by user,
    // here choose to build it automatically from FEC id recceived from vmmsc response (parseResponse).
    m_clientUdpThread = std::thread([&](int daqPort) {
        m_clientUdp->initialize(daqPort);
        // Could pass map file via cmd parameters (.settings.tcl)
        // Not implemented yet but can imagine something like:
        // m_clientUdp->setChannelsMap("mapFile");
        std::string dataSinkBase = "tcp://localhost/";
        for (size_t id = 0; id < m_activeFecs.size(); id++) {
            std::string aSink = dataSinkBase + m_activeFecs[id];
            m_clientUdp->addSink(aSink, m_activeFecsId[id]);
            std::cout<<"CEventSegmentSRS::configure - added data sink: "<<aSink<<std::endl;
        }
        m_clientUdp->run();
    }, daqPort);
    m_clientUdpThread.detach();

    //useless for now
    mytrigger->Initialize(1);
    // std::cout <<"after mytrigger->Initialize(1)" << std::endl;
}

// Parse response from vmmsc
// For now assumes triggerIn, invTrigger, extClock, clockPeriod common to all fec, should depend on the fec, need to update
bool CEventSegmentSRS::parseResponse(std::string response)
{
    char delimiter = ' ';
    std::istringstream iss(response);
    std::string word;
    bool status = false;
    m_activeFecs.clear();

    while (std::getline(iss, word, delimiter)) {
        std::regex patternFec("fec");
        std::regex patternTIn("triggerIn");
        std::regex patternInvT("invTrigger");
        std::regex patternExtClock("extClock");
        std::regex patternClockPeriod("clockPeriod");
        if (std::regex_search(word, patternFec)) {
            m_activeFecs.push_back(word);
            std::string arg = std::regex_replace(word, std::regex("[^0-9]*([0-9]+).*"), std::string("$1"));
            int sid = stoi(arg);
            //dirty trick to let sid [0,9] for other things
            sid += 10 ;
            m_activeFecsId.push_back(sid);
        } else if (std::regex_search(word, patternTIn)) {
            std::string arg = std::regex_replace(word, std::regex("[^0-9]*([0-9]+).*"), std::string("$1"));
            m_triggerIn = stoi(arg);
        }else if (std::regex_search(word, patternInvT)) {
            std::string arg = std::regex_replace(word, std::regex("[^0-9]*([0-9]+).*"), std::string("$1"));
            m_invTrigger = stoi(arg);
        }else if (std::regex_search(word, patternExtClock)) {
            std::string arg = std::regex_replace(word, std::regex("[^0-9]*([0-9]+).*"), std::string("$1"));
            m_extClock = stoi(arg);
        }else if (std::regex_search(word, patternClockPeriod)) {
            std::string arg = std::regex_replace(word, std::regex("[^0-9.]*([0-9.]+)"), std::string("$1"));
            m_clockPeriod = stod(arg);
        } else if (word == "parsed") {
            status = true;
        }
    }
    return status;
}

// Not used udpBroker already send a ring item into the wild, the Readout ringbuffer is empty.
// Will be used when do a proper readout.
size_t CEventSegmentSRS::read(void* rBuffer, size_t maxwords)
{
    // std::cout << "read maxwords " << maxwords <<std::endl;
    return 0;
}

void CEventSegmentSRS::clear()
{
    // Nothing to clear right now
}

void CEventSegmentSRS::disable()
{
    // Nothing to disable right now
}

void CEventSegmentSRS::boot()
{
    // Nothing to boot right now
}
