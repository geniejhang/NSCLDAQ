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


#ifndef DAQ_CDATASOURCE_H
#define DAQ_CDATASOURCE_H

#include <cstddef> // for size_T
#include <memory>


namespace DAQ {

class CDataSource;
class CTimeout;

typedef std::unique_ptr<CDataSource> CDataSourceUPtr;
typedef std::shared_ptr<CDataSource> CDataSourcePtr;

/*! 
   Abstract base class that defines an interface that all dumper data sources 
   must meet.  Data sources provide data that can be dumped in a formatted
   way to the dumper's stdout.
*/

class CDataSource
{
  // constructors and canonicals.  In general most canonicals are not allowed.
  //
  bool m_eof;

private:
  CDataSource(const CDataSource& rhs);
  CDataSource& operator=(const CDataSource& rhs);
  int operator==(const CDataSource& rhs) const;
  int operator!=(const CDataSource& rhs) const;

public:
  CDataSource();
  virtual ~CDataSource();

  /*!
   * \brief Determine whether the end of file has been reached
   * \return boolean
   */
  bool eof() const { return m_eof; }

  /*!
   * \brief Clear the eof status
   */
  void clear() { setEOF(false); }

  virtual size_t availableData() const = 0;
  virtual void ignore(size_t nBytes) = 0;
  virtual size_t peek(char* pBuffer, size_t nBytes) = 0;
  virtual size_t tell() const = 0;

  /*!
   * \brief Read a block of data from the sink
   * \param pBuffer   block of data to copy data into
   * \param nBytes    number of bytes to read from the source
   */
  virtual void read(char* pBuffer, size_t nBytes);
  virtual void timedRead(char* pBuffer, size_t nBytes, const CTimeout& timeout) = 0;

protected:
  void setEOF(bool state) { m_eof = state; }
};

} // end DAQ

#endif
