#ifndef SRSMAPS_H
#define SRSMAPS_H

//#include <iostream>
#include <string>


class SRSMaps {
public:
    SRSMaps();
    ~SRSMaps ();

    void setChannelsMapFile(std::string mapFilePath);
    void setChannelsMap(std::string detector);
    int getMappedChannel(int fecNo, int vmmNo, int chNo);

private:

    void setChannelsMap_dcS800(uint8_t dcId);

    static const int MaxFECs{16};
    static const int MaxVMMs{16};
    //static const int MaxVMMs{32};//MaxVMMs used for markers, 16 normal trigger + 16 ext trigger
    static const int MaxChns{64};

    int m_channelsMap[16][16][64];//[fec][vmm][channel]

};

#endif
