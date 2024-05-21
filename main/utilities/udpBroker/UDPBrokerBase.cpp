// /*
//     This software is Copyright by the Board of Trustees of Michigan
//     State University (c) Copyright 2022.

//     You may use this software under the terms of the GNU public license
//     (GPL).  The terms of this license are described at:

//      http://www.gnu.org/licenses/gpl.txt

//      Authors:
//              Ron Fox
//              Giordano Cerriza
//              Genie Jhang
//              Simon Giraud
// 	     FRIB
// 	     Michigan State University
// 	     East Lansing, MI 48824-1321
// */
// Purpose: bind to UDP datagram socket and encapsulate datagram into a ring item.
// Use this UDPBrokerBase class as a base class for a derived class e.g. for SRS.

#include "UDPBrokerBase.h"
#include "pcapHeader.h"
#include "datagramHeader.h"

#include <iostream>
#include <stdexcept>
#include <memory>
#include <CDataSinkFactory.h>
#include <CDataSink.h>
#include <cstdlib>
#include <string>
#include <string.h>
#include <DataFormat.h>
#include <CRingItem.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iomanip>
#include <Exception.h>
#include <bitset>


UDPBrokerBase::UDPBrokerBase() {
}

UDPBrokerBase::~UDPBrokerBase() {
}

/**
 * UDPBrokerBase initialization
 *
 *    @param port - the port from which to get data.
 *
 */
void UDPBrokerBase::initialize(short port) {
    m_port = port;
    m_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if (m_socket < 0) {
        throw std::runtime_error("UDPBrokerBase::initialize - Failed to create socket");
    }
}

/**
 * addSink
 *     Create new data sink
 *
 *    @param sinkType   - the sink to which to output data.
 *    @param sid    - user provided source id.
 *
 */
void UDPBrokerBase::addSink(std::string sinkType, int sid) {
    std::unique_ptr<CDataSink> dataSink;
    dataSink.reset(CDataSinkFactory().makeSink(sinkType)); 
    if (!dataSink) {
        throw std::runtime_error("UDPBrokerBase::addSink - Failed to create data sink");
    }
    // Transfer ownership from dataSink to m_dataSinks[sid]
    m_dataSinks[sid] = std::move(dataSink);
}

/**
 * run
 *    Initialize and start a UDP server.
 *    Bind the server socket m_port and listens for incoming datagrams.
 */
void UDPBrokerBase::run() {
    try {
        struct sockaddr_in servaddr;        
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = INADDR_ANY;
        servaddr.sin_port = htons(m_port);

        if (bind(m_socket, (const struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
            throw std::runtime_error("UDPBrokerBase::run - Failed to bind socket");
        }

        mainLoop();

    } catch (const std::exception& e) {
        std::cerr << "UDPBrokerBase::run - Exception: " << e.what() << std::endl;
    }
}

/**
 * mainLoop
 *    Accepts datagrams from the server object and forwards them to the
 *    sink via makeRingItem
 */
void UDPBrokerBase::mainLoop() {
    uint8_t datagram[65536];
    struct sockaddr_in senderAddr;
    socklen_t senderAddrLen = sizeof(senderAddr);

    while (1) {
        int bytesReceived = recvfrom(m_socket, datagram, sizeof(datagram), 0,
                                     (struct sockaddr*)&senderAddr, &senderAddrLen);
        if (bytesReceived < 0) {
            perror("UDPBrokerBase::mainLoop - Error receiving datagram");
            continue;
        }

        in_addr_t from = senderAddr.sin_addr.s_addr;
        short fromPort = ntohs(senderAddr.sin_port);

        makeRingItem(from, fromPort, datagram, bytesReceived);
    }
}

/**
 * makeRingItem
 *    Turn a datagram into a ring item.  Note that no transformation on the
 *    data will be done which leaves the routing header in network byte ordering.
 *
 *  Create a ring item that encapsulates a datagram we received from some
 *  remote system. Note that some information in the routing header is turned
 *  into body header info.  The datagram itself is unmodified.
 *
 *  @param from - the IP address from which the data came in network byte order.
 *  @param port - the port that sent the datagram in that host byte order.
 *  @param datagram - pointer to the datagram
 *  @param nBytes   - number of bytes in the datagram.
 */
void UDPBrokerBase::makeRingItem(in_addr_t from, short port, uint8_t* datagram, size_t nBytes) {
    int pointer = 0;
    pcap_hdr_t* pcap_hdr = reinterpret_cast<pcap_hdr_t*>(datagram);
    if (pcap_hdr -> magic_number == 0xa1b2c3d4) {
      pointer += sizeof(pcap_hdr_t);
    }

    pointer += sizeof(pcaprec_hdr_t);

    pDatagramHeader hdr = reinterpret_cast<pDatagramHeader>(datagram + pointer);

    // Part specific to srs, for other application makeRingItem probably needs to be override in derived class
    uint64_t timestamp = ntohl(hdr -> srsHeader.udpTimestamp);
    // Assumes here that sourceId matches the index of m_dataSinks[sid]
    uint8_t sourceId = hdr -> srsHeader.fecId;

    // Make the ring item
    std::unique_ptr<CRingItem> pResult(
      new CRingItem(PHYSICS_EVENT, timestamp, sourceId, 0, nBytes + 1024));

    uint8_t* pData = reinterpret_cast<uint8_t*>(pResult -> getBodyCursor());
    memcpy(pData, datagram, nBytes);
    pData += nBytes;
    pResult -> setBodyCursor(pData);
    pResult -> updateSize();
    auto dataSinkIt = m_dataSinks.find(sourceId);
    if (dataSinkIt != m_dataSinks.end() && dataSinkIt->second) {
        dataSinkIt->second->putItem(*pResult);
    }
    if (dataSinkIt == m_dataSinks.end()) {
        std::cerr << "UDPBrokerBase::makeRingItem - Received datagram with invalid source ID: " << sourceId << std::endl;
    }
}

// /**
//  * getSinks
//  *    Returns reference of m_dataSinks <sid, CDataSink*>
//  */
// const std::map<int, std::unique_ptr<CDataSink>>& UDPBrokerBase::getSinks() {
//   return m_dataSinks;
// }

/**
 * getSocket
 *    Returns m_socket
 */
int UDPBrokerBase::getSocket(){
    return m_socket;
}
