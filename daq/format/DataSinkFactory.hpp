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


#include "FileDataSink.h"
#include "RingDataSink.h"
#include <URL.h>
#include <string>
#include <iostream>
#include <errno.h>

namespace NSCLDAQ 
{

/**! Factory method
* Parses the string as a URL and then call the appropriate 
* private utility method based on the protocol provided.
* Supported protocols are tcp:// and file://. The stdout can
* be obtained by providing file:///stdout or -
*
* \param uri a string of the form protocol://host/path:port
* \return a data sink on success, 0 on failure
*/
  template<class T>
    DataSink<T>* DataSinkFactory<T>::makeSink(std::string uri)
    {
      DataSink<T>* sink = 0;

      // Treat the special case of -
      if (uri=="-") {
        sink = makeFileSink(uri);
      } else {

        // parse the uri
        URL url(uri);
        std::string host = url.getHostName();
        if ((host != "localhost") && (host != "")) {
          errno = EREMOTE;
          throw CErrnoException("CDataSinkFactory::makeSink");
        }

        // 
        if (url.getProto()=="file") {

          sink = makeFileSink(url.getPath());

        } else if (url.getProto()=="ring" || url.getProto()=="tcp") {

          sink = makeRingSink(url.getPath());

        } 

      }
      return sink;

    }

  /**! Handle the construction of a file data sink from a path
   *
   * On successful construction of a CFileDataSource, a pointer to
   * the dynamically allocated object will be passed to the caller.
   * The caller will own the object at this point.
   *
   * This may throw as a result of the constructor objects. 
   * 
   * \return pointer to a sink on success, 0 on failure 
   */
  template<class T>
    DataSink<T>* DataSinkFactory<T>::makeFileSink(std::string fname) 
    {

      DataSink<T>* sink=0;

      try {

        if (fname=="-") {

          sink = new FileDataSink<T>(STDOUT_FILENO);

        } else {

          sink = new FileDataSink<T>(fname);

        }
      } catch (CErrnoException& err) {
        // If either of the above constructor throw,
        // then sink may not equal 0 but the memory will
        // have been freed. Ensure that this send out a
        // result that indicates failure
        sink = 0;
        throw CErrnoException(err);
      }

      return sink;
    }

  /**! Handle the construction of a ring data sink from a path
   *
   * On successful construction of a CRingDataSink, a pointer to
   * the dynamically allocated object will be passed to the caller.
   * The caller will own the object at this point.
   *
   * This may throw as a result of the constructor. 
   * 
   * \return pointer to a sink on success, 0 on failure 
   */
  template<class T>
    DataSink<T>* DataSinkFactory<T>::makeRingSink(std::string fname) 
    {

      DataSink<T>* sink=0;

      try {

        sink = new RingDataSink<T>(fname);

      } catch (CErrnoException& err) {
        // If either of the above constructor throw,
        // then sink may not equal 0 but the memory will
        // have been freed. Ensure that this send out a
        // result that indicates failure
        sink = 0;
        throw CErrnoException(err);
      }

      return sink;
    }


}  // end of NSCLDAQ namespace
