#ifndef UDPBROKERDERIVED_H
#define UDPBROKERDERIVED_H

#include "UDPBrokerBase.h"
#include <CDataSink.h>
#include <CRingItem.h>
#include <SRSMaps.h>
#include <SRSSorter.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <memory>
#include <map>
#include <vector>
#include <chrono>


class UDPBrokerDerived : public UDPBrokerBase {
public:
    UDPBrokerDerived();
    ~UDPBrokerDerived ();

    virtual void initialize(short port, std::string mapStr);
    virtual void addSink(std::string sinkType, int sid);

    //void virtual run();
    void begin();
    void end();
    void stop();
    void pause();
    void resume();

    void setTriggerMode(int triggerIn, int invTrigger);
    void setClockMode(int extClock);
    void setClockPeriod(double period);
    void setRunNumber(uint32_t runNb);
    void setSourceId(uint32_t sourceId);


protected:
    virtual void mainLoop();


private:

    void makeRingItems(in_addr_t from, short port, CDataSink& sink, int sid, uint8_t* buffer, size_t nBytes);
    void extractHitTimeStamp(uint8_t sourceId, uint8_t* data);
    void mapping(uint8_t* data, int fecId);
    void testReadData(uint8_t* data);
    uint16_t invertByteOrder(uint16_t data);

    bool m_stopMainLoop;
    bool m_pauseMainLoop;

    int m_triggerMode;
    int m_extClock;
    double m_clockPeriod;
    uint32_t m_runNumber;
    uint32_t m_sourceId;

    static const int MaxFECs{16};
    static const int MaxVMMs{32};//MaxVMMs used for markers, 16 normal trigger + 16 ext trigger
    static const int MaxChns{64};
    bool startedMarker[MaxFECs * MaxVMMs] = {false};

    int m_hitCounter;
    int m_datagramCounter;
    int m_markerCounter;
    int m_markerErrCounter;
    int m_firstDataCounter;
    bool m_startChrono;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start, m_end, m_pause, m_resume;

    struct VMM3Marker
    {
      uint64_t fecTimeStamp{0};  /// 42 bit
      uint64_t calcTimeStamp{0}; /// 42 bit
      uint16_t lastTriggerOffset{0};
      bool hasDataMarker{false};
    } *markerSRS;

    struct newData
    {
        uint64_t hitTimeStamp{0};
        uint16_t chnoMapped{0};
    } tsAndMappedChno;

    static const int SRSHeaderSize{16};
    static const int HitAndMarkerSize{6};
    static const int Data1Size{4};
    static const int Data2Size{2};

    std::unique_ptr<SRSMaps> m_channelsMap;
    std::unique_ptr<SRSSorter> m_sorter;
    
};

#endif
