

#ifndef MYTRIGGER_H
#define MYTRIGGER_H

#include <CEventTrigger.h>
#include <CAENcard.h>

class MyTrigger : public CEventTrigger
{

  private:
    CAENcard m_module;
    size_t   m_trialsToTimeout;

  public:
    MyTrigger(int slot);
    bool operator()();

    void setPollTimeout(size_t maxPolls) { m_trialsToTimeout = maxPolls;}
    size_t getPollTimeout() const        { return m_trialsToTimeout;}
};

#endif
