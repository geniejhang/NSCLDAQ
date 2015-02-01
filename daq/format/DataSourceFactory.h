/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef __DATASOURCEFACTORY_H
#define __DATASOURCEFACTORY_H

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __CRT_STDINT
#include <stdint.h>
#ifndef __CRT_STDINT
#define __CRT_STDINT
#endif
#endif

/**
 * @file DataSourceFactory.h
 * @author Ron Fox
 * @brief Define a class that produces RingDataSource and FileDataSource objects.
 */


namespace NSCLDAQ 
{

  // forward declaration
  template<class T> class DataSource;

  /**
   * \brief Factory for DataSources
   *
   *  This class produces ring and file ata source objects given a URI and other parameters.
   *  This centralizes the knowledge about how to interpret ring URI's and how to
   *  create the appropriate corresponding data source used by utilities that 
   *  can take datat from online and offline ring sources.
   *
   * This is a templated class whose template arguments are intended to be used
   * for NSCLDAQ data formats.
   */
  template<class T> class DataSourceFactory 
  {
    public:
      static DataSource<T>* makeSource(std::string uri, 
                                        std::vector<uint16_t> sample, 
                                        std::vector<uint16_t> exclude);
  };

} // end of NSCLDAQ namespace

// include implementation
#include <DataSourceFactory.hpp>

#endif
