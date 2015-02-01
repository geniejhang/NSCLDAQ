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

#ifndef _FILEDATASINK_H
#define _FILEDATASINK_H

#include "DataSink.h"
#include <unistd.h>
#include <errno.h>
#include <string>
#include <CErrnoException.h>


namespace NSCLDAQ 
{

///! \brief A "file" data sink
  /*!
   *   Owns and manages a general file object. The user should
   *   prefer constructing from a filename rather than a file
   *   descriptor because this reduces the risk for leaking a 
   *   file. Because this deals with actual file descriptors,
   *   the class will work properly with both files and stdin.
   *
   *   This is a template class whose parameter should be a 
   *   standard 
   *
   */
  template<class T> class FileDataSink : public DataSink<T>
  {
    private: 
      int m_fd;  ///!< The file descriptor

    public:
      /*! \brief Constructor from a file descriptor
       * 
       * Attaches to an already open file.
       *
       * \param fd  file descriptor
      */
      FileDataSink (int fd);    


      /*! \brief Opens file from a path
       *
       * \param path  the path to the file
       */
      FileDataSink (std::string path);    

      /**! Destructors
      */
      virtual ~FileDataSink ();    

    private:
      // Copy and assignment are not sensible b/c ownership
      // of the file becomes ambiguous
      FileDataSink(const FileDataSink&);
      FileDataSink& operator=(const FileDataSink&);


      // Required interface
    public:

      /*! \brief Write a data item into the sink
       *
       * \param item  the data element to write
       */
      virtual void putItem(const T& item);

      /*! \brief Write an arbitrary amount of data to the sink
       *
       * \param pData   pointer to the data 
       * \param nBytes  number of bytes to write (starting at pData)
       */
      virtual void put(const void* pData, size_t nBytes);

      /*! \brief Flush file to synchronize
       *
       * \throws CErrnoException - failed to flush.
       */
      void flush()
      { 
        int retval = fsync(m_fd); 
        if (retval<0) {
          throw CErrnoException("FileDataSink::flush() failed");
        }
      }

      // Private utilities
    private:
      /*! \brief Test whether the file is writable  */
      bool isWritable();

  };

} // end of NSCLDAQ namespace 

// include the implementation
#include <FileDataSink.hpp>

#endif

