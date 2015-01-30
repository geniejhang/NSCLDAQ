
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
/**!
*   Owns and manages a general file object. The user should
*   prefer constructing from a filename rather than a file
*   descriptor because this reduces the risk for leaking a 
*   file.
*/
  template<class T> class FileDataSink : public DataSink<T>
  {
    private: 
      int m_fd;  ///!< The file descriptor

    public:
      /**! Constructors
      */
      FileDataSink (int fd);    
      FileDataSink (std::string pathname);    

      /**! Destructors
      */
      virtual ~FileDataSink ();    

    private:
      // Copy and assignment are not sensible b/c ownership
      // of the file becomes ambiguous
      FileDataSink(const FileDataSink&);
      FileDataSink& operator=(const FileDataSink&);


    public:

      /*
       *  Implementation of the required interface methods
       */
      virtual void putItem(const T& item);
      virtual void put(const void* pData, size_t nBytes);

      /**! Flush file to syncronize
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
      bool isWritable();

  };

} // end of NSCLDAQ namespace 

// include the implementation
#include <FileDataSink.hpp>

#endif

