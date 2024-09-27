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


void SRSMaps::setChannelsMap(std::string map){

    if (map == "dc1S800") {
        setChannelsMap_dcS800(1);
    }
    else if (map == "dc2S800") {
        setChannelsMap_dcS800(2);
    }
    else if (map == "dc12S800") {
        setChannelsMap_dcS800(12);
    }
    else {
        std::cout << "SRSMaps::setChannelsMap - filling detector map using: "<< map << std::endl;
        setChannelsMapFile(map);
    }

    std::cout << "SRSMaps::setChannelsMap - detector map filled" << std::endl;

    return;
}


void SRSMaps::setChannelsMap_dcS800(uint8_t dcId){

    //std::ofstream outputMap("/user/0400x/srs/readout/outMap.txt");
    uint8_t fecId = 2;
    uint8_t minVmmId = 0;
    uint8_t maxVmmId = 15;
    uint8_t offsetMinVmmId = 0;
    if (dcId == 1) {
        minVmmId = 0;
        maxVmmId = 7;
    }
    else if (dcId == 2){
        minVmmId = 8;
        maxVmmId = 15;
    } 
    else if (dcId == 12){
        minVmmId = 0;
        maxVmmId = 15;
    } 
    uint8_t minRawCh = 2;
    uint8_t maxRawCh = 61;
    int shiftId[4] = {0, -2, -1, -1}; 
    for (uint8_t vmmId = minVmmId; vmmId <= maxVmmId; vmmId++){
        int tempId = 0;
        if (dcId == 12 && vmmId >= 8) offsetMinVmmId = 8;
        for (uint8_t rawChId = minRawCh; rawChId <= maxRawCh; rawChId++){
            if (tempId%4 == 0){
                tempId = 0;
            }
            m_channelsMap[fecId][vmmId][rawChId] = (maxRawCh-minRawCh+1)*(vmmId-(minVmmId+offsetMinVmmId)) + rawChId + shiftId[tempId];
            tempId++;
            //std::cout<<"SRSMaps::setChannelsMap_dcS800 "<<(int)vmmId<<" "<<(int)rawChId<<" "<<(int)m_channelsMap[fecId][vmmId][rawChId]<<std::endl;
            //outputMap<<(int)fecId<<" "<<(int)vmmId<<" "<<(int)rawChId<<" "<<(int)m_channelsMap[fecId][vmmId][rawChId]<<std::endl;
        }
    }
    //outputMap.close();
    return;
}


int SRSMaps::getMappedChannel(int fecNo, int vmmNo, int chNo){
    //std::cout<<"SRSMaps::getMappedChannel "<<fecNo<<" "<<vmmNo<<" "<<chNo<<" "<<m_channelsMap[fecNo][vmmNo][chNo]<<std::endl;
    return m_channelsMap[fecNo][vmmNo][chNo];
}
