#ifndef SRSSORTER_H
#define SRSSORTER_H

#include <string>
#include <CDataSink.h>
#include <CRingItem.h>


class SRSSorter {
public:
    SRSSorter();
    SRSSorter(uint16_t maxHits, int dtHits);
    ~SRSSorter ();

    void sort(uint8_t* data, const uint64_t hitTimeStamp, int sid, CDataSink& sink, size_t nBytes);
    void reset();

private:

    void newRingItem(const uint64_t hitTimeStamp, int fecId, size_t nBytes, CDataSink& sink);
    void appendRingItem(int fecId, uint8_t* data);
    void deleteRingItem(int fecId);

    static const int HitAndMarkerSizeExtended{8};
    static const int MaxFECs{16};

    //uint64_t m_timestamp[MaxFECs] = {0};

    //CRingItem* m_pRingItem[MaxFECs] = {nullptr};

    struct event {
        CRingItem* pRingItem{nullptr};
        uint64_t timestamp{0};
        uint16_t nHits{0};

        void reset(){
            if (pRingItem != nullptr){
                delete pRingItem;
            }
            pRingItem = nullptr;
            timestamp = 0;
            nHits = 0;
        }
    };
    event m_event[MaxFECs];

    // m_maxHits: cutoff on the number of hits per fec
    // m_dtHits: timestamp window for srs hits (ts in [0, m_dtHits]), should be a positive value
    uint16_t m_maxHits;
    int m_dtHits;




};

#endif
