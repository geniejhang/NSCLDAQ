#ifndef NSCLDAQ8_CSCALERBUFFER_H
#define NSCLDAQ8_CSCALERBUFFER_H

#include <CV8Buffer.h>
#include <bheader.h>

#include <vector>
#include <cstdint>

namespace DAQ
{
  namespace V8
  {

    class CRawBuffer;

    class CScalerBuffer : public CV8Buffer
    {
    private:
      bheader                    m_header;
      std::uint32_t              m_offsetBegin;
      std::uint32_t              m_offsetEnd;
      std::vector<std::uint32_t> m_scalers;

    public:
      CScalerBuffer();
      CScalerBuffer(const CRawBuffer& rawBuffer);
      CScalerBuffer(const bheader& header, std::uint32_t offsetBegin,
                    std::uint32_t offsetEnd,
                    const std::vector<std::uint32_t>& scalers);

      bheader getHeader() const;
      BufferTypes type() const { return SCALERBF; }

      void toRawBuffer(CRawBuffer &buffer) const;

      std::uint32_t getOffsetBegin() const;
      std::uint32_t getOffsetEnd() const;
      std::vector<std::uint32_t> getScalers() const;
    };

  } // end of V8
} // end of DAQ

#endif // CSCALERBUFFER_H
