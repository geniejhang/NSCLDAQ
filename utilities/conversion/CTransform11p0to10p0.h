#ifndef CTRANSFORM11P0TO10P0_H
#define CTRANSFORM11P0TO10P0_H

namespace NSCLDAQ10
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

namespace NSCLDAQ11 {
    class CRingItem;
}

namespace DAQ {
  namespace Transform {

class CTransform11p0to10p0
{
public:
    typedef typename NSCLDAQ11::CRingItem InitialType;
    typedef typename NSCLDAQ10::CRingItem FinalType;

public:
    FinalType operator()(InitialType& item);
    FinalType dispatch(InitialType& item);

    FinalType transformScaler(InitialType& item);

    NSCLDAQ10::CRingScalerItem transformIncrScaler(InitialType& item);
    NSCLDAQ10::CRingTimestampedRunningScalerItem
        transformNonIncrScaler(InitialType& item);
    NSCLDAQ10::CRingStateChangeItem transformStateChange(InitialType& item);
    NSCLDAQ10::CPhysicsEventItem transformPhysicsEvent(InitialType& item);
    NSCLDAQ10::CRingPhysicsEventCountItem transformPhysicsEventCount(InitialType& item);
    NSCLDAQ10::CRingFragmentItem transformFragment(InitialType& item);
    NSCLDAQ10::CUnknownFragment transformUnknownFragment(InitialType& item);
    NSCLDAQ10::CRingTextItem transformText(InitialType& item);

};

  } // end Transform
} // end DAq

#endif // CTRANSFORM11P0TO10P0_H
