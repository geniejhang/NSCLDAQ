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


#include <config.h>
#include "CFileDataSource.h"


#include <URL.h>
#include <CTimeout.h>
#include <ErrnoException.h>
#include <CInvalidArgumentException.h>
#include <io.h>

#include <string>
#include <stdexcept>
#include <limits>

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using std::vector;
using std::set;
using std::string;

namespace DAQ {

/////////////////////////////////////////////////////////////////////////////////////////////
//
//  Constructors and implemented canonicals:


/*!
  The constructor must:
  - Decode the URL to a filename and open the file (see openFile).
  - Save a copy of the URL just for the hell of it.

  \param url           - Specifies the file to open.
  \param exclusionList - Specifies the list of item types to not return to the caller.

*/
CFileDataSource::CFileDataSource(URL& url, vector<uint16_t> exclusionList) :
  m_fd(-1),
  m_url(*(new URL(url))),
  m_lastReadWasPeek(false),
  m_pos(0)
{
  for (int i=0; i < exclusionList.size(); i++) {
    m_exclude.insert(exclusionList[i]);
  }
  openFile();			// May throw exceptions.
}


CFileDataSource::CFileDataSource(const std::string& path, vector<uint16_t> exclusionList) :
  m_fd(-1),
  m_url(*(new URL(string("file://") + path))),
  m_lastReadWasPeek(false),
  m_pos(0)

{
  for (int i=0; i < exclusionList.size(); i++) {
    m_exclude.insert(exclusionList[i]);
  }
  openFile(path);			// May throw exceptions.
}



/**
 * construtor from fd:
 */
CFileDataSource::CFileDataSource(int fd, vector<uint16_t> exclusionlist) :
  m_fd(fd),  
  m_url(*(new URL("file://stdin/junk"))), 
  m_lastReadWasPeek(false),
  m_pos(0)
{
  for (int i=0; i < exclusionlist.size(); i++) {
    m_exclude.insert(exclusionlist[i]);
  }
}

/*!
   The destructor must close the file (ignoring errors so that if it's open
   nothing happens).
   The url must be deleted as well.
*/
CFileDataSource::~CFileDataSource()
{
  delete &m_url;
  close(m_fd);
}
/////////////////////////////////////////////////////////////////////////////////////////
//
//  Mandatory interface:


/*! \brief Return the amount of data available for reading
 *
 * The return value depends largely on whether the file data source is governing
 * a file descriptor referring to a seekable or non-seekable entity (i.e. stdin).
 *
 * \retval numeric_limits<size_t>::max() if fd == STDIN_FILENO
 * \retval number of bytes to end of file otherwise
 */
size_t CFileDataSource::availableData() const
{
    size_t nBytes = 0;
    if ( m_fd != STDIN_FILENO ){
        // read current position and size of file
        // return the difference
        off_t currentPosition = lseek(m_fd, 0, SEEK_CUR);
        off_t endPos = lseek(m_fd, 0, SEEK_END);
        off_t status = lseek(m_fd, currentPosition, SEEK_SET);

        nBytes = (endPos - currentPosition);
    } else {
        nBytes = std::numeric_limits<size_t>::max();
    }
    return nBytes;
}

/*!
 * \brief Read data from the file while giving the illusion that the get pointer stays fixed
 *
 *  Because stdin is not seekable, this can't be implemented using a basic read
 *  operation followed by a seek. Instead, this has to read data into a "peek buffer"
 *  that holds onto data for subsequent read or peek operations.
 *
 *  
 *  \param pBuffer    the buffer to fill with the new data
 *  \param nBytes     the number of bytes to read
 *
 *  \returns the number of bytes actually read
 *
 */
size_t CFileDataSource::peek(char* pBuffer, size_t nBytes)
{
    int nBytesToCopy = 0;
    size_t previousSize = 0;

    if (m_lastReadWasPeek) {

        // we have data in the peek buffer
        previousSize = m_peekBuffer.size();

        if (previousSize < nBytes) {
            m_peekBuffer.resize(nBytes);
            size_t nBytesRead = io::timedReadData(m_fd,
                                             m_peekBuffer.data() + previousSize,
                                             nBytes - previousSize,
                                             CTimeout(0));

            if (nBytesRead < 0) {
                std::string msg("CFileDataSource::peek() ");
                msg += strerror(errno);
                throw std::runtime_error(msg);
            } else {
                nBytesToCopy = previousSize + nBytesRead;
            }
        } else {
            nBytesToCopy = nBytes;
        }
    } else {
        // store our file positio
        m_pos = tell();

        // no data in peek buffer... set it up and then read into it
        m_peekBuffer.resize(nBytes);
        nBytesToCopy = io::timedReadData(m_fd, m_peekBuffer.data(), nBytes, CTimeout(0));
    }

    // copy from our peek buffer into the user's
    std::copy(m_peekBuffer.begin(), m_peekBuffer.begin() + nBytesToCopy, pBuffer);
    m_lastReadWasPeek = true;

    return nBytesToCopy;
}

/*! \brief Ignore (i.e. skip) the next nBytes available in the source
 *
 * \param nBytes  the number of bytes to skip
 *
 * This gives the illusion of simply moving the get pointer forward.
 */
void CFileDataSource::ignore(size_t nBytes)
{
    std::vector<char> tempBuffer;
    size_t nBytesInPeekBuffer = m_peekBuffer.size();
    if (m_lastReadWasPeek) {

        // First ignore the data that was last read in a peek operation
        //
        // 2 situations need to be addressed:
        // 1. User want to ignore more than the last peek
        // 2. User wants to ignore less than the last peek
        if (nBytesInPeekBuffer >= nBytes) {
            // user wants to ignore fewer bytes than are in the peek buffer
            // remove the bytes that they want to ignore
            size_t nBytesToErase = std::min(nBytes, nBytesInPeekBuffer);
            m_peekBuffer.erase(m_peekBuffer.begin(), m_peekBuffer.begin()+nBytesToErase);

            // update our file position
            m_pos += nBytesToErase;

        } else {
            // user wants to ignore more bytes than were in the peek buffer

            // empty out the peek buffer
            m_peekBuffer.clear();
            m_lastReadWasPeek = false;

            // resize our vector to store the new data we need to read...
            // remember, some of what they wanted to ignore has already been
            // ignored... we just need to ignore the remainder
            tempBuffer.resize(nBytes - nBytesInPeekBuffer);

            read(tempBuffer.data(), tempBuffer.size());
        }
    } else {
        // there is nothing in the peek buffer to care about... just ignore
        // the next nBytes by reading them.
        tempBuffer.resize(nBytes);
        read(tempBuffer.data(), nBytes);
    }
}

/*!
 *  \returns the current position of the get pointer
 *
 *  Of course this is referrring to a "virtual" get pointer because
 *  a peek operation actually moves the get pointer. 
 */
size_t CFileDataSource::tell() const
{
    if (m_lastReadWasPeek) {
        return m_pos;
    } else {
        return lseek(m_fd, 0, SEEK_CUR);
    }
}


/*! \brief Read the next nBytes from the file
 *
 * \param pBuffer   the buffer to read data into
 * \param nBytes    the number of bytes to read
 * 
 * Here we have once again to deal with the complications of the 
 * peek buffer. We first read data from the peek buffer and then 
 * from the file. Depending on how much data is in the peek buffer
 * compared to the number of bytes requested, the may or may not
 * be interaction with the underlying file.
 *
 */
void CFileDataSource::timedRead(char* pBuffer, size_t nBytes, const CTimeout& timeout)
{
    if (m_lastReadWasPeek) {

        if (nBytes <= m_peekBuffer.size()) {
            // the user asked for less data than is in the peek buffer... copy over the
            // data itno the user's buffer and return... no IO needs to take place
            std::copy(m_peekBuffer.begin(), m_peekBuffer.begin()+ nBytes, pBuffer);
            m_peekBuffer.erase(m_peekBuffer.begin(), m_peekBuffer.begin()+nBytes);

            if (m_peekBuffer.size() > 0) {
                m_lastReadWasPeek = true;
            } else {
                m_lastReadWasPeek = false;
            }
        } else {
            // the user asked for more data than was in the peek buffer
            // copy over what was in the peek buffer, then read the rest
            size_t nBytesInPeek = m_peekBuffer.size();

            pBuffer = std::copy(m_peekBuffer.begin(), m_peekBuffer.end(), pBuffer);
            m_peekBuffer.clear();

            m_lastReadWasPeek = false;

            // read the rest
            if (! eof() ) {
                size_t nToRead = nBytes-nBytesInPeek;
                size_t nRead = io::timedReadData(m_fd, pBuffer, nToRead, timeout);

                if (nRead != nToRead && !timeout.expired()) {
                    setEOF(true);
                }
            }
        }
    } else {


        if (! eof() ) {
            size_t nRead = io::timedReadData(m_fd, pBuffer, nBytes, timeout);

            if (nRead != nBytes) {
                setEOF(true);
            }
        }
    }
}

void CFileDataSource::setExclusionList(const std::set<uint16_t>& list)
{
    m_exclude = list;
}


//////////////////////////////////////////////////////////////////////////////////////////
//
// Private utilties.

/*
** Opens the file that corresponds to the URL m_url.
** The resulting fd is placed in m_fd.
** errors are reported via exceptions which include:
**  CErrnoException - For open failures
**  CInvalidArgumentException  - for protocols that are not file:
**
*/
void CFileDataSource::openFile(const string& fullPath)
{
    m_fd = open(fullPath.c_str(), O_RDONLY);
    if (m_fd == -1) {
      throw CErrnoException("Opening file data source");
    }

    m_pos = tell();
}

void
CFileDataSource::openFile()
{
  string p = m_url.getProto();
  if (p != string("file")) {
    throw CInvalidArgumentException(string(m_url), "A file URL only",
				    "Opening a file data source");
  }
  string fullPath= m_url.getPath();


  openFile(fullPath);
}


} // end DAQ
