#include "CRawBuffer.h"
#include <BufferPtr.h>

namespace DAQ
{
  namespace V8
  {


CRawBuffer::CRawBuffer(size_t size)
  : m_parsedHeader(), m_unparsedBuffer()
{
  m_unparsedBuffer.reserve(size);
}

bheader CRawBuffer::getHeader() const
{
  return m_parsedHeader;
}

//Buffer::ByteBuffer& CRawBuffer::getBuffer()
//{  return m_unparsedBuffer; }


const Buffer::ByteBuffer& CRawBuffer::getBuffer() const
{ return m_unparsedBuffer; }


void CRawBuffer::setBuffer(const Buffer::ByteBuffer &buffer)
{
  m_unparsedBuffer = buffer;

  Buffer::BufferPtr<uint16_t> p16(m_unparsedBuffer.begin(), false);
  Buffer::BufferPtr<uint32_t> p32 = p16;

  m_parsedHeader.nwds       = *p16++;
  m_parsedHeader.type       = BufferTypes(*p16++);
  m_parsedHeader.cks        = *p16++;
  m_parsedHeader.run        = *p16++;
  p32 = p16++;
  m_parsedHeader.seq        = *p32++;
  p16 = p32;
  m_parsedHeader.nevt       = *p16++;
  m_parsedHeader.nlam       = *p16++;
  m_parsedHeader.cpu        = *p16++;
  m_parsedHeader.nbit       = *p16++;
  m_parsedHeader.buffmt     = BufferVersion(*p16++);
  m_parsedHeader.ssignature = *p16++;
  p32 = p16++;
  m_parsedHeader.lsignature = *p32++;
  p16 = p32;
  m_parsedHeader.unused[0]  = *p16++;
  m_parsedHeader.unused[1]  = *p16++;

}

void CRawBuffer::toRawBuffer(CRawBuffer &buffer) const
{
  buffer.setBuffer(getBuffer());
}

  } // end of V8
} // end of DAQ
