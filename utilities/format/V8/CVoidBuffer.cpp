#include "V8/CVoidBuffer.h"

namespace DAQ {
  namespace V8 {
    
    CVoidBuffer::CVoidBuffer()
      : m_header()
    {
    }

    void CVoidBuffer::toRawBuffer(CRawBuffer &rawBuf) const
    {
      // do nothing.
    }
    
  } // namespace V8
} // namespace DAQ
