#ifndef DAQ_TRANSFORM_CTRANSFORM10P0TO8P0_H
#define DAQ_TRANSFORM_CTRANSFORM10P0TO8P0_H


namespace NSCLDAQ10 {
  class CRingItem;
}

namespace DAQ {

  namespace V8 {
    class CRawBuffer;
    class CControlBuffer;
    class CScalerBuffer;
    class CPhysicsEventBuffer;
    class CTextBuffer;

    extern std::size_t gBufferSize;
  }

  namespace Transform {
    
    class CTransform10p0to8p0
    {
      using InitialType = NSCLDAQ10::CRingItem;
      using FinalType   = V8::CRawBuffer;

    public:
      CTransform10p0to8p0();

      FinalType operator()(const InitialType& item);

      V8::CControlBuffer transformStateChange(const InitialType& item);
      V8::CScalerBuffer transformIncrScaler(const InitialType& item);
      V8::CScalerBuffer transformNonIncrScaler(const InitialType& item);
      V8::CPhysicsEventBuffer transformPhysicsEvent(const InitialType& item);
      V8::CTextBuffer transformText(const InitialType& item);

    private:
      std::uint16_t mapControlType(std::uint16_t type) const;
    };
    
  } // namespace Transform
} // namespace DAQ

#endif // DAQ_TRANSFORM_CTRANSFORM10P0TO8P0_H
