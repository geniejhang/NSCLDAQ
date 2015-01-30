#ifndef __RINGDATASOURCE_H
#define __RINGDATASOURCE_H
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
    Concrete subclass of CDataSource.  This provides the dumper application
    with a data source that comes from a ring buffer.  Since the ring buffer
    is specified via a URI, and opened via the CRingAccess::daqConsumeFrom
    static member function, the URI can represent a local or a remote ring.
    If a remote ring is specified, the usual proxy ring scheme is used to hoist
    data to localhost from the remote host.

  */
  template<class T>  class RingDataSource : public DataSource<T>
  {
    // Private per object data:
    private:
      CRingBuffer*        m_pRing;
      CAllButPredicate*   m_pPredicate;
      URL&                m_url;

      // Canonical methods.
    public:
      RingDataSource(URL&                   url,
                     std::vector<uint16_t>  sample,
                     std::vector<uint16_t>  exclude);
      virtual ~RingDataSource();
    private:
      RingDataSource(const RingDataSource& rhs);
      RingDataSource& operator=(const RingDataSource& rhs);
      int operator==(const RingDataSource& rhs) const;
      int operator!=(const RingDataSource& rhs) const;
    public:

      // Mandatory public interface:

    public:
      virtual T* getItem();

      // Utilities:

    private:
      void openRing();
      void makePredicate(std::vector<uint16_t> sample, std::vector<uint16_t> exclude);


  };
}

#include <RingDataSource.hpp>


#endif
