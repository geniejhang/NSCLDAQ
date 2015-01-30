/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
       NSCLDAQ Development Group
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef DATASINK_H
#define DATASINK_H

#ifndef __CRT_STDLIB_H
#include <stdlib.h>
#ifndef __CRT_STDLIB_H
#define __CRT_STDLIB_H
#endif
#endif

namespace NSCLDAQ
{
  /**! Interface for DataSinks
   *
   * This is a pure virtual base class that establishes an
   * expected interface for all data sinks.
   */
  template<class T> class DataSink
  {

    public:

      // The virtual destructor
      virtual ~DataSink() {};


      // A method defining how to send ring items to the sink
      virtual void putItem(const T& item) =0;

      // A method for putting arbitrary data to a sink:

      virtual void put(const void* pData, size_t nBytes) = 0;

  };
}
#endif

