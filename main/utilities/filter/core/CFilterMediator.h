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
using CFilterMediatorPtr  = std::shared_ptr<CFilterMediator>;

///////////////////////////////////////////////////////////////////////
/// \brief The CFilterMediator class
///
/// The CFilterMediator class reads data from a source, passes it to a
/// handler, and then writes resultant item to a sink. Because it has to support multiple
/// data format versions, the actual interaction with the source, sink, and handling logic
/// is delegated to an object derived from CFilterVersionAbstraction. In this sense, it
/// implements a strategy pattern.
class CFilterMediator : public CPredicatedMediator
{
private:
    CCompositePredicatePtr       m_pPredicate;
    CFilterVersionAbstractionPtr m_pVsnAbstraction;
    bool                         m_abort;

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

    void setAbort();
    bool getAbort() const { return m_abort; }
};


} // end DAQ


#endif // DAQ_CFILTERMEDIATOR_H
