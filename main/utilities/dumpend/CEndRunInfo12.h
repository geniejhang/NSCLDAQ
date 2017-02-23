#ifndef CENDRUNINFO12_H
#define CENDRUNINFO12_H

#include <CEndRunInfo.h>
#include <V12/CRingStateChangeItem.h>

#include <vector>
#include <memory>


class CEndRunInfo12 : public CEndRunInfo
{
    std::vector<std::unique_ptr<DAQ::V12::CRingStateChangeItem> > m_endRuns;

  public:
      CEndRunInfo12(int fd);
      virtual ~CEndRunInfo12();

      // Overrides:

  public:
      virtual unsigned numEnds()                        const ;

      virtual bool     hasBodyHeader(int which = 0)     const ;
      virtual uint64_t getEventTimestamp(int which = 0) const ;
      virtual uint32_t getSourceId(int which = 0)       const ;
      virtual uint32_t getBarrierType(int which = 0)    const ;

      virtual uint32_t getRunNumber(int which = 0)      const ;
      virtual float    getElapsedTime(int which=0)      const ;
      virtual std::string getTitle(int which=0)         const ;
      virtual time_t   getTod(int which = 0)            const ;

      // Utilities:

  private:
      void loadEndRuns();
      void throwIfBadIndex(int which) const;
};

#endif // CENDRUNINFO12_H
