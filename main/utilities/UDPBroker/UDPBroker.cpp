/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2022.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
             Genie Jhang
	     FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  UDPBroker.cpp
 *  @brief: Broker dealing with a UDP data source
 */

/**
 * usage:
 *    --port   - Port on which to receive messages.
 *    --sink   - Sink URL (ringbuffer or file).
 *    --formatted - data are formatted in some ASCII format.
 */

#include <CUDPServer.h>
#include <CDataSinkFactory.h>
#include <CDataSink.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <DataFormat.h>
#include <CRingItem.h>
#include <stdint.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iomanip>
#include <Exception.h>

#include "udpbrokeropts.h"
#include "pcapHeader.h"
#include "datagramHeader.h"

/**
 * usage
 *    Output an error message, the program usage and
 *    exit with error status:
 * @param msg - the error message.
 */
void usage(const char* msg)
{
    std::cerr << msg << std::endl;
    cmdline_parser_print_help();
    exit(EXIT_FAILURE);
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
 *  @param sid  - user provided source id if not -1.
 *  @param datagram - the datagram
 *  @param nBytes   - number of bytes in the datagram.
 *  @return CRingItem* - pointer to a dynamically allocated ring item that
 *              must be deleted by the caller.
 */
static CRingItem*
makeRingItem(in_addr_t from, short port, int sid, void* datagram, size_t nBytes)
{
    // Let's get the stuff we need from the header to put into the body header
    // before creating the ring item:
    
    uint8_t *buffer = reinterpret_cast<uint8_t*>(datagram);

    int pointer = 0;
    pcap_hdr_t* pcap_hdr = reinterpret_cast<pcap_hdr_t*>(buffer);
    if (pcap_hdr -> magic_number == 0xa1b2c3d4) {
      pointer += sizeof(pcap_hdr_t);
    }

    pointer += sizeof(pcaprec_hdr_t);

    pDatagramHeader hdr = reinterpret_cast<pDatagramHeader>(buffer + pointer);
    uint64_t timestamp = ntohl(hdr -> srsHeader.udpTimestamp);
    uint8_t sourceId = 0;
    // If source id is not specified by user, use FEC ID as a source id
    if (sid == -1) {
        sourceId = hdr -> srsHeader.fecId;
    } else {
        sourceId = sid;
    }

    // Make the ring item
    CRingItem *pResult = new CRingItem(PHYSICS_EVENT, timestamp,
//    CRingItem *pResult = new CRingItem(FIRST_USER_ITEM_CODE, timestamp,
                                       sourceId, 0, nBytes + 1024);

    uint8_t* pData = reinterpret_cast<uint8_t*>(pResult -> getBodyCursor());
    memcpy(pData, buffer, nBytes);
    pData += nBytes;
    pResult -> setBodyCursor(pData);
    pResult -> updateSize();
    return pResult;
}

/**
 * mainloop
 *    Accepts datagrams from the server object and forwards them to the
 *    sink. Data sent looks like ring items of type FIRST_USER_ITEM_CODE
 *    The body header will be filled in as:
 *    - timestamp from the routing header.
 *    - sourceid as (detectorId << 8) | (subdetectorId)
 *    - barrier type 0.
 *    The ring item payload will be the complete datagram including
 *    routing header preceded by the IP address and port of the sender.
 *
 *    @param broker - the server object from which to get data.
 *    @param sink   - the sink to which to output data.
 *    @param sid    - user provided source id if not -1.
 *
 */
static void
mainloop(UDP::CUDPServer& server, CDataSink& sink, int sid)
{
    uint8_t datagram[65536];    // Largest possible datagram.
    
    while(1) {
        in_addr_t from;
        short     fromPort;
        int nrcv = server.receive(datagram, sizeof(datagram), from, fromPort);
        if (nrcv < 0) {
            perror("Unable to receive a data gram:");
            exit(EXIT_FAILURE);
        }

        CRingItem* pRingItem = makeRingItem(from, fromPort, sid, datagram, nrcv);
        sink.putItem(*pRingItem);
        delete pRingItem;
    }
}

/**
 * entry point
 *    - Process our command line arguments:
 *    - setup the server
 *    - setup our sink
 *    - Enter the main loop:
 */
int main (int argc, char** argv)
{
    gengetopt_args_info args;
    if (cmdline_parser(argc, argv, &args)) {
        usage("Failed to process command line:");
    }
    // Create the data sink:
    try {
        CDataSinkFactory fact;
        CDataSink* pSink = fact.makeSink(std::string(args.sink_arg));
        if(!pSink) {
            std::string msg("Failed to open data sink: ");
            msg += args.sink_arg;
            usage(msg.c_str());
        }

        // Setup the server:
        short port = args.port_arg;

        // Source ID by user
        int sid = -1;
        if (args.sourceid_given) {
            sid = args.sourceid_arg;
        }

        UDP::CUDPServer server(port);

        mainloop(server, *pSink, sid);
    }
    catch (CException& e) {
        std::string msg=e.ReasonText();
        usage(msg.c_str());
    }
}
