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

/**
 * @file io.cpp
 * @brief Commonly used I/O methods.
 * @author Ron Fox.
 */

#include "io.h"
#include <CTimeout.h>

#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <set>
#include <sys/statvfs.h>

#include <system_error>
#include <cmath>


static std::set<int>                 okErrors;	// Acceptable errors in I/O operations.
/**
 * Return true if an I/O errno is not an allowed one.
 * 
 * @param error - the  errno to check.
 *
 * @return bool - True if the error is a bad one.
 *
 * @note okErrors is a set that will contain the 'allowed' errors.
 */
static bool 
badError(int error)
{
  // Stock the okErrors set if empty:

  if (okErrors.empty())
  {
    okErrors.insert(EAGAIN);
    okErrors.insert(EWOULDBLOCK);
    okErrors.insert(EINTR);
  }

  // Not in the set -> true.

  return (okErrors.count(error) == 0);
}


namespace io {
/**
 * Write a block of data to a file descriptor.
 * As with getBuffer, multiple writes are done..until either the entire data
 * are written or
 * *  A write ends in an eof condition.
 * *  A write ends in an error condition that is not EAGAIN, EWOUDLDBLOCK or EINTR.
 *
 * @param fd    - file descriptor to which the write goes:
 * @param pData - Pointer to the data to write.
 * @param size  - Number of words of data to write.
 * 
 * @throw int 0 - I/O showed eof on output.
 * @throw int errno - An error and why.
 */

void writeData (int fd, const void* pData , size_t size)
{
  const uint8_t* pSrc(reinterpret_cast<const uint8_t*>(pData));
  size_t   residual(size);
  ssize_t  nWritten;

  while (residual) {
    nWritten = write(fd, pSrc, residual);
    if (nWritten == 0) {
      throw 0;
    }
    if ((nWritten == -1) && badError(errno)) {
      throw errno;
    }
    // If an error now it must be a 'good' error... set the nWritten to 0 as no data was
    // transferred:

    if (nWritten < 0)
    {
      nWritten = 0;
    }
    // adjust the pointers, and residuals:


    residual -= nWritten;
    pSrc     += nWritten;
  }

}
/**
 * Get a buffer of data from  a file descritor.
 * If necessary multiple read() operation are performed to deal
 * with potential buffering between the source an us (e.g. we are typically
 * on the ass end of a pipe where the pipe buffer may be smaller than an
 * event buffer.
 * @param fd      - File descriptor to read from.
 * @param pBuffer - Pointer to a buffer big enough to hold the event buffer.
 * @param size    - Number of bytes in the buffer.
 *
 * @return size_t - Number of bytes read (might be fewer than nBytes if the EOF was hit
 *                  during the read.
 *
 * @throw int - errno on error.
 */
  ssize_t readData(int fd, void* pBuffer,  size_t nBytes)
{
  uint8_t* pDest(reinterpret_cast<uint8_t*>(pBuffer));
  size_t    residual(nBytes);
  ssize_t   nRead;

  // Read the buffer until :
  //  error other than EAGAIN, EWOULDBLOCK  or EINTR
  //  zero bytes read (end of file).
  //  Regardless of how all this ends, we are going to emit a message on sterr.
  //

  while (residual) {
    nRead = read(fd, pDest, residual);
    if (nRead == 0)		// EOF
    {
      return nBytes - residual;
    }
    if ((nRead < 0) && badError(errno) )
    {
      throw errno;
    }
    // If we got here and nread < 0, we need to set it to zero.
    
    if (nRead < 0)
    {
      nRead = 0;
    }

    // Adjust all the pointers and counts for what we read:

    residual -= nRead;
    pDest  += nRead;
  }
  // If we get here the read worked:

  return nBytes;		// Complete read.
}

  /**
   * Get a buffer of data from a file descriptor with a timeout
   * If necessary multiple read() operations are performed to deal
   * with potential buffering between the source and us (e.g. we are typically
   * on the end of a pipe where the pipe buffer may be smaller than an
   * event buffer.
   * @param fd      - File descriptor to read from.
   * @param pBuffer - Pointer to a buffer big enough to hold the event buffer.
   * @param size    - Number of bytes in the buffer.
   * @param timeout - a timeout specifier
   *
   * @return size_t - Number of bytes read (might be fewer than nBytes if the EOF was hit
   *                  during the read.
   *
   * @throw int - errno on error.
   */
  std::pair<ssize_t, ReturnCode>
  timedReadData(int fd, void* pBuffer,  size_t nBytes, const ::DAQ::CTimeout& timeout)
  {
      using namespace std::chrono;

    uint8_t*  pDest(reinterpret_cast<uint8_t*>(pBuffer));
    size_t    residual(nBytes);
    ssize_t   nRead = 0;

    // perform a read with a timeout iteratively until there is a timeout or something
    // else stops the reading.
    while (residual) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);

        struct timeval tv;
        // break up the remaining time into complete seconds and fractional seconds
        auto time = duration_cast<microseconds>(timeout.getRemainingTime());

        tv.tv_sec  = time.count()/1000000;
        tv.tv_usec = time.count()%1000000;

        // wait until fd is readable with the timeout
        int status = select(fd+1, &readfds, nullptr, nullptr, &tv);

        if (status < 0) {
            // error state... don't worry about EAGAIN, EINTR... just try again
            if ((errno == EAGAIN || errno == EINTR)) {
                if (timeout.expired()) {
                    return std::make_pair(nRead, TIMED_OUT);
                } else {
                    continue;
                }
            } else {
                // bad error
                throw std::system_error(errno, std::system_category(),
                                        "Failed while waiting on fd to become readable");
            }
        } else if (status == 0) {
            // timeout
            return std::make_pair(nRead, TIMED_OUT);
        } else {
            // there is a file descriptor set for reading ... read it!
            ssize_t result = read(fd, pDest, residual);
            if (result == 0) {
                return std::make_pair(nRead, END_OF_FILE);
            } else if ((result < 0)) {
                // error condition

                if (badError(errno) ) {
                    throw std::system_error(errno, std::system_category(),
                                            "Failed while reading from file descriptor");
                } else if (!timeout.expired()) {
                    continue;
                } else {
                    return std::make_pair(nRead, ERROR);
                }
            } else {
                // good data to read...
                nRead    += result;
                pDest    += result;
                residual -= result;
            }
        }

        if (timeout.expired()) {
            break;
        }
    }
    // If we get here the read worked:

    return std::make_pair(nRead, SUCCESS);	// Complete read.
  }


/**
 * freeSpacePercent
 *    Returns the percent of a filesystem that is free for user files.
 *
 *  @param[in] fd - File descriptor for a file open on the device.
 *  @return    float - Percent of filesystem free (100.0*free/total).
 *  @note      total space has to be determined by converting fragments into
 *             blocks (f_frsize/f_bsize).
 */
double freeSpacePercent(int fd)
{
  struct statvfs volumeInfo;
  if (fstatvfs(fd, &volumeInfo)) {
    throw errno;
  }
  fsblkcnt_t     availBlocks = volumeInfo.f_bavail;
  unsigned long  totalBlocks =
    volumeInfo.f_blocks * (volumeInfo.f_frsize/volumeInfo.f_bsize);
    
  return (100.0)*availBlocks/totalBlocks;
  
}

}
