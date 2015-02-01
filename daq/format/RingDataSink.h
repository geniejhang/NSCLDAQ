/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

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

// Forward declaration
class CRingBuffer;

namespace NSCLDAQ 
{
  /*! \brief A data sink for ring buffers.
   *
   * This provides the implementation for writing data elements to a ring buffer.
   * Because the ring buffer is the data sink, producing one of these is subject 
   * to succeed as long as no other processes have already attached to the ring
   * buffer as a producer.  Also, producer processes must be local to their ring
   * buffer so there are no URLs to locate a ring buffer somewhere on a network.
   *
   * This is provided as template class to work on arbitrary data types, however
   * it is expected that the data types it will operate on are NSCLDAQ types.
   */
  template<class T> class RingDataSink : public DataSink<T>
  {
    private:
      CRingBuffer*  m_pRing;      //!< the ring buffer
      std::string   m_ringName;   //!< name of the ring (on localhost)

    public:
      /*! \brief Constructor
       *
       * Attempts to attach to the ring buffer as a producer.
       *
       * \param ringName  name of ring
       *
       * \throws CDataSinkException - failed to open ring
       * \throws CErrnoException - a producer already exists for the ring
       */
      RingDataSink(std::string ringName);

      /*! \brief Destructor
       *
       * Destroys the ring buffer owned by this.
       */
      virtual ~RingDataSink();

    private:
      RingDataSink(const RingDataSink& rhs);
      RingDataSink& operator=(const RingDataSink& rhs);
      int operator==(const RingDataSink& rhs) const;
      int operator!=(const RingDataSink& rhs) const;

      // The interface functions required by the ABC:
    public:
      /*! \brief Writes a data item to the ring buffer
       *
       * This will block until the complete item has successfully been written
       * to the ringbuffer.
       *
       * \param item  the data element to write
       */
      void putItem(const T& item);

      /*! \brief Write generic data to ring buffer
       *
       * Can be used for transferring multiple data items to the ring buffer.
       *
       * \param pData     pointer to the data
       * \param nBytes    number of bytes to write
       */
      void put(const void* pData, size_t nBytes);

    private:
      /*! \brief Attach to the ring buffer as a producer
       *
       * This the real guts of what the constructor does. 
       *
       * \throws CErrnoException - producer already exists
       * \throws CDataSinkException - failed to open the ring
       */
      void openRing();

  };

} // end of NSCLDAQ namespace

// include the implementation
#include <RingDataSink.hpp>

#endif
