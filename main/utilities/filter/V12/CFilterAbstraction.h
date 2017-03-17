#ifndef DAQ_V12_CFILTERABSTRACTION_H
#define DAQ_V12_CFILTERABSTRACTION_H

#include <CFilterVersionAbstraction.h>
#include <V12/CRawRingItem.h>
#include <V12/CCompositeFilter.h>

#include <CSimpleAllButPredicate.h>

#include <memory>

namespace DAQ {

class CDataSource;
class CDataSink;
class CFilterMediator;

namespace V12 {

class CFilter;
class CFilterAbstraction;

using CFilterAbstractionUPtr = std::unique_ptr<CFilterAbstraction>;
using CFilterAbstractionPtr = std::shared_ptr<CFilterAbstraction>;

class CFilterAbstraction : public CFilterVersionAbstraction {

private:
    CRawRingItem            m_item;
    CRingItemPtr            m_pInputItem;
    CRingItemPtr            m_pOutputItem;
    CCompositeFilterPtr     m_pFilter;
    CSimpleAllButPredicate  m_predicate;
    CFilterMediator*        m_pMediator;

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

    virtual void initialize();
    virtual void finalize();

    virtual void setExcludeList(const std::string& excludeList);
    virtual void setSampleList(const std::string& sampleList);

    virtual void setFilterMediator(CFilterMediator& mediator);
    virtual CFilterMediator* getFilterMediator();

    void registerFilter(CFilterPtr pFilter);
    CFilterPtr getFilter() const;
    CRingItemPtr dispatch(CRingItemPtr item);
};


} // end V12
} // end DAQ

#endif // CV11VERSIONFILTERABSTRACTION_H
