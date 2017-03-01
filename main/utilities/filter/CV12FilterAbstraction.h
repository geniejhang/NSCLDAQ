#ifndef DAQ_V12_CFILTERABSTRACTION_H
#define DAQ_V12_CFILTERABSTRACTION_H

#include <CFilterVersionAbstraction.h>
#include <V12/CRingItem.h>

namespace DAQ {

class CDataSource;
class CDataSink;

namespace V12 {

class CFilter;

//////////////////////////////////////////////////////////////////////
/// \brief The CV12VersionAbstraction class
class CFilterAbstraction : public CFilterVersionAbstraction {
private:
    CRawRingItem m_item;
    CRingItemPtr m_pInputItem;
    CRingItemPtr m_pOutputItem;
    std::shared_ptr<CFilter> m_pFilter;


public:
    CFilterAbstraction();
    virtual void readDatum(CDataSource& source);
    virtual void processDatum();
    virtual void outputDatum(CDataSink& sink);
    virtual uint32_t getDatumType() const;
    virtual void cleanup();

    void setFilter(std::shared_ptr<CFilter> pFilter);
    CRingItemPtr dispatch(CRingItemPtr pItem);
};

} // end V12
} // end DAQ

#endif // DAQ_V12_CFILTERABSTRACTION_H
