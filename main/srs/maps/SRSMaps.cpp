// /*
//     This software is Copyright by the Board of Trustees of Michigan
//     State University (c) Copyright 2022.

//     You may use this software under the terms of the GNU public license
//     (GPL).  The terms of this license are described at:

//      http://www.gnu.org/licenses/gpl.txt

//      Authors:
//              Simon Giraud
// 	     FRIB
// 	     Michigan State University
// 	     East Lansing, MI 48824-1321
// */


#include "SRSMaps.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <stdlib.h>
#include <string>


SRSMaps::SRSMaps() {
}


SRSMaps::~SRSMaps() {
}


void SRSMaps::setChannelsMapFile(std::string mapFilePath){

    std::ifstream mapFile(mapFilePath);

    if (!mapFile.is_open()) {
        std::cerr << "SRSMaps::setChannelsMapFile - Error opening mapFile: " << mapFilePath << std::endl;
        return;
    }
    std::string line;
    while (std::getline(mapFile, line)) {
        std::istringstream iss(line);
        int fecId, vmmId, rawChId, chMapped;
        if (iss >> fecId >> vmmId >> rawChId >> chMapped) {
            m_channelsMap[fecId][vmmId][rawChId] = chMapped;
        } else {
            std::cerr << "SRSMaps::setChannelsMapFile - Invalid map file line format, expect: fecId vmmId rawChId chMapped" << std::endl;
        }
    }
    mapFile.close();

    return;
}


void SRSMaps::setChannelsMap(std::string detector){

    //std::cout<<"Simon - SRSMaps::setChannelsMap - detector "<<detector<<std::endl;

    if (detector == "dcS800") {
        setChannelsMap_dcS800();
    }
    else {
        std::cerr << "SRSMaps::setChannelsMap - detector identifier unknown" << std::endl;
    }

    return;
}


void SRSMaps::setChannelsMap_dcS800(){

    uint8_t fecId = 2;
    uint8_t minVmmId = 8;
    uint8_t maxVmmId = 15;
    uint8_t minRawCh = 2;
    uint8_t maxRawCh = 61;
    int shiftId[4] = {0, -2, -1, -1}; 
    for (uint8_t vmmId = minVmmId; vmmId <= maxVmmId; vmmId++){
        int tempId = 0;
        for (uint8_t rawChId = minRawCh; rawChId <= maxRawCh; rawChId++){
            if (tempId%4 == 0){
                tempId = 0;
            }
            m_channelsMap[fecId][vmmId][rawChId] = (maxRawCh-minRawCh+1)*(vmmId-minVmmId) + rawChId + shiftId[tempId];
            tempId++;
            //std::cout<<"SRSMaps::setChannelsMap_dcS800 "<<(int)vmmId<<" "<<(int)rawChId<<" "<<(int)m_channelsMap[fecId][vmmId][rawChId]<<std::endl;
        }
    }
    return;
}


int SRSMaps::getMappedChannel(int fecNo, int vmmNo, int chNo){
    //std::cout<<"SRSMaps::getMappedChannel "<<fecNo<<" "<<vmmNo<<" "<<chNo<<" "<<m_channelsMap[fecNo][vmmNo][chNo]<<std::endl;
    return m_channelsMap[fecNo][vmmNo][chNo];
}