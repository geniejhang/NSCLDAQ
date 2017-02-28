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


#ifndef CFILEDATASOURCE_H
#define CFILEDATASOURCE_H

#include "CDataSource.h"

#include <set>
#include <vector>
#include <stdint.h>
#include <string>
#include <vector>

// Forward class definitions:

class URL;
class CRingItem;
struct _RingItemHeader;

namespace DAQ {


/*!
  \brief Data source for a unix file descriptor 
  
  Because the data source supports reading from a standard file descriptor,
  it is useful for reading from a file or stdin. There is a big difference
  between the functionality of these two types of sources in terms of whether
  they are seekable. To support the difference, the class implements a buffer
  to hold data read through a peek operation. Even though this gives the 
  facade of not moving the get pointer in the file, it really will always read
  from the file descriptor. The user will get data from a previous peek operation
  if they trying to either peek or read since the previous peek operation.

*/

class CFileDataSource : public CDataSource
{
  // Private per-object data:

private:
  int                  m_fd;	       // File descriptor open on the event source.
  std::set<uint16_t>   m_exclude;    // item types to exclude from the return set.
  URL&                 m_url;	       // URI that points to the file.
  std::vector<char>    m_peekBuffer; // a buffer for storing data peeked at but not read
  bool                 m_lastReadWasPeek; // a flag to specify whether the last operation was a peek
  off_t                m_pos;        // position prior to the last peek operation 

  // Constructors and other canonicals:

public:
  CFileDataSource(URL& url,  std::vector<uint16_t> exclusionlist = std::vector<uint16_t>());
  CFileDataSource(const std::string& path, std::vector<uint16_t> exclusionList = std::vector<uint16_t>());
  CFileDataSource(int fd, std::vector<uint16_t> exclusionlist = std::vector<uint16_t>());
  virtual ~CFileDataSource();

private:
  CFileDataSource(const CFileDataSource& rhs);
  CFileDataSource& operator=(const CFileDataSource& rhs);
  int operator==(const CFileDataSource& rhs) const;
  int operator!=(const CFileDataSource& rhs) const;
public:

  // Mandatory interface:

  size_t peek(char* pBuffer, size_t nBytes);
  void ignore(size_t nBytes);
  size_t availableData() const;
  size_t tell() const;

  virtual CRingItem* getItem();

  void read(char* pBuffer, size_t nBytes);

  void setExclusionList(const std::set<uint16_t>& list);

  // utilities:

private:
  void       openFile(const std::string& fullPath);
  CRingItem* getItemFromFile();
  bool       acceptable(CRingItem* item) const;
  void       openFile();
  uint32_t   getItemSize(_RingItemHeader& header);
};


} // end DAQ
#endif
