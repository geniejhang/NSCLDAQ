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



#ifndef DATASINKFACTORY_H
#define DATASINKFACTORY_H

#include <string>

namespace NSCLDAQ 
{

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
   * To be supported in the future:
   *   CRingDataSink   - TODO...
   *
   */
  template<class T> class DataSinkFactory
  {
    public:
      /**! 

        Parse the argument and return the proper type of sink
        */
      virtual DataSink<T>* makeSink(std::string uri);

    private:
      /**!
        Create a file data sink for the specified file    
        */
      DataSink<T>* makeFileSink(std::string fname); 

      /**!
        Create a ring data sink with the specified name
        */
      DataSink<T>* makeRingSink(std::string ringname); 
  };

} // end of NSCLDAQ namespace

#include <DataSinkFactory.hpp>

#endif
