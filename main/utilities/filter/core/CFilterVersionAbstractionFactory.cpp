#include "CFilterVersionAbstractionFactory.h"

namespace DAQ {


void CFilterVersionAbstractionFactory::addCreator(int type, CreatorPtr creator)
{

  m_creators[type] = creator;

}

CFilterVersionAbstractionFactory::CreatorPtr
CFilterVersionAbstractionFactory::getCreator(int type) {

  auto pFound =  m_creators.find(type);

  if (pFound != m_creators.end()) {
      return pFound->second;
  } else {
      return nullptr;
  }
}

CFilterVersionAbstractionUPtr
CFilterVersionAbstractionFactory::create(int type) const
{
    // this should throw if the type does not exist
    return m_creators.at(type)->create();
}

} // end DAQ
