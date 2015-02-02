/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/




#ifndef RINGDATASINK_H
#define RINGDATASINK_H

#include <DataSink.h>

#include <string>

class CRingBuffer;

namespace NSCLDAQ 
{
  template<class T> class RingDataSink : public DataSink<T>
  {
    private:
      CRingBuffer*  m_pRing;
      std::string   m_ringName; 

    public:
      RingDataSink(std::string ringName);
      virtual ~RingDataSink();

    private:
      RingDataSink(const RingDataSink& rhs);
      RingDataSink& operator=(const RingDataSink& rhs);
      int operator==(const RingDataSink& rhs) const;
      int operator!=(const RingDataSink& rhs) const;

      // The interface functions required by the ABC:
    public:
      void putItem(const T& item);
      void put(const void* pData, size_t nBytes);

    private:
      void openRing();

  };

} // end of NSCLDAQ namespace

// include the implementation
#include <RingDataSink.hpp>

#endif
