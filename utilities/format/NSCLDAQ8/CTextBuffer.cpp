#include "CTextBuffer.h"
#include <CRawBuffer.h>
#include <BufferPtr.h>
#include <algorithm>

namespace DAQ {
  namespace V8 {
    
    CTextBuffer::CTextBuffer()
      : m_header(), m_strings()
    {
    }

    CTextBuffer::CTextBuffer(const bheader &header,
                             const std::vector<std::string> &strings)
      : m_header(header), m_strings(strings)
    {}

    CTextBuffer::CTextBuffer(const CRawBuffer &rawBuf)
      : m_header(rawBuf.getHeader()), m_strings()
    {
      validateTypeToConstructFrom(); // throws if bad

      std::uint16_t totalBytes = extractTotalBytes(rawBuf.getBuffer());

      // locate beginning of body (i.e. skip the 16-word header)
      auto beg    = rawBuf.getBuffer().begin() + 16*sizeof(std::uint16_t);
      auto deadEnd = beg + totalBytes;

      validateDeadEndMeaningful(deadEnd, rawBuf.getBuffer().end());

      // Skip over the 16-bit total byte size
      beg += sizeof(std::uint16_t);

      parseStringsFromBuffer(beg, deadEnd);

    }

    CTextBuffer::CTextBuffer(const CTextBuffer &rhs)
      : m_header(rhs.m_header), m_strings(rhs.m_strings)
    {}

    void CTextBuffer::toRawBuffer(CRawBuffer &buffer) const
    {
      Buffer::ByteBuffer buf;
      buf << m_header;
      buf << totalBytes();
      for (auto& str : m_strings) {
        buf << str.c_str();
      }

      buffer.setBuffer(buf);
    }

    std::uint16_t CTextBuffer::totalBytes() const
    {
      std::uint16_t totalBytes = sizeof(std::uint16_t);

      for (auto& element : m_strings) {
        totalBytes += element.size() + 1;
      }

      return totalBytes;
    }
    
    std::uint16_t CTextBuffer::extractTotalBytes(const Buffer::ByteBuffer &buffer) const
    {
      Buffer::BufferPtr<std::uint16_t> pSize(buffer.begin(), m_header.mustSwap());
      pSize += 16; // skip over 16 shorts
      return *pSize;
    }

    void CTextBuffer::validateTypeToConstructFrom()
    {
      if (m_header.type != STATEVARBF && m_header.type != RUNVARBF
          && m_header.type != PKTDOCBF && m_header.type != PARAMDESCRIP) {

        std::string errmsg("CTextBuffer::CTextBuffer(CRawBuffer const&) ");
        errmsg += "Cannot construct from buffer type ";
        errmsg += std::to_string(m_header.type);

        throw std::runtime_error(errmsg);

      }
    }


    void CTextBuffer::validateDeadEndMeaningful(Buffer::ByteBuffer::const_iterator deadEnd,
                                                Buffer::ByteBuffer::const_iterator bufferEnd) {
      if (deadEnd > bufferEnd) {
        std::string errmsg("CTextBuffer::CTextBuffer(CRawBuffer const&) ");
        errmsg += "Inclusive size indicates more data exists than is valid.";

        throw std::runtime_error(errmsg);
      }
    }

    void CTextBuffer::parseStringsFromBuffer(Buffer::ByteBuffer::const_iterator beg,
                                             Buffer::ByteBuffer::const_iterator end) {
      auto begEnd = beg;
      while (beg != end) {

        begEnd = std::find(beg, end, '\0');
        m_strings.push_back(std::string(beg, begEnd));

        beg = begEnd+1;
      }

    }
  } // namespace V8
} // namespace DAQ
