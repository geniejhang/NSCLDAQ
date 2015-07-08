#include "CTransformFactory.h"

using namespace std;

namespace DAQ {
  namespace Transform {

CTransformFactory::CTransformFactory()
  : m_creators()
{
}


void
CTransformFactory::setCreator(int vsnFrom, int vsnTo, std::unique_ptr<CTransformCreator> pCreator)
{
    pair<int,int> transformId(vsnFrom, vsnTo);

   m_creators[transformId] = move(pCreator);
}

unique_ptr<CBaseMediator>
CTransformFactory::create(int vsnFrom, int vsnTo)
{
    const pair<int, int> transformId(vsnFrom, vsnTo);

    auto& pCreator = m_creators.at(transformId);

    return (*pCreator)();

}

  } // end of Transform
} // end of DAQ
