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

      ~CTextBuffer() {}

      bheader getHeader() const { return m_header; }
      BufferTypes type() const { return m_header.type; }
      void toRawBuffer(CRawBuffer &buffer) const;

      std::vector<std::string>& getStrings() {return m_strings; }
      std::vector<std::string> getStrings() const { return m_strings; }

      std::uint16_t totalBytes() const;
    };
    
  } // namespace V8
} // namespace DAQ

#endif // DAQ_V8_CTEXTBUFFER_H
