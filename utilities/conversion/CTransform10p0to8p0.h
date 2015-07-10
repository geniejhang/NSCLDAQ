#ifndef DAQ_TRANSFORM_CTRANSFORM10P0TO8P0_H
#define DAQ_TRANSFORM_CTRANSFORM10P0TO8P0_H

#include <NSCLDAQ8/CPhysicsEventBuffer.h>
#include <NSCLDAQ8/CTextBuffer.h>
#include <cstdint>

namespace NSCLDAQ10 {
  class CRingItem;
}

namespace DAQ {

  namespace V8 {
    class CRawBuffer;
    class CControlBuffer;
    class CScalerBuffer;

    extern std::size_t gBufferSize;
  }

  namespace Transform {
    
    class CTransform10p0to8p0
    {
      using InitialType = NSCLDAQ10::CRingItem;
      using FinalType   = V8::CRawBuffer;

    private:
      std::uint32_t m_seq;
      std::uint16_t m_run;
      V8::CPhysicsEventBuffer              m_physicsBuffer;
      std::vector<V8::CTextBuffer>         m_textBuffers;

    public:
      CTransform10p0to8p0();

      FinalType operator()(const InitialType& item);

      V8::CControlBuffer transformStateChange(const InitialType& item);
      V8::CScalerBuffer transformIncrScaler(const InitialType& item);
      V8::CScalerBuffer transformNonIncrScaler(const InitialType& item);
      V8::CRawBuffer transformPhysicsEvent(const InitialType& item);
      V8::CRawBuffer transformText(const InitialType& item);

      const V8::CPhysicsEventBuffer& getCurrentPhysicsBuffer() const;
      const std::vector<V8::CTextBuffer>& getStagedTextBuffers() const;

      void clearStagedTextBuffers () { m_textBuffers.clear(); }

      std::uint16_t getCurrentRunNumber() const { return m_run; }

      void setSequenceNumber(std::uint32_t seqNo) { m_seq = seqNo; }

    private:
      V8::CPhysicsEventBuffer createNewPhysicsBuffer();
      void appendNewTextBuffer(std::uint16_t type);
      std::uint16_t mapControlType(std::uint16_t type) const;
      std::uint16_t mapTextType(std::uint16_t type) const;
    };
    
  } // namespace Transform
} // namespace DAQ

#endif // DAQ_TRANSFORM_CTRANSFORM10P0TO8P0_H
