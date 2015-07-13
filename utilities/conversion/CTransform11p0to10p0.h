#ifndef CTRANSFORM11P0TO10P0_H
#define CTRANSFORM11P0TO10P0_H



namespace DAQ {

  namespace V10
  {
      class CRingItem;
      class CRingScalerItem;
      class CRingTimestampedRunningScalerItem;
      class CRingStateChangeItem;
      class CPhysicsEventItem;
      class CRingPhysicsEventCountItem;
      class CRingTextItem;
      class CRingFragmentItem;
      class CUnknownFragment;
  }

  namespace V11 {
      class CRingItem;
  }

  namespace Transform {

    class CTransform11p0to10p0
    {
    public:
      typedef typename V11::CRingItem InitialType;
      typedef typename V10::CRingItem FinalType;

    public:
      FinalType operator()(InitialType& item);
      FinalType dispatch(InitialType& item);

      FinalType transformScaler(InitialType& item);

      V10::CRingScalerItem transformIncrScaler(InitialType& item);
      V10::CRingTimestampedRunningScalerItem
              transformNonIncrScaler(InitialType& item);
      V10::CRingStateChangeItem transformStateChange(InitialType& item);
      V10::CPhysicsEventItem transformPhysicsEvent(InitialType& item);
      V10::CRingPhysicsEventCountItem transformPhysicsEventCount(InitialType& item);
      V10::CRingFragmentItem transformFragment(InitialType& item);
      V10::CUnknownFragment transformUnknownFragment(InitialType& item);
      V10::CRingTextItem transformText(InitialType& item);

    };

  } // end Transform
} // end DAq

#endif // CTRANSFORM11P0TO10P0_H
