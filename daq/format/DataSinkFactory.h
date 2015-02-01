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



#ifndef DATASINKFACTORY_H
#define DATASINKFACTORY_H

#include <string>

namespace NSCLDAQ 
{

  // Forward declaration
  template<class T> class DataSink;


  /**! Factory class for constructing CDataSink objects
   *
   * When provided a universal resource identifier (URI), this 
   * will return the appropriate type of data sink.
   *
   * Supported sinks at the present are:
   *   CFileDataSink   - specified by the file:// protocol
   *                     (stdout can be specified as file:///stdout or - )
   *
   *   CRingDataSink   - specified by tcp:// or ring:// protocol
   */
  template<class T> class DataSinkFactory
  {
    
    public:
      /**! 
       * \brief Parse the argument and return the proper type of sink
       *
       * This is the means by which a user can request a new data sink.
       *
       * \param  uri    identifier of the sink (proto://...)
       *
       * \returns a new data sink (caller assumes ownership)
       */
      virtual DataSink<T>* makeSink(std::string uri);

    private:
      /**!
       * Create a file data sink for the specified file    
       *
       * \returns a new file data sink
       */
      DataSink<T>* makeFileSink(std::string fname); 

      /**!
       * Create a ring data sink with the specified name
       *
       * \returns a new ring data sink
       */
      DataSink<T>* makeRingSink(std::string ringname); 
  };

} // end of NSCLDAQ namespace

#include <DataSinkFactory.hpp>

#endif
