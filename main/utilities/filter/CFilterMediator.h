#ifndef DAQ_CFILTERMEDIATOR_H
#define DAQ_CFILTERMEDIATOR_H

#include <CPredicatedMediator.h>

#include <memory>

namespace DAQ {

class CFilterVersionAbstraction;

class CFilterMediator;
using CFilterMediatorUPtr = std::unique_ptr<CFilterMediator>;
using CFilterMediatorPtr = std::shared_ptr<CFilterMediator>;

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


} // end DAQ


#endif // DAQ_CFILTERMEDIATOR_H
