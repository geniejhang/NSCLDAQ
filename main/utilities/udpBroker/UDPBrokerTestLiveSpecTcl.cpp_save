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
             Simon Giraud
	     FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  UDPBrokerTestLiveSpecTcl.cpp
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

static const int SRSHeaderSize{16};
static const int HitAndMarkerSize{6};
static const int Data1Size{4};
static const int MaxVMMs{16}; ///Maximum number of VMMs per FEC card
static const int MaxFECs{16}; ///Maximum number of FECs per EFU

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
    // before creating the ring item.
    // The datagram send from the slow controler has only srsHeader and data
    
    uint8_t *buffer = reinterpret_cast<uint8_t*>(datagram);

    psrshdr hdr = reinterpret_cast<psrshdr>(buffer);
    uint64_t timestamp = ntohl(hdr -> udpTimestamp);
    uint8_t sourceId = 0;
    // If source id is not specified by user, use FEC ID as a source id
    if (sid == -1) {
        sourceId = hdr -> fecId;
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

static void
testReadRing(CRingItem item)
{
    //Simon - the datagram has srsHeader + data
    std::cout<<"Has body header: "<<item.hasBodyHeader()<<std::endl;    
    uint16_t type  = item.type();
    uint32_t bytes = item.getBodySize();
    uint8_t* pItem = reinterpret_cast<uint8_t*>(item.getBodyPointer());
    int pointer = 0;
    

    if ((bytes % 6) != 0) {
        std::cout<<"error data length (bytes)"<<bytes<<std::endl;
    }
    std::cout<<"Bytes "<<bytes<<" bytes/6 "<<bytes/6.<<std::endl;

    
    psrshdr* hdr = reinterpret_cast<psrshdr*>(pItem);
    pointer += sizeof(srshdr);

    uint32_t srshdrId = ntohl(hdr->dataId);
    if ((srshdrId & 0xffffff00) != 0x564d3300) {
        std::cout<<"error dataId "<<srshdrId<<std::endl;
    }
    std::cout<<" dataId "<<std::hex<<srshdrId<<std::dec<<std::endl;
 

//Simon - test --------------------------------------------------------
                  int dataIndex = 0;
                  int readoutIndex = 0;
                  int hits = 0;

                  VMM3Marker *markers = new VMM3Marker[MaxFECs * MaxVMMs];


                  while (bytes >= HitAndMarkerSize) {

                    VMM3Data *vd = new VMM3Data();
                    printf("readoutIndex: %d, bytes %d \n",
                            readoutIndex,bytes);
                    auto Data1Offset = SRSHeaderSize + HitAndMarkerSize * readoutIndex;
                    auto Data2Offset = Data1Offset + Data1Size;
                    std::cout<<"data offset "<<Data1Offset<<" "<<Data2Offset<<std::endl;
                    /* std::cout<<"buffer "<<buffer[Data1Offset]<<" "<<buffer[Data2Offset]<<std::endl; */
                    /* uint32_t data1 = htonl(*(uint32_t *) &buffer[Data1Offset]); */
                    /* uint16_t data2 = htons(*(uint16_t *) &buffer[Data2Offset]); */
                    uint32_t data1 = htonl(*reinterpret_cast<uint32_t*>(&pBuffer[Data1Offset]));
                    uint16_t data2 = htons(*reinterpret_cast<uint16_t*>(&pBuffer[Data2Offset]));
                    /* uint32_t data1 = htonl((uint32_t) buffer[Data1Offset]); */
                    /* uint16_t data2 = htons((uint16_t) buffer[Data2Offset]); */


                    printf("data1: 0x%08x, data2: 0x%04x \n", data1, data2);
                    std::cout<<"bit rep of data1: "<<std::bitset<32>( data1 )<<std::endl;
                    std::cout<<"bit rep of data2: "<<std::bitset<16>( data2 )<<std::endl;
                    std::cout<<"bit rep of data1 >> 12: "<<std::bitset<32>( data1 >> 12 )<<std::endl;
                    std::cout<<"bit rep of data1 >> 12: "<<std::bitset<32>( 0x3FF)<<std::endl;
                    std::cout<<"bit rep of data1 >> 12: "<<std::bitset<32>( data1 >> 12 & 0x3FF)<<std::endl;
                    int dataflag = (data2 >> 15) & 0x1;
                    if(dataflag){
                        std::cout<<"SRS data"<<std::endl;
                    /* int res = parse(data1, data2, &data[dataIndex]); */
                        /// Data

                        vd->overThreshold = (data2 >> 14) & 0x01;
                        vd->chno = (data2 >> 8) & 0x3f;
                        vd->tdc = data2 & 0xff;
                        vd->vmmid = (data1 >> 22) & 0x1F;
                        vd->triggerOffset = (data1 >> 27) & 0x1F;
                        uint16_t idx = (hdr->fecId - 1) * MaxVMMs + vd->vmmid;
                        /* if(vd->triggerOffset < markers[idx].lastTriggerOffset) { */
                          /* if(markers[idx].calcTimeStamp != 0) { */
                          /*   markers[idx].calcTimeStamp +=32*srsTime.trigger_period_ns()/SRSTime::internal_SRS_clock_period_ns; */
                          /* } */
                          /* if(markers[idx].fecTimeStamp != markers[idx].calcTimeStamp){ */
                              /* std::cout<<"error timestamp"<<std::endl; */
                          /* } */
                        /* } */
                        markers[idx].lastTriggerOffset = vd->triggerOffset;
                        vd->adc = (data1 >> 12) & 0x3FF;
                        vd->bcid = BitMath::gray2bin32(data1 & 0xFFF);
                        /// \todo Maybe here use the calculated timestamp instead
                        /// vd->fecTimeStamp = markers[idx].calcTimeStamp;
                        vd->fecTimeStamp = markers[idx].fecTimeStamp;
                        if(vd->fecTimeStamp > 0) {
                          markers[idx].hasDataMarker = true;
                          vd->hasDataMarker = true;
                        }
                        printf("SRS Data: vmm: %d, channel: %d. adc: %d, tdc: %d \n",
                          vd->vmmid, vd->chno, vd->adc,vd->tdc);
                      } else {
                        /// Marker
                        uint8_t vmmid = (data2 >> 10) & 0x1F;
                        uint16_t idx = (pSrshdr->fecId - 1) * MaxVMMs + vmmid;
                        uint64_t timestamp_lower_10bit = data2 & 0x03FF;
                        uint64_t timestamp_upper_32bit = data1;

                        uint64_t timestamp_42bit = (timestamp_upper_32bit << 10)
                            + timestamp_lower_10bit;
                        printf("SRS Marker vmmid %d: timestamp lower 10bit %lu, timestamp upper 32 bit %lu, 42 bit timestamp %lu \n", vmmid, timestamp_lower_10bit, timestamp_upper_32bit, timestamp_42bit);

                        if(markers[idx].fecTimeStamp > timestamp_42bit) {
                          if (markers[idx].fecTimeStamp < 0x1FFFFFFF + timestamp_42bit) {
                            printf( "ParserTimestampSeqErrors:  ts %lu, marker ts %lu \n", timestamp_42bit, markers[idx].fecTimeStamp);
                          }
                        }
                        if(markers[idx].calcTimeStamp == 0) {
                          markers[idx].calcTimeStamp = timestamp_42bit;
                        }
                        markers[idx].fecTimeStamp = timestamp_42bit;
                      }

                    /* if (res == 1) { // This was data */
                    /*   hits++; */
                    /*   stats.ParserData++; */
                    /*   dataIndex++; */
                    /* } else { */
                    /*   stats.ParserMarkers++; */
                    /* } */
                    /* stats.ParserReadouts++; */
                    readoutIndex++;

                    bytes -= 6;
                    if (hits == 2000 && bytes > 0) {//2000 is maxHits
                        printf("Data overflow, skipping %d bytes", bytes);
                    }
                  }

//Simon -- end -- test ---------------------------------------------------------------


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
    
    std::cout<<"Simon debugg start main loop func"<<std::endl;
    while(1) {
        in_addr_t from;
        short     fromPort;
        int nrcv = server.receive(datagram, sizeof(datagram), from, fromPort);
        if (nrcv < 0) {
            perror("Unable to receive a data gram:");
            exit(EXIT_FAILURE);
        }

        std::cout<<"Simon debuggi in while main loop "<<sizeof(datagram)<<std::endl;

        CRingItem* pRingItem = makeRingItem(from, fromPort, sid, datagram, nrcv);

        testReadRing(*pRingItem);

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
