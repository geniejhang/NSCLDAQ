#ifndef DAQ_TRANSFORM_CFILTERMEDIATOR_H
#define DAQ_TRANSFORM_CFILTERMEDIATOR_H

#include <CPredicatedMediator.h>

#include <V10/CRingItem.h>
#include <V11/CRingItem.h>
#include <V12/CRawRingItem.h>

namespace DAQ {
namespace Transform {








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
    std::shared_ptr<CPredicate> getPredicate();
};


} // end Transform
} // end DAQ


#endif // CFILTERMEDIATOR_H
