#ifndef __FILEDATASOURCE_H
#define __FILEDATASOURCE_H
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
  Provide a data source from an event file.  This allows users to directly dump
  an event file to stdout.  The data source returns sequential ring items
  that are not in the excluded set of data types.

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
      FileDataSource(URL& url,  std::vector<uint16_t> exclusionlist);
      FileDataSource(int fd,    std::vector<uint16_t> exclusionlist);
      virtual ~FileDataSource();

    private:
      FileDataSource(const FileDataSource& rhs);
      FileDataSource& operator=(const FileDataSource& rhs);
      int operator==(const FileDataSource& rhs) const;
      int operator!=(const FileDataSource& rhs) const;
    public:

      // Mandatory interface:
      virtual T* getItem();

      // utilities:

    private:
      T* getItemFromFile();
      bool       acceptable(T* item) const;
      void       openFile();
  };

}

#include <FileDataSource.hpp>

#endif
