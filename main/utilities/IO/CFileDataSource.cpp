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
#include <CRingItem.h>
#include <DataFormat.h>
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
            size_t nBytesRead = io::readData(m_fd,
                                             m_peekBuffer.data() + previousSize,
                                             nBytes - previousSize);

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
        nBytesToCopy = io::readData(m_fd, m_peekBuffer.data(), nBytes);
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

/*!
  Provide the caller with the next item from the ring source.
  This is done by getting the ring item from the file and then ensuring
  that it is acceptable.  The first acceptable item is returned to the caller.
  
  \return CRingItem*
  \retval NULL - end of file reached without an acceptable item being found.
  \retval other - Pointer to a dynamically allocated CRingItem that is the
                  next acceptable item from the ring.  The caller is responsible for
		  deleting this storage when done with it.
*/
CRingItem*
CFileDataSource::getItem()
{
  while (1) {
    CRingItem* pItem = getItemFromFile();
    if (!pItem) {
      return pItem;
    }
    if (acceptable(pItem)) {
      return pItem;
    }
    // Skip the item, it's not acceptable.

    delete pItem;
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
void CFileDataSource::read(char* pBuffer, size_t nBytes)
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
                size_t nRead = io::readData(m_fd, pBuffer, nToRead);

                if (nRead != nToRead) {
                    setEOF(true);
                }
            }

        }
    } else {


        if (! eof() ) {
            size_t nRead = io::readData(m_fd, pBuffer, nBytes);

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
**  Get the next item from the file:
**  First we fetch the header.  Using that we can create storage of the right size
**  and read the item into that.
**  We then allocate a ring item and fill its body in from the file.
**  Returns:
**    Pointer to the item read from file or NULL if we hit the end of file or an error.
*/
CRingItem*
CFileDataSource::getItemFromFile()
{
  // First read a header:

  RingItemHeader header;

  int nRead = io::readData(m_fd, &header, sizeof(header));
  if (nRead != sizeof(header)) {
    return reinterpret_cast<CRingItem*>(NULL);
  }
  uint32_t itemsize = getItemSize(header);
  uint32_t bodysize = itemsize - sizeof(header);

  // Read the remainder of the data:

  uint8_t* pBody = new uint8_t[bodysize];
  nRead          = io::readData(m_fd, pBody, bodysize);
  if (nRead != bodysize) {
    delete []pBody;
    return reinterpret_cast<CRingItem*>(NULL);
  }

  CRingItem* pItem = new CRingItem(1, itemsize); //  Type will get overwritten:


  // The ring item is filled in this way to preserve the initial byte order.

  pRingItemHeader pStorage = reinterpret_cast<pRingItemHeader>(pItem->getItemPointer());
  memcpy(pStorage, &header, sizeof(RingItemHeader));
  memcpy(pStorage+1, pBody, bodysize);

  delete []pBody;

  // Set the cursor:

  char* pCursor = reinterpret_cast<char*>(pStorage+1); // start of body.
  pCursor      += bodysize;	                       // end of body.
  pItem->setBodyCursor(pCursor);

  return pItem;
  
}
/*
** Determines if an item is acceptable.
**
** Parameters:
**   item - Pointer to the item.
** Returns:
**   True if acceptable, false if not.
*/
bool
CFileDataSource::acceptable(CRingItem* item) const
{
  uint16_t type             = item->type(); // Byte swaps as needed.
  set<uint16_t>::iterator i = m_exclude.find(type);
  return (i == m_exclude.end()); // Not found is good.
}
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
/*
**  Return the size of an item.  This does the right thing in the presence
**  of a creating system that is byte backwards than the executing system.
**
** Parameters:
**    header - refers to the item header to analyze.
** Returns:
**    number of bytes in the item.
*/
uint32_t
CFileDataSource::getItemSize(RingItemHeader& header)
{
  uint32_t size = header.s_size;
  uint32_t type = header.s_type;
  
  // If necessary, byte swap the size:

  if ((type & 0xffff0000) != 0) {
    union {
      uint8_t  bytes[4];
      uint32_t l;
    } lsize;
    lsize.l = size;
    size = 0;
    for (int i=0; i < 4; i++) {
      size |= (lsize.bytes[i] << 4*(3-i));
    }
  }


  return size;
}


} // end DAQ
