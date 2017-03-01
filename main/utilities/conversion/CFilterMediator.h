#ifndef DAQ_TRANSFORM_CFILTERMEDIATOR_H
#define DAQ_TRANSFORM_CFILTERMEDIATOR_H

#include <CPredicatedMediator.h>

#include <V10/CRingItem.h>
#include <V11/CRingItem.h>
#include <V12/CRawRingItem.h>

namespace DAQ {
namespace Transform {

class CVersionAbstraction {
public:
    virtual CVersionAbstraction();
    virtual void readDatum(CDataSource& source) = 0;
    virtual void processDatum() = 0;
    virtual void outputDatum(CDataSink&) = 0;
    virtual uint32_t getDatumType() const = 0;
    virtual void cleanup() = 0;
};


class CV10VersionAbstraction : public CVersionAbstraction {
private:
    V10::CRingItem* m_pItem;
    V10::CRingItem* m_pOutputItem;
public:
    CV10VersionAbstraction();
    ~CV10VersionAbstraction();
    virtual void readDatum(CDataSource& source);
    virtual void processDatum();
    virtual void outputDatum(CDataSink&);
    virtual uint32_t getDatumType() const;
    virtual void cleanup();
};


class CV11VersionAbstraction : public CVersionAbstraction {
private:
    V11::CRingItem* m_pItem;

public:
    CV11VersionAbstraction();
    virtual void readDatum(CDataSource& source);
    virtual void processDatum();
    virtual void outputDatum(CDataSink&);
    virtual uint32_t getDatumType() const;
    virtual void cleanup();
};

class CV12VersionAbstraction : public CVersionAbstraction {
private:
    V12::CRawRingItem* m_pItem;

public:
    CV12VersionAbstraction();
    virtual void readDatum(CDataSource& source);
    virtual void processDatum();
    virtual void outputDatum(CDataSink&);
    virtual uint32_t getDatumType() const;
    virtual void cleanup();
};

class CFilterMediator : public CPredicatedMediator
{
private:
    std::shared_ptr<CPredicate>          m_pPredicate;
    std::shared_ptr<CVersionAbstraction> m_pVsnAbstraction;

public:
    CFilterMediator(std::shared_ptr<CDataSource> pSource = nullptr,
                    std::shared_ptr<CDataSink> pSink = nullptr);

    void mainLoop();
    void initialize();
    void finalize();

    void
    void setPredicate(std::shared_ptr<CPredicate> pPredicate);
    std::shared_ptr<CPredicate> getPredicate() const;
};


} // end Transform
} // end DAQ


#endif // CFILTERMEDIATOR_H
