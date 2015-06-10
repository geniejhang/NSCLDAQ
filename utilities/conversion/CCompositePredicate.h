
#ifndef CCOMPOSITEPREDICATE_H
#define CCOMPOSITEPREDICATE_H

#include <CPredicate.h>

#include <vector>
#include <memory>

class CCompositePredicate : public CPredicate
{
  private:
    std::vector<std::unique_ptr<CPredicate> > m_predicates;

  public:
    void addPredicate(std::unique_ptr<CPredicate> pPred);

    bool operator()();
};

#endif
