

#ifndef DAQ_STATECHANGEHANDLER_H
#define DAQ_STATECHANGEHANDLER_H

#include <cstdint>
#include <map>
#include <vector>

namespace DAQ {


/*!
 * \brief Encapsulation of the oneshot logic in a version independent mannner
 *
 * For the most part, this class justs handles some bookkeeping with some methods
 * to indicate the state of the bookkeeping. It requires extra logic to
 * fully implement the behavior of oneshot. In the filter, that extra logic
 * is in one of the COneShotLogicFilter classes. To function independent of version,
 * this class deals with integers and nothing more. The logic that is encapsulated
 * identifies when :
 *
 * 1. No begin run types have been passed to the update method
 * 2. The number of expected end types have been observed.
 * 3. The run number has been observed to change unexpectedly
 * 4. The number of begin types observed exceeds the expected number.
 */
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
