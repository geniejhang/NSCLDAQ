#ifndef DAQ_V11_CFILTERABSTRACTION_H
#define DAQ_V11_CFILTERABSTRACTION_H

#include <CFilterVersionAbstraction.h>
#include <V11/CRingItem.h>
#include <V11/CCompositeFilter.h>

#include <memory>

namespace DAQ {

class CDataSource;
class CDataSink;

namespace V11 {

class CFilter;
class CFilterAbstraction;

using CFilterAbstractionUPtr = std::unique_ptr<CFilterAbstraction>;
using CFilterAbstractionPtr = std::shared_ptr<CFilterAbstraction>;

class CFilterAbstraction : public CFilterVersionAbstraction {
private:
    CRingItem  m_item;
    CRingItem* m_pInputItem;
    CRingItem* m_pOutputItem;
    CCompositeFilterPtr m_pFilter;

public:
    CFilterAbstraction();
    CFilterAbstraction(const CFilterAbstraction&) = default;
    CFilterAbstraction& operator=(const CFilterAbstraction&) = default;
    ~CFilterAbstraction();

    virtual void readDatum(CDataSource& source);
    virtual void processDatum();
    virtual void outputDatum(CDataSink& sink);
    virtual uint32_t getDatumType() const;
    virtual void cleanUp();

    void registerFilter(CFilterPtr pFilter);
    CFilterPtr getFilter() const;
    CRingItem* dispatch(CRingItem &item);
};


} // end V11
} // end DAQ

#endif // CV11VERSIONFILTERABSTRACTION_H
