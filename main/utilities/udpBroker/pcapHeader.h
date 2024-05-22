/*
 * Composer:
 *          Genie Jhang
 *
 *     FRIB
 *     Michigan State University
 *     East Lansing, MI 48824-1321
*/

/** @file:  pcapHeader.h
 *  @brief: PCAP header structs included in libpcap-dev package
 *          Separate to handle PCAP file from VMM3a handy.
 *  
 *  Source: https://wiki.wireshark.org/Development/LibpcapFileFormat
 */

#ifndef PCAPHEADER_H
#define PCAPHEADER_H

#ifdef  __cplusplus
#pragma pack(push, 1)                // C++ requires this to pack the struct.
#endif

typedef struct pcap_hdr_s {
    uint32_t magic_number;   /* magic number */
    uint16_t version_major;  /* major version number */
    uint16_t version_minor;  /* minor version number */
    int32_t  thiszone;       /* GMT to local correction */
    uint32_t sigfigs;        /* accuracy of timestamps */
    uint32_t snaplen;        /* max length of captured packets, in octets */
    uint32_t network;        /* data link type */
} pcap_hdr_t;

typedef struct pcapng_hdr_s {
    uint32_t type;           /* header block type */
    uint32_t length;         /* header block length */
    uint32_t magic_number;   /* magic number */
    uint16_t version_major;  /* major version number */
    uint16_t version_minor;  /* minor version number */
    uint64_t section_length; /* section length*/
} pcapng_hdr_t;

typedef struct pcapng_hdrOpt_s {
    uint16_t type;           /* header option type */
    uint16_t length;         /* header option length */
} pcapng_hdrOpt_t;


typedef struct pcaprec_hdr_s {
    uint32_t ts_sec;         /* timestamp seconds */
    uint32_t ts_usec;        /* timestamp microseconds */
    uint32_t incl_len;       /* number of octets of packet saved in file */
    uint32_t orig_len;       /* actual length of packet */
} pcaprec_hdr_t;

#ifdef __cplusplus
#pragma pack(pop)                    // return to our regularly scheduled packing.
#endif

#endif
