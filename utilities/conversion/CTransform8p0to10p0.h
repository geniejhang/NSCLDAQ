#ifndef DAQ_TRANSFORM_CTRANSFORM8P0TO10P0_H
#define DAQ_TRANSFORM_CTRANSFORM8P0TO10P0_H

#include <ByteBuffer.h>
#include <cstdint>

namespace NSCLDAQ10 {
  class CRingItem;
  class CRingScalerItem;
  class CRingTextItem;
  class CPhysicsEventItem;
  class CRingStateChangeItem;
}

namespace DAQ {

  namespace V8 {
    class CRawBuffer;
    class ScalerBuffer;

    std::size_t gBufferSize = 8192;
  }

  namespace Transform {
    
    class CTransform8p0to10p0
    {
    public:
      typedef typename DAQ::V8::CRawBuffer InitialType;
      typedef typename NSCLDAQ10::CRingItem FinalType;

    public:
      CTransform8p0to10p0(std::size_t bufferSize) {
        DAQ::V8::gBufferSize = bufferSize;
      }

      FinalType operator()(const InitialType& type);

      NSCLDAQ10::CRingScalerItem transformScaler(const InitialType& item);
      NSCLDAQ10::CRingStateChangeItem transformControl(const InitialType& item);
      NSCLDAQ10::CPhysicsEventItem transformPhysicsEvent(const InitialType &item);
      NSCLDAQ10::CRingTextItem transformText(const InitialType& item);

    };
    
  } // namespace Transform
} // namespace DAQ

#endif // DAQ_TRANSFORM_CTRANSFORM8P0TO10P0_H
