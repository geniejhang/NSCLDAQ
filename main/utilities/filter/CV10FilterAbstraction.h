#ifndef DAQ_V10_CFILTERABSTRACTION_H
#define DAQ_V10_CFILTERABSTRACTION_H

#include <CFilterVersionAbstraction.h>
#include <V10/CRingItem.h>

#include <memory>

namespace DAQ {

class CDataSource;
class CDataSink;

namespace V10 {

class CFilter;

class CFilterAbstraction : public CFilterVersionAbstraction {
private:
    CRingItem m_item;
    CRingItem* m_pInputItem;
    CRingItem* m_pOutputItem;
    std::shared_ptr<CFilter> m_pFilter;

public:
    CV11FilterAbstraction();
    virtual void readDatum(CDataSource& source);
    virtual void processDatum();
    virtual void outputDatum(CDataSink&);
    virtual uint32_t getDatumType() const;
    virtual void cleanup();

    void setFilter(std::shared_ptr<CFilter> pFilter);
    CRingItem* dispatch(CRingItem &item);
};

} // end V10
} // end DAQ

#endif // DAQ_V10_CFILTERABSTRACTION_H
