#ifndef DAQ_CGLOM_H
#define DAQ_CGLOM_H

#include <CDataSink.h>
#include <V12/CGlomParameters.h>

#include <cstdint>
#include <vector>

namespace DAQ {


/*!
 * \brief The CGlom class
 *
 * The CGlom class is the logic of the glom program. It is responsible for
 * reading in fragments from stdin, stripping off the fragment header, performing
 * any correlation necessary, and then outputting the built item to stdout. There is
 * so extra logic besides that to ensure that the glom parameters are outputted when
 * the first barrier BEGIN_RUN arrives.
 *
 */
class CGlom
{
    private:
     uint64_t m_firstTimestamp;
     uint64_t m_lastTimestamp;
     uint64_t m_timestampSum;
     uint32_t m_sourceId;
     uint64_t m_fragmentCount;
     uint32_t m_currentType;
     uint64_t m_dtInt;

     bool     m_firstEvent;
     bool     m_nobuild;
     V12::CGlomParameters::TimestampPolicy m_timestampPolicy;
     unsigned m_stateChangeNesting;
     bool m_firstBarrier;

    std::vector<V12::CRingItemPtr> m_accumulatedItems;
    CDataSinkPtr m_pSink;

public:
    CGlom(CDataSinkPtr pSink);
    ~CGlom();

    int operator()();

    void outputGlomParameters(uint64_t dt, bool building);
    void emitAbnormalEnd();
    void accumulateEvent(uint64_t dt, V12::CRingItemPtr pItem);
    void outputEventFormat();

    void disableBuilding(bool nobuild);
    void setCorrelationTime(uint64_t dt);
    void setTimestampPolicy(V12::CGlomParameters::TimestampPolicy policy);
    void setFirstBarrier(bool expectingBarrier);

    void handleItem(V12::CRingItemPtr pItem);
private:
    void flushEvent();

};

} // end DAQ

#endif // CGLOM_H
