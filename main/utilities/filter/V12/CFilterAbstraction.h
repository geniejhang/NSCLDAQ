#ifndef DAQ_V12_CFILTERABSTRACTION_H
#define DAQ_V12_CFILTERABSTRACTION_H

#include <CFilterVersionAbstraction.h>
#include <V12/CRawRingItem.h>
#include <V12/CCompositeFilter.h>

#include <CSimpleAllButPredicate.h>

#include <memory>

namespace DAQ {

// forward declarations
class CDataSource;
class CDataSink;
class CFilterMediator;

namespace V12 {

// Forward declarations
class CFilter;
class CFilterAbstraction;


// Useful smart pointer typedefs
using CFilterAbstractionUPtr = std::unique_ptr<CFilterAbstraction>;
using CFilterAbstractionPtr = std::shared_ptr<CFilterAbstraction>;

/*!
 * \brief The CFilterAbstraction class
 *
 * The V12::CFilterAbstraction defines the logic for handling version 12.0
 * data in a filter. It defines each step of the strategy that is laid out in the
 * CFilterMediator. Basically, it reads 12.0 data from a data source, dispatches
 * the data to the appropriate handler of the registered filters, and then
 * writes the output to the data sink.
 *
 * Objects of this class maintain a composite filter that users can register their
 * own filters to. If the user does not register any new filters, this acts just like
 * a transparent filter, i.e. input ring items are returned without any manipulation.
 *
 * It is also possible for users to exclude certain types of data from processing.
 * To do so, the user must pass a comma separated list of types (as understood
 * by V12::stringListToIntegers()). Any type of item in this list will be skipped
 * over without processing.
 */
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
    CFilterAbstraction(const CFilterAbstraction&);
    CFilterAbstraction& operator=(const CFilterAbstraction&);
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
    CCompositeFilterPtr getFilter() const;
    CRingItemPtr dispatch(CRingItemPtr item);
};


} // end V12
} // end DAQ

#endif // CV11VERSIONFILTERABSTRACTION_H
