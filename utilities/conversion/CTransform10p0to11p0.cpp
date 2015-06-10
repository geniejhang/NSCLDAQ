
#include <CTransform10p0to11p0.h>
#include <NSCLDAQ10/CRingItemFactory.h>

using namespace std;

CTransform10p0to11p0::FinalType
CTransform10p0to11p0::operator()(InitialType& item)
{
  InitialType* pItem = NSCLDAQ10::CRingItemFactory::createRingItem(&item);

  return dispatch(*pItem);
}

CTransform10p0to11p0::FinalType
CTransform10p0to11p0::dispatch(InitialType& item)
{
    return CTransform10p0to11p0::FinalType(1);
}
