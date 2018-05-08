/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
             Jeromy Tompkins
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#ifndef IO_H
#define IO_H

#include <cstdint>
#include <utility>

#include <unistd.h>

namespace DAQ {
class CTimeout;
}

#include <set>
/**
 * @file io.h
 * @brief Commonly used I/O method definitions.
 * @author Ron Fox
 */

namespace io {

    enum ReturnCode {
        SUCCESS     = 0,
        ERROR       = 1,
        END_OF_FILE = 2,
        TIMED_OUT   = 3
    };

  void writeData (int fd, const void* pData , size_t size);

  ssize_t readData (int fd, void* pBuffer,  size_t nBytes);

  std::pair<ssize_t, ReturnCode>
  timedReadData(int fd, void* pBuffer,  size_t nBytes,
                        const ::DAQ::CTimeout& timeout);
  double freeSpacePercent(int fd);
  void closeUnusedFiles(std::set<int> keepOpen);

}


#endif
