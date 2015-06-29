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
      bheader              m_parsedHeader;
      Buffer::ByteBuffer   m_unparsedBuffer;

    public:
      CRawBuffer(std::size_t size);

      bheader  getHeader() const;
      BufferTypes type() const { return GENERIC; }
      void toRawBuffer(CRawBuffer &buffer) const;

//      Buffer::ByteBuffer& getBuffer();
      const Buffer::ByteBuffer& getBuffer() const;

      void setBuffer(const Buffer::ByteBuffer& buffer);
    };

  } // end of V8
} // end of DAQ

#endif // CRAWBUFFER_H
