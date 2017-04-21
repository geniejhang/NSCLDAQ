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
 * @file CFragwriter.cpp
 * @brief Fragment writer (fragment -> ring item -> file number) class implementation
 */

#include "CFragWriter.h"
#include <io.h>

#include <ByteBuffer.h>
#include <V12/CRawRingItem.h>
#include <V12/Serialize.h>
#include <ByteBuffer.h>
#include <ContainerDeserializer.h>

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <string>
#include <iostream>
#include <iomanip>

using namespace DAQ;
/**
 * Constructor
 *
 * @param fd - File descriptor to output ring fragments on.
 */
CFragWriter::CFragWriter(int fd) :
  m_fd(fd) {}

/**
 * operator() 
 *
 * Depending on whether the user wants to convert to ring items by stripping
 * off the fragment header or by simply converting to EVB_FRAGMENT types, this
 * will call the correct method. 
 *
 * @param pFragment - Really a pointer to a 'flattened' fragment.  A
 *                    flattened fragment is a fragment header that is immediately
 *                    followed by its payload.
 */
void
CFragWriter::operator()(const DAQ::Buffer::ByteBuffer& fragment)
{

    // note: fragment header is 20 bytes... we only care about the data that
    // comes after the first 20 bytes
  if (fragment.size() < 20) {
      throw std::runtime_error("Cannot process unless complete fragment exists! (i.e. > 20 bytes)");
  }


  const uint8_t* pBody = fragment.data() + 20;
  // Write the payload only
  Write(fragment.size() - 20, pBody);
}


/**
 * Write
 *
 * Write a block of data to the file descriptor.  If needed multiple underlying
 * write(2) calls are made.
 * 
 * @param size - Number of bytes to write.
 * @param pBuffer - Pointer to the first word of the buffer to write.
 */
void
CFragWriter::Write(size_t nBytes, const void* pBuffer)
{
  try {
    io::writeData(m_fd, pBuffer, nBytes);
  }
  catch(int e) {
    if(e) {
      throw std::string(strerror(e));
    } else {
      throw std::string("Premature end of file");
    }
  }
}

