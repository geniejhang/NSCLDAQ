

#ifndef DAQ_STATECHANGEHANDLER_H
#define DAQ_STATECHANGEHANDLER_H

#include <cstdint>
#include <map>
#include <vector>

namespace DAQ {

class COneShotHandler 
{
  private:
  int m_nExpectedSources;
  std::map<uint32_t,uint32_t> m_stateCounts;
  uint32_t m_cachedRunNo;
  bool     m_complete;
  uint32_t m_beginType;
  uint32_t m_endType;

  public:
    COneShotHandler(int ntrans,
                    uint32_t beginType, uint32_t endType,
                    const std::vector<uint32_t>& types);

    void setExpectedTransitions(int transitions);
    void initialize(uint32_t runNumber);

    void update(uint32_t type, uint32_t runNumber);

    bool waitingForBegin() const;
    bool complete() const;

    void reset();

    uint32_t getCount(uint32_t key) const;

  private:
    bool validType(uint32_t type) const;
    void clearCounts();
    
    void updateState(uint32_t type, uint32_t runNumber);
};

} // end DAQ

#endif
