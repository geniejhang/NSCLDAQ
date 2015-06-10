
#include <CSkipNumberPredicate.h>

#include <CRunStatistics.h>


CSkipNumberPredicate::CSkipNumberPredicate(size_t nToSkip,
    CRunStatistics& stats)
  : m_runStats(stats),
  m_toSkip(nToSkip)
{}

bool CSkipNumberPredicate::operator()()
{
  return (m_runStats.getItemCount() >= m_toSkip);
}
