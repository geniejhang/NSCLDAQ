#ifndef DAQ_CFILTERVERSIONABSTRACTIONFACTORY_H
#define DAQ_CFILTERVERSIONABSTRACTIONFACTORY_H

#include <CFilterVersionAbstraction.h>

#include <map>

namespace DAQ {


class CFilterVersionAbstractionCreator;
using CFilterVersionAbstractionCreatorPtr = std::shared_ptr<CFilterVersionAbstractionCreator>;


class CFilterVersionAbstractionCreator {
public:
  virtual CFilterVersionAbstractionUPtr create() const = 0;
};





class CFilterVersionAbstractionFactory {
public:
  using CreatorPtr = CFilterVersionAbstractionCreatorPtr;
  enum Key { v10 = 0, v11 = 1, v12 = 2 };

private:
  std::map<int, CreatorPtr> m_creators;

public:
  void addCreator(int type, CreatorPtr creator);
  CreatorPtr getCreator(int type);

  CFilterVersionAbstractionUPtr create(int type) const;
};

} // end DAQ

#endif // DAQ_CFILTERVERSIONABSTRACTIONFACTORY_H
