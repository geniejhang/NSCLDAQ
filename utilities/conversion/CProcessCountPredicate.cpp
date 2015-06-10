
#include <CProcessCountPredicate.h>
#include <CRunStatistics.h>


CProcessCountPredicate::CProcessCountPredicate(size_t nToProcess, CRunStatistics& stats)
  : m_stats(stats), m_toProcess(nToProcess)
{}


bool CProcessCountPredicate::operator()()
{
  return (m_stats.getProcessCount() < m_toProcess);
}
