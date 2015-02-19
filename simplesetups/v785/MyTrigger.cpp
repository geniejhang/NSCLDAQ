#include "MyTrigger.h"

MyTrigger::MyTrigger(int slot) 
  : CEventTrigger(), 
  m_module(slot),
  m_trialsToTimeout(100)
{}


bool MyTrigger::operator()() {
  int nTrials=0;

  // Loop waits for data to become ready
  while(nTrials<m_trialsToTimeout && !m_module.dataPresent()) {
    ++nTrials;
  }

  return (nTrials<m_trialsToTimeout);
}
