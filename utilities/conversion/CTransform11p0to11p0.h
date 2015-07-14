

#ifndef CTransform11p0to11p0_H
#define CTransform11p0to11p0_H

#include <V11/CRingItem.h>
#include <memory>

class CFilter;

namespace DAQ {
  namespace Transform {

    class CTransform11p0to11p0
    {
    public:
      using InitialType = V11::CRingItem;
      using FinalType =   V11::CRingItem;

    private:
      std::unique_ptr<CFilter> m_pFilter;

    public:
      CTransform11p0to11p0(std::unique_ptr<CFilter> pFilt);

      FinalType operator()(InitialType& item);

      FinalType dispatch(InitialType* item);
    };

  } // end Transform
} // end DAQ

#endif
