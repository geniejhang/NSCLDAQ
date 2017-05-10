#include "CTimeout.h"

#include <iostream>
using namespace std::chrono;

namespace DAQ {

CTimeout::CTimeout(long nMicroseconds)
    : m_start(high_resolution_clock::now()),
      m_end(m_start + microseconds(nMicroseconds))
{}

CTimeout::CTimeout(const high_resolution_clock::duration& duration)
    : m_start(high_resolution_clock::now()),
      m_end(m_start + duration)
{}

nanoseconds CTimeout::getTotalTime() const {
  return duration_cast<nanoseconds>(m_end - m_start);
}

nanoseconds CTimeout::getRemainingTime() const {
   auto now = high_resolution_clock::now();

   if (m_end > now) {
      return duration_cast<nanoseconds>(m_end - now);
   } else {
       return nanoseconds(0);
   }
}

double CTimeout::getRemainingSeconds() const {
    auto now = high_resolution_clock::now();

    if (m_end > now) {
        return duration<double>(m_end - now).count();
    } else {
        return 0.0;
    }
}


bool CTimeout::expired() const
{
    return high_resolution_clock::now() > m_end;
}

void CTimeout::reset()
{
    high_resolution_clock::duration period = m_end - m_start;

    m_start = high_resolution_clock::now();
    m_end = m_start + period;
}



} // end DAQ
