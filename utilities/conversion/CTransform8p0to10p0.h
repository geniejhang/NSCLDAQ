#ifndef DAQ_TRANSFORM_CTRANSFORM8P0TO10P0_H
#define DAQ_TRANSFORM_CTRANSFORM8P0TO10P0_H

#include <ByteBuffer.h>
#include <NSCLDAQ10/CPhysicsEventItem.h>

#include <cstdint>
#include <vector>
#include <memory>
#include <ctime>


namespace DAQ {

  namespace V8 {
    struct bftime;
    class CRawBuffer;
    class ScalerBuffer;
    class CPhysicsEvent;

    extern std::size_t gBufferSize;
  }

  namespace V10 {
    class CRingItem;
    class CRingScalerItem;
    class CRingTextItem;
    class CRingStateChangeItem;
  }

  namespace Transform {
    
    class CTransform8p0to10p0
    {
    private:
      std::vector<V10::CPhysicsEventItem> m_physicsEvents;

    public:
      typedef typename DAQ::V8::CRawBuffer InitialType;
      typedef typename V10::CRingItem FinalType;

    public:
      CTransform8p0to10p0() {}

      FinalType operator()(const InitialType& type);

      V10::CRingScalerItem transformScaler(const InitialType& item);
      V10::CRingStateChangeItem transformControl(const InitialType& item);
      V10::CPhysicsEventItem transformPhysicsEvent(const InitialType &item);
      V10::CRingTextItem transformText(const InitialType& item);

      std::vector<V10::CPhysicsEventItem>& getRemainingEvents()
      {  return m_physicsEvents; }
      void transformOnePhysicsEvent(const std::shared_ptr<V8::CPhysicsEvent>& pEvent);

      std::time_t convertToTime_t(const V8::bftime& tstruct) const;
      std::uint16_t mapControlType(std::uint16_t type) const;

    };
    
  } // namespace Transform
} // namespace DAQ

#endif // DAQ_TRANSFORM_CTRANSFORM8P0TO10P0_H
