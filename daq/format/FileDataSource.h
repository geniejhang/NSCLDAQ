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

#ifndef __FILEDATASOURCE_H
#define __FILEDATASOURCE_H

#ifndef __DATASOURCE_h
#include <DataSource.h>
#endif

#ifndef __STL_SET
#include <set>
#ifndef __STL_SET
#define __STL_SET
#endif
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


// Forward class definitions:

class URL;


namespace NSCLDAQ 
{

  /*!
    Provide a data source from an event file. The data source returns sequential data elements
    that are not in the excluded set of data types. This is intended for use with 
    the defined NSCLDAQ data types and is parameterized around those.

    This class deals with files as though they are file descriptors. For that reason,
    this works for reading from real files as well as stdin. To construct
  */
  template<class T> class FileDataSource : public DataSource<T>
  {

    // Private per-object data:
    private:
      int                  m_fd;	  // File descriptor open on the event source.
      std::set<uint16_t>   m_exclude; // item types to exclude from the return set.
      URL&                 m_url;	  // URI that points to the file.

      // Constructors and other canonicals:
    public:
      /*! \brief Construct from a URL
       *
       * The file referred to by the url should not be open already because this
       * will try to locate it and then open it for reading.
       *
       * \param url             universal resource locator referring to file
       * \param exclusionlist   list of types to exclude 
       */
      FileDataSource(URL& url,  std::vector<uint16_t> exclusionlist);

      /*! \brief Construct from a file descriptor
       *
       * The file descriptor passed into this should refer to a real file. For
       * that reason, the user should have already opened the file for reading.
       *
       * \param url             universal resource locator referring to file
       * \param exclusionlist   list of types to exclude 
       */
      FileDataSource(int fd,    std::vector<uint16_t> exclusionlist);
      virtual ~FileDataSource();

    private:
      FileDataSource(const FileDataSource& rhs);
      FileDataSource& operator=(const FileDataSource& rhs);
      int operator==(const FileDataSource& rhs) const;
      int operator!=(const FileDataSource& rhs) const;
    public:

      /*! \brief Read data item from the file
       *
       * This reads data elements from the file until one has been found that is not
       * of a type in the exclusion list. Once one is found, it is returend to the caller.
       *
       * \returns pointer to data element read from file (caller assumes ownership)
       */
      virtual T* getItem();


      // utilities:
    private:
      /*! \brief Retrieves an item from the file
       *
       *  \returns pointer to data element read from file (caller assumes ownership)
       *           or NULL if end of file or error
       */
      T* getItemFromFile();

      /*! \brief Determines whether a data elements is on the exclusions list or not
       *
       *  \param item   pointer to a data item
       */
      bool       acceptable(T* item) const;

      /*! \brief Open the file referred to by m_url for reading
       *
       **  \throws CErrnoException - For open failures
       **  \throws CInvalidArgumentException  - for protocols that are not file:
       */
      void       openFile();
  };

}

// include the implementation
#include <FileDataSource.hpp>

#endif
