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

#ifndef __RINGDATASOURCE_H
#define __RINGDATASOURCE_H

#ifndef __DATASOURCE_H
#include <DataSource.h>
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif


// forward class definitions.

class CRingBuffer;
class CAllButPredicate;
class URL;

namespace NSCLDAQ
{

  /*!
    Concrete subclass of CDataSource for reading data elements from ring buffers.
    Since the ring buffer
    is specified via a URI, and opened via the CRingAccess::daqConsumeFrom
    static member function, the URI can represent a local or a remote ring.
    If a remote ring is specified, the usual proxy ring scheme is used to hoist
    data to localhost from the remote host.

    This is a template class whose template parameter is expected to be an 
    NSCLDAQ data type.
  */
  template<class T>  class RingDataSource : public DataSource<T>
  {
    // Private per object data:
    private:
      CRingBuffer*        m_pRing;        //!< ring buffer consuming from
      CAllButPredicate*   m_pPredicate;   //!< predicate for acceptance decisions
      URL&                m_url;          //!< url of ring

      // Canonical methods.
    public:
      /*! \brief Constructor
       *
       * \param url     location of ring (proto://....)
       * \param sample  list of types to sample
       * \param exclude list of types to exclude
       */
      RingDataSource(URL&                   url,
                     std::vector<uint16_t>  sample,
                     std::vector<uint16_t>  exclude);

      /*! \brief Virtual desctructor
       * 
       * Closes up shop. Destroys both the ring and the predicate the instance
       * owns.
       */
      virtual ~RingDataSource();
    private:
      RingDataSource(const RingDataSource& rhs);
      RingDataSource& operator=(const RingDataSource& rhs);
      int operator==(const RingDataSource& rhs) const;
      int operator!=(const RingDataSource& rhs) const;

      ///////   Mandatory public interface:
    public:

      /*! \brief Read data item from the file
       *
       * This reads data elements from the file until one has been found that is not
       * of a type in the exclusion list. Once one is found, it is returend to the caller.
       *
       * \returns pointer to data element read from file (caller assumes ownership)
       */
      virtual T* getItem();


      ////// Utilities:
    private:
      /*! \brief Opens up the ringbuffer for consumption
       *
       * 
       */
      void openRing();

      /*! \brief Make the predicate from the lists of sampled and excluded item types:
       * 
       *  \param sample   list of types to sample from
       *  \param exclude  list of types to exclude
       */
      void makePredicate(std::vector<uint16_t> sample, std::vector<uint16_t> exclude);


  };
  
} // end of NSCLDAQ namespace

// include the implementation
#include <RingDataSource.hpp>

#endif
