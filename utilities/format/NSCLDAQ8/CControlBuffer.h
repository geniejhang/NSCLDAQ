#ifndef DAQ_V8_CCONTROLBUFFER_H
#define DAQ_V8_CCONTROLBUFFER_H

#include <CV8Buffer.h>
#include <bheader.h>

#include <string>
#include <cstdint>

namespace DAQ {
  namespace V8 {
    
    class CRawBuffer;

    class CControlBuffer : public CV8Buffer
    {
    private:
      bheader       m_header;
      std::string   m_title;
      std::uint32_t m_offset;
      bftime        m_time;

    public:
      CControlBuffer(const bheader& header, const std::string& title,
                     std::uint32_t offset, const bftime& time);
      CControlBuffer(const CRawBuffer& rawBuf);


      bheader getHeader() const { return m_header; }
      BufferTypes type() const { return m_header.type; }
      void toRawBuffer(CRawBuffer &buffer) const;

      void setTitle(const std::string& title);
      std::string getTitle() const;

      bftime getTimeStruct() const { return m_time; }
      void setTimeStruct(const bftime& time) { m_time = time; }

      std::uint32_t getOffset() const { return m_offset; }
      void setOffset(std::uint32_t offset) { m_offset = offset; }

    }; // end of CControlBuffer

  } // namespace V8
} // namespace DAQ

#endif // DAQ_V8_CCONTROLBUFFER_H
