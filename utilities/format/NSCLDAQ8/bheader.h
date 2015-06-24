

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

#include <cstdint>

#define BUFFER_REVISION 5
#define JUMBO_BUFFER_REVISION 6
/*		Absolute time:		*/

namespace DAQ
{
  namespace V8
  {

    struct bftime
    {
      std::int16_t	month;			/* Month 1-12		*/     /* 3 */
      std::int16_t	day;			/* Day	 1-31		*/     /* 3 */
      std::int16_t	year;			/* e.g. 1987		*/
      std::int16_t	hours;			/* 0-23			*/     /* 3 */
      std::int16_t	min;			/* 0-59			*/     /* 3 */
      std::int16_t	sec;			/* 0-59			*/     /* 3 */
      std::int16_t	tenths;			/* 0-9.			*/     /* 3 */
    };

    /*		Structures which describe the final output data buffers */

    struct bheader				/* Data buffer header	*/
    {
      std::int16_t	nwds;			/* Used part of buffer	*/
      BufferTypes	  type;			/* buffer type		*/
      std::int16_t	cks;			/* checksum over used part of buffer */
      std::int16_t	run;			/* Run number		*/
      std::int32_t	seq;			/* Buffer sequence number */
      std::int16_t	nevt;			/* Event count in buffer    */
      std::int16_t	nlam;			/* Number of lam masks	    */
      std::int16_t	cpu;			/* Processor number	    */
      std::int16_t	nbit;			/* Number of bit registers */
      BufferVersion	buffmt;			/* Data format revision level */
      std::int16_t  ssignature;		/* Short byte order signature */
      std::int32_t  lsignature;		/* Long byte order signature  */
      std::int16_t	unused[2];		/* Pad out to 16 words.	    */

      bool mustSwap() const {
        return (lsignature != 0x01020304);
      }

    };

  } // end of V8
} // end of DAQ

#endif // BHEADER_H
