

#ifndef CTransform11p0to11p0_H
#define CTransform11p0to11p0_H

#include <NSCLDAQ11/CRingItem.h>
#include <memory>

class CFilter;

class CTransform11p0to11p0 
{
  public:
    using InitialType = NSCLDAQ11::CRingItem;
    using FinalType =   NSCLDAQ11::CRingItem;

  private:
    std::unique_ptr<CFilter> m_pFilter;

  public:
    CTransform11p0to11p0(std::unique_ptr<CFilter> pFilt);

    FinalType operator()(InitialType& item);

    FinalType dispatch(InitialType* item);
};

#endif
