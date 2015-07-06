#include "CScalerBuffer.h"
#include "CRawBuffer.h"
#include <ByteBuffer.h>
#include <BufferPtr.h>
#include <Deserializer.h>

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

      if (m_header.type != SCALERBF && m_header.type != SNAPSCBF) {
        std::string errmsg = "CScalerBuffer::CScalerBuffer(CRawBuffer const&) ";
        errmsg += "Buffer is not of type SCALERBF or SNAPSCBF!";
        throw std::runtime_error(errmsg);
      }

      Buffer::Deserializer<Buffer::ByteBuffer> buffer(rawBuffer.getBuffer(),
                                                      rawBuffer.bufferNeedsSwap());

      // skip the header b/c we obtained it from the RawBuffer already
      uint16_t discard;
      for (std::size_t i=0; i<16; ++i) buffer >> discard;

      buffer >> m_offsetEnd;
      buffer >> discard;
      buffer >> discard;
      buffer >> discard;
      buffer >> m_offsetBegin;
      buffer >> discard;
      buffer >> discard;
      buffer >> discard;

      std::uint32_t value;
      m_scalers.reserve(m_header.nevt);
      for (std::size_t index=0; index<m_header.nevt; ++index) {
        buffer >> value;
        m_scalers.push_back(value);
      }

    }

    CScalerBuffer::CScalerBuffer(const bheader &header, std::uint32_t offsetBegin,
                  std::uint32_t offsetEnd,
                  const std::vector<uint32_t> &scalers)
      : m_header(header),
        m_offsetBegin(offsetBegin),
        m_offsetEnd(offsetEnd), m_scalers(scalers)
    {}

    bheader CScalerBuffer::getHeader() const {
      return m_header;
    }

    void CScalerBuffer::toRawBuffer(CRawBuffer &buffer) const
    {
      vector<uint8_t> empty(6);

      bheader header = m_header;
      header.nwds = 16 + 6 + 2*m_scalers.size();
      header.nevt = m_scalers.size();

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
