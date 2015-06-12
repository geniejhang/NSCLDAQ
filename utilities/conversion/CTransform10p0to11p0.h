

#ifndef CTransform10p0to11p0_H
#define CTransform10p0to11p0_H

namespace NSCLDAQ10
{
    class CRingItem;
}

namespace NSCLDAQ11 {
    class CRingItem;
}

class CTransform10p0to11p0 
{
  public:
    typedef typename NSCLDAQ10::CRingItem InitialType;
    typedef typename NSCLDAQ11::CRingItem FinalType;

  public:
    FinalType operator()(InitialType& item);
    FinalType dispatch(InitialType& item);

   public:
    FinalType transformScaler(InitialType& item);
    FinalType transformStateChange(InitialType& item);
    FinalType transformPhysicsEvent(InitialType& item);
    FinalType transformPhysicsEventCount(InitialType& item);
    FinalType transformText(InitialType& item);
    FinalType transformNonIncrScaler(InitialType& item);
    FinalType transformFragment(InitialType& item);
};

#endif
