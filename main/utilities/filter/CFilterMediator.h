#ifndef DAQ_CFILTERMEDIATOR_H
#define DAQ_CFILTERMEDIATOR_H

#include <CPredicatedMediator.h>
#include <CCompositePredicate.h>

#include <memory>

namespace DAQ {

class CFilterVersionAbstraction;
using CFilterVersionAbstractionPtr = std::shared_ptr<CFilterVersionAbstraction>;

class CFilterMediator;
using CFilterMediatorUPtr = std::unique_ptr<CFilterMediator>;
using CFilterMediatorPtr = std::shared_ptr<CFilterMediator>;

///////////////////////////////////////////////////////////////////////
/// \brief The CFilterMediator class
///
class CFilterMediator : public CPredicatedMediator
{
private:
    CCompositePredicatePtr       m_pPredicate;
    CFilterVersionAbstractionPtr m_pVsnAbstraction;

public:
    CFilterMediator(CDataSourcePtr pSource = nullptr,
                    CDataSinkPtr pSink = nullptr);

    void mainLoop();
    void initialize();
    void finalize();

    void setVersionAbstraction(CFilterVersionAbstractionPtr pAbstraction);
    void setPredicate(CPredicatePtr pPredicate);
    CPredicatePtr getPredicate();

    void setExcludeList(const std::string& excludeList);
    void setSampleList(const std::string& sampleList);

};


} // end DAQ


#endif // DAQ_CFILTERMEDIATOR_H
