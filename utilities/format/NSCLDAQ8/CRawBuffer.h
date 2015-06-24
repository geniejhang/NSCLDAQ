#ifndef NSCLDAQ8_CRAWBUFFER_H
#define NSCLDAQ8_CRAWBUFFER_H

#include <CV8Buffer.h>
#include <ByteBuffer.h>
#include <DataFormatV8.h>

namespace DAQ
{
  namespace V8
  {
    class CRawBuffer : public CV8Buffer
    {
    private:
      bheader                     m_header;
      DAQ::Buffer::ByteBuffer     m_body;

    public:
      CRawBuffer();
      bheader  getHeader() const;
      void setHeader(const bheader& header) { m_header = header; }
      BufferTypes type() const { return GENERIC; }

      Buffer::ByteBuffer& getBody();
      const Buffer::ByteBuffer& getBody() const;

    };

  } // end of V8
} // end of DAQ

#endif // CRAWBUFFER_H
