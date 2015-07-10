

/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
             NSCL
             Michigan State University
             East Lansing, MI 48824-1321
*/

#ifndef NSCLDAQ8_BHEADER_H
#define NSCLDAQ8_BHEADER_H

#include <DataFormatV8.h>
#include <ByteBuffer.h>

#include <cstdint>
#include <iosfwd>
#include <iomanip>

#define BUFFER_REVISION 5
#define JUMBO_BUFFER_REVISION 6
/*		Absolute time:		*/

namespace DAQ
{
  namespace V8
  {

    struct bftime
    {
      std::uint16_t	month;			/* Month 1-12		*/     /* 3 */
      std::uint16_t	day;			/* Day	 1-31		*/     /* 3 */
      std::uint16_t	year;			/* e.g. 1987		*/
      std::uint16_t	hours;			/* 0-23			*/     /* 3 */
      std::uint16_t	min;			/* 0-59			*/     /* 3 */
      std::uint16_t	sec;			/* 0-59			*/     /* 3 */
      std::uint16_t	tenths;			/* 0-9.			*/     /* 3 */
    };

    extern bftime to_bftime(const std::time_t& time);

    /*		Structures which describe the final output data buffers */

    struct bheader				/* Data buffer header	*/
    {
      std::uint16_t	nwds;			/* Used part of buffer	*/
      std::uint16_t type;			/* buffer type		*/
      std::uint16_t	cks;			/* checksum over used part of buffer */
      std::uint16_t	run;			/* Run number		*/
      std::uint32_t	seq;			/* Buffer sequence number */
      std::uint16_t	nevt;			/* Event count in buffer    */
      std::uint16_t	nlam;			/* Number of lam masks	    */
      std::uint16_t	cpu;			/* Processor number	    */
      std::uint16_t	nbit;			/* Number of bit registers */
      std::uint16_t	buffmt;			/* Data format revision level */
      std::uint16_t ssignature;		/* Short byte order signature */
      std::uint32_t lsignature;		/* Long byte order signature  */
      std::uint16_t	unused[2];		/* Pad out to 16 words.	    */

      bheader();
      bheader(std::uint16_t nwds, std::uint16_t type, std::uint16_t cks, std::uint16_t run,
              std::uint32_t seq, std::uint16_t nevt, std::uint16_t nlam, std::uint16_t cpu,
              std::uint16_t nbit, std::uint16_t buffmt, std::uint16_t ssignature,
              std::uint32_t lsignature, std::uint16_t unused0, std::uint16_t unused1);

      bool mustSwap() const {
        return (lsignature != BOM32);
      }


    };

  } // end of V8
} // end of DAQ

inline bool operator==(const DAQ::V8::bftime& lhs, const DAQ::V8::bftime& rhs) {
  bool equal=true;
  equal &= (lhs.month==rhs.month);
  equal &= (lhs.day==rhs.day);
  equal &= (lhs.year==rhs.year);
  equal &= (lhs.hours==rhs.hours);
  equal &= (lhs.min==rhs.min);
  equal &= (lhs.sec==rhs.sec);
  equal &= (lhs.tenths==rhs.tenths);

  return equal;
}


inline bool operator==(const DAQ::V8::bheader& lhs, const DAQ::V8::bheader& rhs) {
  bool equal=true;
  equal &= (lhs.nwds==rhs.nwds);
  equal &= (lhs.type==rhs.type);
  equal &= (lhs.cks==rhs.cks);
  equal &= (lhs.run==rhs.run);
  equal &= (lhs.seq==rhs.seq);
  equal &= (lhs.nevt==rhs.nevt);
  equal &= (lhs.nlam==rhs.nlam);
  equal &= (lhs.cpu==rhs.cpu);
  equal &= (lhs.nbit==rhs.nbit);
  equal &= (lhs.buffmt==rhs.buffmt);
  equal &= (lhs.ssignature==rhs.ssignature);
  equal &= (lhs.lsignature==rhs.lsignature);
  equal &= (lhs.unused[0]==rhs.unused[0]);
  equal &= (lhs.unused[1]==rhs.unused[1]);

  return equal;
}

extern
DAQ::Buffer::ByteBuffer& operator<<(DAQ::Buffer::ByteBuffer& buffer,
                                    const DAQ::V8::bftime& time);

extern
DAQ::Buffer::ByteBuffer& operator<<(DAQ::Buffer::ByteBuffer& buffer,
                                    const DAQ::V8::bheader& header);

inline std::ostream& operator<<(std::ostream& stream, const DAQ::V8::bheader& header)
{
  stream << "{nwds:" << header.nwds << ", ";
  stream << "type:"<< header.type << ", ";
  stream << "cks:" << header.cks << ", ";
  stream << "run: "<< header.run << ", ";
  stream << "seq:" << header.seq << ", ";
  stream << "nevt:" << header.nevt << ", ";
  stream << "nlam:" << header.nlam << ", ";
  stream << "cpu:" << header.cpu << ", ";
  stream << "nbit:" << header.nbit << ", ";
  stream << "buffmt:" << header.buffmt << ", ";
  stream << "ssig:" << std::hex << header.ssignature << std::dec << ", ";
  stream << "lsig:" << std::hex << header.lsignature << std::dec << ", ";
  stream << "u[0]:" << header.unused[0] << ", ";
  stream << "u[1]:" << header.unused[1] << "}";
  return stream;
}
#endif // BHEADER_H
