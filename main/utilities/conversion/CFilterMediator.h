#ifndef DAQ_TRANSFORM_CFILTERMEDIATOR_H
#define DAQ_TRANSFORM_CFILTERMEDIATOR_H

#include <CPredicatedMediator.h>

#include <V10/CRingItem.h>
#include <V11/CRingItem.h>
#include <V12/CRawRingItem.h>

namespace DAQ {
namespace Transform {

class CFilterVersionAbstraction {
public:
    virtual CFilterVersionAbstraction();
    virtual void readDatum(CDataSource& source) = 0;
    virtual void processDatum() = 0;
    virtual void outputDatum(CDataSink&) = 0;
    virtual uint32_t getDatumType() const = 0;
    virtual void cleanup() = 0;
};

//////////////////////////////////////////////////////////////////////

class CV10VersionAbstraction : public CFilterVersionAbstraction {
private:
    V10::CRingItem  m_item;
    V10::CRingItem* m_pInputItem;
    V10::CRingItem* m_pOutputItem;
    std::shared_ptr<CV10Filter>     m_pFilter;

public:
    CV10VersionAbstraction();
    ~CV10VersionAbstraction();
    virtual void readDatum(CDataSource& source);
    virtual void processDatum();
    virtual void outputDatum(CDataSink&);
    virtual uint32_t getDatumType() const;
    virtual void cleanup();

    void setFilter(std::shared_ptr<CV10Filter> pFilter);
    V10::CRingItem* dispatch(V10::CRingItem& item);
};

//////////////////////////////////////////////////////////////////////

class CV11VersionAbstraction : public CFilterVersionAbstraction {
private:
    V11::CRingItem m_item;
    V11::CRingItem* m_pInputItem;
    V11::CRingItem* m_pOutputItem;
    std::shared_ptr<CV11Filter> m_pFilter;

public:
    CV11VersionAbstraction();
    virtual void readDatum(CDataSource& source);
    virtual void processDatum();
    virtual void outputDatum(CDataSink&);
    virtual uint32_t getDatumType() const;
    virtual void cleanup();

    void setFilter(std::shared_ptr<CV11Filter> pFilter);
    DAQ::V12::CRawRingItem dispatch(DAQ::V12::CRawRingItem &item);
};


//////////////////////////////////////////////////////////////////////
/// \brief The CV12VersionAbstraction class


class CV12VersionAbstraction : public CFilterVersionAbstraction {
private:
    V12::CRawRingItem* m_pItem;


public:
    CV12VersionAbstraction();
    virtual void readDatum(CDataSource& source);
    virtual void processDatum();
    virtual void outputDatum(CDataSink&);
    virtual uint32_t getDatumType() const;
    virtual void cleanup();

    void setFilter(std::shared_ptr<CV12Filter> pFilter);
    V12::CRawRingItem dispatch(V12::CRawRingItem& item);
};



///////////////////////////////////////////////////////////////////////
/// \brief The CFilterMediator class
///
class CFilterMediator : public CPredicatedMediator
{
private:
    std::shared_ptr<CPredicate>                m_pPredicate;
    std::shared_ptr<CFilterVersionAbstraction> m_pVsnAbstraction;

public:
    CFilterMediator(std::shared_ptr<CDataSource> pSource = nullptr,
                    std::shared_ptr<CDataSink> pSink = nullptr);

    void mainLoop();
    void initialize();
    void finalize();

    void setVersionAbstraction(std::shared_ptr<CFilterVersionAbstraction> pAbstraction);
    void setPredicate(std::shared_ptr<CPredicate> pPredicate);
    std::shared_ptr<CPredicate> getPredicate() const;
};


} // end Transform
} // end DAQ


#endif // CFILTERMEDIATOR_H
