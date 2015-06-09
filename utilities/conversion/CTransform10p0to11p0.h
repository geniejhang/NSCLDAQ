

#ifndef CTransform10p0to11p0_H
#define CTransform10p0to11p0_H

#include <CFilter.h>
#include <memory>

class CTransform10p0to11p0 
{
  public:
    using InitialType = Format::10p0;
    using FinalType   = Format::11p0;

  public:
    FinalType operator()(InitialType& item);
};

#endif
