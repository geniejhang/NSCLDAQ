/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/**
 * @file CFragwriter.h
 * @brief Fragment writer (fragment -> ring item -> file number) class definition.
 */


#ifndef CFRAGWRITER_H
#define CFRAGWRITER_H

#include <ByteBuffer.h>

#include <stdlib.h>		/* for size_t */

/**
 * CFragWriter
 *
 *  Class to write fragments as EVB_FRAG ring items to a file id.
 *  To use construct on the output file descriptor.  Invoke operator()
 *  passing it a pointer to a flattened ring item.
 *
 *  A flattened ring item is a EVB::FragmentHeader immediately followed by
 *  its payload (size determined by s_size).
 */
class CFragWriter
{
  // private data:
private:
  int m_fd;			// output file descriptor.

  // Canonicals we need:

public:
  CFragWriter(int fd);

  // public operations:

public:
  void operator()(const DAQ::Buffer::ByteBuffer& fragment);

  // Private utilities:

private:
  void Write(size_t nBytes, const void *pBuffer);

};

#endif
