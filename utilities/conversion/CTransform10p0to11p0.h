

#ifndef CTransform10p0to11p0_H
#define CTransform10p0to11p0_H

#include <NSCLDAQ10/CRingItem.h>
#include <NSCLDAQ11/CRingItem.h>
#include <memory>

class CTransform10p0to11p0 
{
  public:
    typedef typename NSCLDAQ10::CRingItem InitialType;
    typedef typename NSCLDAQ11::CRingItem FinalType;

  public:
    FinalType operator()(InitialType& item);
    FinalType dispatch(InitialType& item);
};

#endif
