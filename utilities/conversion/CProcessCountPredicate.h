
#ifndef CPROCESSCOUNTPREDICATE_H
#define CPROCESSCOUNTPREDICATE_H

#include <CPredicate.h>

class CRunStatistics;

class CProcessCountPredicate: public CPredicate
{
  private:
    CRunStatistics& m_stats;
    size_t m_toProcess;
  public:
    CProcessCountPredicate(size_t nToProcess, CRunStatistics& stats);

    bool operator()();
};

#endif
