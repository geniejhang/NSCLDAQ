#ifndef DAQ_V8_CTEXTBUFFER_H
#define DAQ_V8_CTEXTBUFFER_H


#include <CV8Buffer.h>
#include <bheader.h>
#include <DataFormatV8.h>

#include <vector>
#include <string>

namespace DAQ {
  namespace V8 {
    
    class CRawBuffer;

    class CTextBuffer : public CV8Buffer
    {
      bheader                  m_header;
      std::vector<std::string> m_strings;

    public:
      CTextBuffer();
      CTextBuffer(const bheader& header, const std::vector<std::string>& strings);
      CTextBuffer(const CRawBuffer& rawBuf);

      CTextBuffer(const CTextBuffer& rhs);

      ~CTextBuffer() {}

      bheader getHeader() const { return m_header; }
      BufferTypes type() const { return BufferTypes(m_header.type); }
      void toRawBuffer(CRawBuffer &buffer) const;

      std::vector<std::string>& getStrings() {return m_strings; }
      std::vector<std::string> getStrings() const { return m_strings; }

      std::uint32_t totalBytes() const;
      std::uint16_t totalShorts() const;

    private:
      void validateTypeToConstructFrom();
      void validateDeadEndMeaningful(Buffer::ByteBuffer::const_iterator deadEnd,
                                     Buffer::ByteBuffer::const_iterator bufferEnd);
      std::uint16_t extractTotalShorts(const Buffer::ByteBuffer& buffer, bool needsSwap) const;
      void parseStringsFromBuffer(Buffer::ByteBuffer::const_iterator beg,
                                  Buffer::ByteBuffer::const_iterator end);
      Buffer::ByteBuffer::const_iterator skipNullCharPadding(Buffer::ByteBuffer::const_iterator beg,
                                                             Buffer::ByteBuffer::const_iterator end);
    };
    
  } // namespace V8
} // namespace DAQ

#endif // DAQ_V8_CTEXTBUFFER_H
