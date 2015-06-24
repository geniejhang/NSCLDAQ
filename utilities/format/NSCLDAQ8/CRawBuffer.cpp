#include "CRawBuffer.h"

namespace DAQ
{
  namespace V8
  {


CRawBuffer::CRawBuffer()
  : m_header(), m_body()
{
}

bheader CRawBuffer::getHeader() const
{
  return m_header;
}

Buffer::ByteBuffer& CRawBuffer::getBody()
{  return m_body; }


const Buffer::ByteBuffer& CRawBuffer::getBody() const
{ return m_body; }

  } // end of V8
} // end of DAQ
