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


#include <io.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <iostream>

#include <DataFormat.h>
#include <CRingItem.h>

namespace NSCLDAQ 
{

  /**! Construct from a file descriptor
   * Ownership of this file descriptor is transferred to the
   * this object. Write operations on the file descriptor must 
   * be permissible or an exception is thrown.
   *
   * \param a file descriptor to define the sink
   * \throw std::string
   */
  template<class T>
    FileDataSink<T>::FileDataSink(int fd)
    : m_fd(fd)
    {
      if (!isWritable()) {
        throw std::string("FileDataSink::FileDataSink(int) file descriptor is not write only");
      }

    }


  /**! Construct from a file name
   * Obtains a file descriptor given a valid pathname. If file doesn't exist
   * a new file is opened with RDWR permissions. If the file 
   * doesn't exist, it is created with read/write permissions. 
   *
   * \param a file descriptor to define the sink
   *
   * \throw int on failure opening file
   * \throw std::string if file is not writable
   *
   * TODO:  Open failures might be best signalled with CErrnoException or,
   *        if not, the string should at least have strerror in it for the
   *        errno so the user can know why the file could not be opened.
   */
  template<class T>
    FileDataSink<T>::FileDataSink(std::string fname)
    : m_fd(-1)
    {

      // Open or create if the file doesn't exist
      m_fd = open(fname.c_str(),O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
      // check to see if failed
      if (m_fd==-1) {
        std::string errmsg("FileDataSink::FileDataSink(std::string)");
        errmsg += " failed to open file ";
        errmsg += fname;
        throw CErrnoException(errmsg);
      }

      if (!isWritable()) {
        throw std::string("FileDataSink::FileDataSink(std::string) file descriptor is not write only");
      }

    }

  /**! Close file
   * 
   * If the file descriptor does not refer to STDOUT_FILENO and
   * and it points to a valid file, then close it.
   *
   * 
   */
  template<class T>
    FileDataSink<T>::~FileDataSink()
    {

      // Can't close stdout
      if (m_fd!=STDOUT_FILENO && m_fd>0) {
        close(m_fd);

      }

    }


  /**! Write ring item to the sink
   *
   * Writes the data to the file. This delegates the writing to
   * a static function io::writeData(int,void*,int).
   *
   * \param item refers to a CRingItem that contains the data
   *
   *
   * \throw CErrnoException when io failure
   *
   */
  template<class T>
    void FileDataSink<T>::putItem(const T& item)
    {
      // write the object 
      put(&item, sizeof(item));
    }

  /**
   * put
   *    Puts an arbitrary chunk of data to the sink (file)
   *
   *   @param pData - pointer to the buffer containing the data.
   *   @param nBytes - Number of bytes of data to put.
   *
   * @note the underlying implemenation is just io::writeData.  It's int exception
   *        is converted to an errno exception
   * @throw CErrnoException
   * 
   */
  template<class T>
    void FileDataSink<T>::put(const void* pData, size_t nBytes)
    {
      try {
        io::writeData(m_fd, pData, nBytes);
      } catch (int err) {
        errno = err;                    // CErrnoException captures the global errno.
        std::string errmsg("FileDataSink::putItem(const CRingItem&)"); 
        errmsg += " : writeData failed ";

        throw CErrnoException(errmsg);
      }
    }


  /**! Check if write operates are allowed on file
   *
   * \throw CErrnoException if fcntl failed while checking
   */
  template<class T>
    bool FileDataSink<T>::isWritable() 
    {
      // Get the status flags of the file
      int status = fcntl(m_fd,F_GETFL);

      if (status<0) {
        std::string errmsg ("FileDataSink::isWritable()");
        errmsg += " failed checking file status flags";
        throw CErrnoException(errmsg);
      }; 

      // Check if we can write
      return ( (status&O_WRONLY)!=0 || (status&O_RDWR)!=0 );

    }

  // BEGIN SPECIALIZATIONS
 
  template<>
    inline void FileDataSink<CRingItem>::putItem(const CRingItem& item) 
    {
      // Get the underlying structure containing the state 
      const RingItem* pItem = item.getItemPointer();

      // Set up variable for writing it to stream
      size_t nbytes = pItem->s_header.s_size;
      put(pItem, nbytes);
    }



} // end of NSCLDAQ namespace
