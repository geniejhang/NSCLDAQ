#include "CControlBuffer.h"
#include <CRawBuffer.h>
#include <BufferPtr.h>
#include <stdexcept>

namespace DAQ {
  namespace V8 {
    
    CControlBuffer::CControlBuffer() : m_header(), m_title(), m_offset(), m_time()
    {
      setTitle(m_title);
    }

    CControlBuffer::CControlBuffer(const bheader &header, const std::string &title,
                                   std::uint32_t offset, const bftime &time)
      : m_header(header), m_title(), m_offset(offset), m_time(time)
    {
      setTitle(title);
    }

    CControlBuffer::CControlBuffer(const CRawBuffer &rawBuf)
      : m_header(rawBuf.getHeader()), m_title(), m_offset(), m_time()
    {
      Buffer::BufferPtr<std::uint8_t> p8(rawBuf.getBuffer().begin(), m_header.mustSwap());

      p8 += 16*sizeof(std::uint16_t); // skip over buffer header

      // create the title
      setTitle(std::string(p8, p8+80));

      // skip over the title
      p8 += 80;

      // the next piece is a 32-bit integer, extract it
      Buffer::BufferPtr<std::uint32_t> p32(p8);
      m_offset = *p32++;

      // the remainder are all 16-bit words, create 16-bit pointer, and
      // extract the data
      Buffer::BufferPtr<std::uint16_t> p16(p32);
      m_time.month  = *p16++;
      m_time.day    = *p16++;
      m_time.year   = *p16++;
      m_time.hours  = *p16++;
      m_time.min    = *p16++;
      m_time.sec    = *p16++;
      m_time.tenths = *p16++;
    }

    void CControlBuffer::toRawBuffer(CRawBuffer &buffer) const
    {

      Buffer::ByteBuffer tmpBuf;
      tmpBuf << m_header;
      tmpBuf << m_title; // need to accommodate the 80 character size
      tmpBuf << m_offset;
      tmpBuf << m_time;

      buffer.setBuffer(tmpBuf);

    }
    
    void CControlBuffer::setTitle(const std::string &title)
    {
      if (title.size() > 80) {
        throw std::runtime_error("CControlBuffer::CControlBuffer() title cannot exceed 80 characters");
      } else {
        m_title = title;
        m_title.resize(80, '\0');
      }

    }

    std::string CControlBuffer::getTitle() const
    {
      return m_title;
    }
  } // namespace V8
} // namespace DAQ
