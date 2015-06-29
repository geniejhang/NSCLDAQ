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

    CTextBuffer::CTextBuffer(const bheader &header, const std::vector<std::string> &strings)
      : m_header(header), m_strings(strings)
    {}

    CTextBuffer::CTextBuffer(const CRawBuffer &rawBuf)
      : m_header(rawBuf.getHeader()), m_strings()
    {
      Buffer::BufferPtr<std::uint16_t> pSize(rawBuf.getBuffer().begin(),
                                             m_header.mustSwap());
      pSize += 16;

      auto beg = rawBuf.getBuffer().begin() + 16*sizeof(std::uint16_t);
      auto deadEnd = beg + *pSize;

      if (deadEnd > rawBuf.getBuffer().end()) {
        std::string errmsg("CTextBuffer::CTextBuffer(CRawBuffer const&) ");
        errmsg += "Inclusive size indicates more data exists than is valid.";

        throw std::runtime_error(errmsg);
      }
      beg += sizeof(std::uint16_t);

      auto begEnd = beg;

      while (beg != deadEnd) {
        begEnd = std::find(beg, deadEnd, '\0');
        m_strings.push_back(std::string(beg, begEnd));
        beg = begEnd+1;
      }

    }

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
    
  } // namespace V8
} // namespace DAQ
