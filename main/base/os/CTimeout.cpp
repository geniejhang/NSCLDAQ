#include "CTimeout.h"


namespace DAQ {

CTimeout::CTimeout(double nSeconds)
    : m_nSeconds(nSeconds),
      m_start(std::chrono::high_resolution_clock::now())
{}

double CTimeout::getTotalSeconds() const {
  return m_nSeconds;
}

double CTimeout::getRemainingSeconds() const {
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedTime = now - m_start;

    double remainingSecs = m_nSeconds - elapsedTime.count();

    return (remainingSecs > 0) ? remainingSecs : 0;
}


bool CTimeout::expired() const
{
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedTime = now - m_start;

    return (elapsedTime.count() > m_nSeconds);
}

void CTimeout::reset()
{
    m_start = std::chrono::high_resolution_clock::now();
}



} // end DAQ
