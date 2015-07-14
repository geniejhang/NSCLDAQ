#ifndef DAQ_V8_CVOIDBUFFER_H
#define DAQ_V8_CVOIDBUFFER_H

#include <V8/DataFormatV8.h>
#include <V8/CV8Buffer.h>

namespace DAQ {
  namespace V8 {
    
    class CVoidBuffer : public CV8Buffer
    {
      bheader m_header;

    public:
      CVoidBuffer();

      bheader getHeader() const { return m_header; }

      BufferTypes type() const { return VOID; }

      void toRawBuffer(CRawBuffer& rawBuf) const;

    };
    
  } // namespace V8
} // namespace DAQ

#endif // DAQ_V8_CVOIDBUFFER_H
