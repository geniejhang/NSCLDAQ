

#ifndef CTransform11p0to11p0_H
#define CTransform11p0to11p0_H

#include <CFilter.h>
#include <memory>

class CTransform11p0to11p0 
{
  public:
    using InitialType = Format::11p0;
    using FinalType = Format::11p0;

  private:
    std::unique_ptr<CFilter> m_pFilter;

  public:
    CTransform11p0to11p0(std::unique_ptr<CFilter> pFilt);

    FinalType operator()(InitialType& item);

    FinalType dispatch(InitialType* item);
};

#endif
