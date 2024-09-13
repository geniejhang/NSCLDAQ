/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
             NSCL
             Michigan State University
             East Lansing, MI 48824-1321
*/

#ifndef NSCLDAQ8_BHEADER_H
#define NSCLDAQ8_BHEADER_H

#include <V8/DataFormatV8.h>
#include <ByteBuffer.h>

#include <cstdint>
#include <iosfwd>
#include <iomanip>

#define BUFFER_REVISION 5
#define JUMBO_BUFFER_REVISION 6
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"

namespace DAQ
{
  namespace V8
  {

    /*!
     * \brief Human readable time that is easy to query
     *
     * This is the struct that is used by the control buffer types
     */
#pragma pack(push, 1)
    struct  bftime
    {
      std::uint16_t	month;			/* Month 1-12		*/     /* 3 */
      std::uint16_t	day;			/* Day	 1-31		*/     /* 3 */
      std::uint16_t	year;			/* e.g. 1987		*/
      std::uint16_t	hours;			/* 0-23			*/     /* 3 */
      std::uint16_t	min;			/* 0-59			*/     /* 3 */
      std::uint16_t	sec;			/* 0-59			*/     /* 3 */
      std::uint16_t	tenths;			/* 0-9.			*/     /* 3 */
    };
#pragma pack(pop)

    /*!
     * \brief Convert between std::time_t and V8::bftime
     * \param time a value set by time() or std::chrono::system_clock::to_time_t()
     * \return the equivalent bftime value
     */
    extern bftime to_bftime(const std::time_t& time);

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    /*!
     * \brief The bheader struct
     *
     * This header is common to all V8 buffer types. Many elements are defunct and
     * unused (i.e. cks, nlam, nbit, and cpu). The remaining define the type of
     * data that is carried in the remainder of the buffer, how it is structured,
     * the amount of valid data in the buffer, and how to intperpret it.  It is
     * always the very first data that is in a V8 buffer.
     */
#pragma pack(push, 1)
    struct bheader				/* Data buffer header	*/
    {
      std::uint16_t	nwds;			/* Incl. count of 16-bit units in buffer with valid data	*/
      std::uint16_t type;			/* buffer type		*/
      std::uint16_t	cks;			/* checksum over used part of buffer (unused) */
      std::uint16_t	run;			/* Run number		*/
      std::uint32_t	seq;			/* Number of physics events preceeding the buffer */
      std::uint16_t	nevt;			/* Event count in buffer    */
      std::uint16_t	nlam;			/* Number of lam masks	(unused)    */
      std::uint16_t	cpu;			/* Processor number	(unused)    */
      std::uint16_t	nbit;			/* Number of bit registers (unused) */
      std::uint16_t	buffmt;			/* Data format revision level */
      std::uint16_t ssignature;		/* Short byte order signature (0x0102)*/
      std::uint32_t lsignature;		/* Long byte order signature  (0x01020304)*/
      std::uint16_t	unused[2];		/* Pad out to 16 words.	    */


      /*!
       * \brief Default constructor
       *
       * This creates a "VOID" buffer.
       *
       * type = 0
       * nwds = 16
       * buffmt = 5 (i.e. V8::StandardVsn)
       * ssignature = 0x0102
       * lsignature = 0x01020304
       *
       * All other items initialized to 0.
       *
       */
      bheader();

      /*!
       * \brief Completely explicit constructor
       */
      bheader(std::uint16_t nwds, std::uint16_t type, std::uint16_t cks,
              std::uint16_t run, std::uint32_t seq,
              std::uint16_t nevt, std::uint16_t nlam,
              std::uint16_t cpu, std::uint16_t nbit,
              std::uint16_t buffmt, std::uint16_t ssignature,
              std::uint32_t lsignature, std::uint16_t unused0,
              std::uint16_t unused1);

      /*! Checks whether long signature is in native byte ordering */
      bool mustSwap() const {
        return (lsignature != BOM32);
      }


    }; // end of bheader
#pragma pack(pop)
  } // end of V8
} // end of DAQ


/////////////////////////////////////////////////////////////////////////////////
////// Operator overloads ///////////////////////////////////////////////////////


///////////////// Equality operator

extern int operator==(const DAQ::V8::bftime& lhs, const DAQ::V8::bftime& rhs);
extern int operator==(const DAQ::V8::bheader& lhs, const DAQ::V8::bheader& rhs);

////////////////////// Insertion into a ByteBuffer


extern
DAQ::Buffer::ByteBuffer& operator<<(DAQ::Buffer::ByteBuffer& buffer,
                                    const DAQ::V8::bftime& time);

extern
DAQ::Buffer::ByteBuffer& operator<<(DAQ::Buffer::ByteBuffer& buffer,
                                    const DAQ::V8::bheader& header);

namespace std {
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

}
#endif // BHEADER_H
