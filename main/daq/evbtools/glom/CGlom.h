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
     uint64_t m_firstTimestamp; ///< stores earliest timestamp in accumulated items
     uint64_t m_lastTimestamp;  ///< stores latest timestamp in accumulated items
     uint64_t m_timestampSum;   ///< stores sum of timestamps in accumulated items for average
     uint32_t m_sourceId;       ///< source id to assign to all outputted items
     uint32_t m_currentType;    ///< current type of items being accumulated
     uint64_t m_dtInt;          ///< correlation time width

     bool     m_nobuild;        ///< indicates whether or not correlation will occur
     V12::CGlomParameters::TimestampPolicy m_timestampPolicy; ///< policy for assigning timestamps
     unsigned m_stateChangeNesting;  ///< sum of all BEGIN_RUN and END_RUN items where BEGIN_RUN=1, END_RUN=-1
     bool m_firstBarrier;  ///< flag to assist with glom info outputting logic

    std::vector<V12::CRingItemPtr> m_accumulatedItems; ///< the correlated items for output
    CDataSinkPtr m_pSink;  ///< the data sink to write to

public:
    CGlom(CDataSinkPtr pSink);
    ~CGlom();

    void operator ()();

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
