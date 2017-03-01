#include "CFilterMediator.h"

namespace DAQ {

CFilterMediator::CFilterMediator()
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

        action = m_pPredicate->postInputUpdate(*this, pItem->type());
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

        action = m_pPredicate->postOutputUpdate(*this, pNewItem->type());
        if (action == CPredicatedMediator::SKIP) {
            continue;
        } else if (action == CPredicatedMediator::ABORT) {
            break;
        }

        m_pVsnAbstraction->cleanup();
    }

}

void CFilterMediator::initialize()
{
  getFilter()->initialize();
}

void CFilterMediator::finalize()
{
  getFilter()->finalize();
}

std::shared_ptr<CPredicate> CFilterMediator::getPredicate() const
{
    return m_pPredicate;
}

void CFilterMediator::setPredicate(std::shared_ptr<CPredicate> pPredicate)
{
    m_pPredicate = pPredicate;
}

} // end DAQ
