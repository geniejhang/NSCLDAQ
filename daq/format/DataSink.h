/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

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
   * expected interface for all data sinks. It is parameterized 
   * around a type, which should be any of the defined NSClDAq 
   * data formats. Specializations of the derived classes can be
   * provided for the specific data types.
   */
  template<class T> class DataSink
  {

    public:

      /*! \brief Default constructor */
      DataSink() = default;

      /*! \brief The virtual destructor
       *
       * No resources are managed by this class so this is a no-op.
       *
       */
      virtual ~DataSink() {};

    public:

      /* \brief A method defining how to send a unit of data to the sink
       *
       * \param item   a unit of data
       */
      virtual void putItem(const T& item) =0;

      /*! \brief Method for sending raw data to the sink. 
       *
       * This is a method for sending generic data to the sink. It can be
       * used for sending multiple units of data in a single call.
       *
       * \param pData   pointer to data for sending
       * \param nBytes  number of bytes to send, starting at pData
       */
      virtual void put(const void* pData, size_t nBytes) = 0;

  };
}
#endif

