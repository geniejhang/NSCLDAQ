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
      bool                 m_bytesNeededSwap;

    public:
      CRawBuffer(std::size_t size = gBufferSize);

      bheader  getHeader() const;
      BufferTypes type() const { return GENERIC; }
      void toRawBuffer(CRawBuffer &buffer) const;

      const Buffer::ByteBuffer& getBuffer() const;

      void setBuffer(const Buffer::ByteBuffer& buffer);

      bool bufferNeedsSwap() const { return m_bytesNeededSwap; }

    private:
      void parseHeader(const Buffer::ByteBuffer& buffer, bool swap);
    };

  } // end of V8
} // end of DAQ

#endif // CRAWBUFFER_H
