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

#ifdef __cplusplus
#pragma pack(pop)                    // return to our regularly scheduled packing.
#endif

#endif
