#include "CScalerBuffer.h"
#include "CRawBuffer.h"
#include <ByteBuffer.h>
#include <BufferPtr.h>

using namespace std;

namespace DAQ
{
  namespace V8
  {
    CScalerBuffer::CScalerBuffer()
      : CScalerBuffer( bheader(), 0, 0, vector<uint32_t>())
    {
    }

    CScalerBuffer::CScalerBuffer(const CRawBuffer &rawBuffer)
      : m_header(rawBuffer.getHeader()),
        m_offsetBegin(),
        m_offsetEnd(),
        m_scalers()
    {
      // need to skip over the 32-byte buffer header
      auto bodyBegin = rawBuffer.getBuffer().begin() + 16*sizeof(std::uint16_t);
      Buffer::BufferPtr<uint8_t>  p8(bodyBegin, m_header.mustSwap());

      Buffer::BufferPtr<uint32_t> p32 = p8;
      m_offsetEnd = *p32;
      p8 += 10;

      p32 = p8;
      m_offsetBegin = *p32;
      p8 += 10;

      p32 = p8;

      m_scalers.reserve(m_header.nevt);
      for (std::size_t index=0; index<m_header.nevt; ++index) {
        m_scalers.push_back(*p32++);
      }


    }

    CScalerBuffer::CScalerBuffer(const bheader &header, std::uint32_t offsetBegin,
                  std::uint32_t offsetEnd,
                  const std::vector<uint32_t> &scalers)
      : m_header(header), m_offsetBegin(offsetBegin),
        m_offsetEnd(offsetEnd), m_scalers(scalers)
    {}

    bheader CScalerBuffer::getHeader() const {
      return m_header;
    }

    void CScalerBuffer::toRawBuffer(CRawBuffer &buffer) const
    {
      vector<uint8_t> empty(6);

      Buffer::ByteBuffer newbuf;
      newbuf << m_header;
      newbuf << m_offsetEnd;
      newbuf << empty;
      newbuf << m_offsetBegin;
      newbuf << empty;
      newbuf << m_scalers;

      buffer.setBuffer(newbuf);
    }

    std::uint32_t CScalerBuffer::getOffsetBegin() const
    {
      return m_offsetBegin;
    }

    std::uint32_t CScalerBuffer::getOffsetEnd() const
    {
      return m_offsetEnd;
    }

    std::vector<std::uint32_t> CScalerBuffer::getScalers() const
    {
      return m_scalers;
    }
  } // end of V8
} // end of DAQ
