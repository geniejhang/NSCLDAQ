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

/** @file:  datagramHeader.h
 *  @brief: Provides the structure of a datagram header for FEC
 */
#ifndef DATAGRAMHEADER_H
#define DATAGRAMHEADER_H

#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <linux/if_packet.h>

#ifdef  __cplusplus
#pragma pack(push, 1)                // C++ requires this to pack the struct.
#endif

typedef struct _srshdr {
    uint32_t frameCounter;
    uint32_t dataId:24;              // Data ID: 0x564d33 - VMM3a Data
    uint8_t  :4;                     // 4-bit Padding
    uint8_t  fecId:4;
    uint32_t udpTimestamp;
    uint32_t offsetOverflow;
} srshdr, *psrshdr;

typedef struct _datagramHeader {
    struct ether_header ethernetHeader;
    struct iphdr        ipHeader;
    struct udphdr       udpHeader;
    srshdr              srsHeader;
} datagramHeader, *pDatagramHeader;

typedef struct _datagramHeaderNoSRS {
    struct ether_header ethernetHeader;
    struct iphdr        ipHeader;
    /* struct udphdr       udpHeader; */
    uint16_t  udpSource;                    
    uint16_t  udpDestination;                    
    uint16_t  udpLength;                    
    uint16_t  udpCheckSum;                    
} datagramHeaderNoSRS, *pDatagramHeaderNoSRS;

  /// Data related to a single Hit
typedef struct _VMM3Data {
    uint64_t fecTimeStamp; /// 42 bits can change within a packet so must be here
    uint16_t bcid;         /// 12 bit - bcid after graydecode
    uint16_t adc;          /// 10 bit - adc value from vmm readout
    uint8_t tdc;           ///  8 bit - tdc value from vmm readout
    uint8_t chno;          ///  6 bit - channel number from readout
    uint8_t overThreshold; ///  1 bit - over threshold flag for channel from readout
    uint8_t vmmid;         ///  5 bit - asic identifier - unique id per fec 0 - 15
    uint8_t triggerOffset; ///  5 bit
    bool hasDataMarker;    ///
  }VMM3Data, *pVMM3Data;

  // \todo no need for this struct
typedef struct _VMM3Marker {
    uint64_t fecTimeStamp{0};   /// 42 bit
    uint64_t calcTimeStamp{0};   /// 42 bit
    uint16_t lastTriggerOffset{0}; 
    bool hasDataMarker{false};
  }VMM3Marker, *pVMM3Marker;


#ifdef __cplusplus
#pragma pack(pop)                    // return to our regularly scheduled packing.
#endif

#endif
