#ifndef DAQ_V11_CFILTERABSTRACTION_H
#define DAQ_V11_CFILTERABSTRACTION_H

#include <CFilterVersionAbstraction.h>
#include <V11/CRingItem.h>

#include <memory>

namespace DAQ {

class CDataSource;
class CDataSink;

namespace V11 {

class CFilter;

class CFilterAbstraction : public CFilterVersionAbstraction {
private:
    CRingItem m_item;
    CRingItem* m_pInputItem;
    CRingItem* m_pOutputItem;
    std::shared_ptr<CFilter> m_pFilter;

public:
    CFilterAbstraction();
    virtual void readDatum(CDataSource& source);
    virtual void processDatum();
    virtual void outputDatum(CDataSink& sink);
    virtual uint32_t getDatumType() const;
    virtual void cleanup();

    void setFilter(std::shared_ptr<CFilter> pFilter);
    CRingItem* dispatch(CRingItem &item);
};


} // end V11
} // end DAQ

#endif // CV11VERSIONFILTERABSTRACTION_H
