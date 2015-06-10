

#ifndef CSKIPNUMBERPREDICATE_H
#define CSKIPNUMBERPREDICATE_H

class CRunStatistics;

class CSkipNumberPredicate : public CPredicate
{
  private:
    CRunStatistics m_runStats;
    
  public:
    CSkipNumberPredicate(size_t nToSkip, CRunStatistics& stats);

    bool operator()();
};

#endif
