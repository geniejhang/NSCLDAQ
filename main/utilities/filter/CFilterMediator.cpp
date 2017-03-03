#include "CFilterMediator.h"
#include <CFilterVersionAbstraction.h>

namespace DAQ {

CFilterMediator::CFilterMediator(CDataSourcePtr pSource, CDataSinkPtr pSink)
    : CPredicatedMediator(pSource, pSink)
{
}


void CFilterMediator::mainLoop()
{

    // Dereference our pointers before entering
    // the main loop
    CDataSource& source = *getDataSource();
    CDataSink& sink = *getDataSink();

    while (1) {
        auto action = m_pPredicate->preInputUpdate(*this);
        if (action == CPredicatedMediator::SKIP) {
            continue;
        } else if (action == CPredicatedMediator::ABORT) {
            break;
        }

        m_pVsnAbstraction->readDatum(source);

        action = m_pPredicate->postInputUpdate(*this, m_pVsnAbstraction->getDatumType());
        if (action == CPredicatedMediator::SKIP) {
            continue;
        } else if (action == CPredicatedMediator::ABORT) {
            break;
        }

        m_pVsnAbstraction->processDatum();

        // Only send an item if it is not null.
        // The user could return null to prevent sending data
        // to the sink
        action = m_pPredicate->preOutputUpdate(*this, m_pVsnAbstraction->getDatumType());
        if (action == CPredicatedMediator::SKIP) {
            continue;
        } else if (action == CPredicatedMediator::ABORT) {
            break;
        }

        m_pVsnAbstraction->outputDatum(sink);

        action = m_pPredicate->postOutputUpdate(*this, m_pVsnAbstraction->getDatumType());
        if (action == CPredicatedMediator::SKIP) {
            continue;
        } else if (action == CPredicatedMediator::ABORT) {
            break;
        }

        m_pVsnAbstraction->cleanUp();
    }

}

void CFilterMediator::initialize()
{
  m_pVsnAbstraction->initialize();
}

void CFilterMediator::finalize()
{
  m_pVsnAbstraction->finalize();
}

CPredicatePtr CFilterMediator::getPredicate()
{
    return m_pPredicate;
}

void CFilterMediator::setPredicate(CPredicatePtr pPredicate)
{
    m_pPredicate = std::dynamic_pointer_cast<CCompositePredicate>(pPredicate);
}


void CFilterMediator::setVersionAbstraction(CFilterVersionAbstractionPtr pAbstraction)
{
    m_pVsnAbstraction = pAbstraction;
}


void CFilterMediator::setExcludeList(const std::string &excludeList)
{
    m_pVsnAbstraction->setExcludeList(excludeList);
}


void CFilterMediator::setSampleList(const std::string &sampleList)
{
    m_pVsnAbstraction->setSampleList(sampleList);
}

} // end DAQ
