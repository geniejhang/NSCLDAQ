
#include <CCompositePredicate.h>

using namespace std;

void CCompositePredicate::addPredicate(std::unique_ptr<CPredicate> pPred)
{
  m_predicates.push_back(move(pPred));
}


bool CCompositePredicate::operator()()
{
  bool flag = true;

  for (auto& pred : m_predicates) {
    flag &= (*pred)();
  }

  return flag;
}
