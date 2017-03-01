#include "CFilterMediator.h"
#include <RingIOV10.h>
#include <RingIOV11.h>
#include <RingIOV12.h>
#include <V10/CRingItem.h>
#include <V11/CRingItem.h>
#include <V12/CRawRingItem.h>

namespace DAQ {
namespace Transform {


CV10VersionAbstraction::CV10VersionAbstraction()
    : m_pItem(new V10::CRingItem(VOID)),
      m_pOutputItem(nullptr)
{}

CV10VersionAbstraction::~CV10VersionAbstraction()
{
    if (m_pOutputItem != m_pItem) {
        // this is legal if m_pOutputItem == nullptr
        delete m_pOutputItem;
    }

    delete m_pItem;
}

void CV10VersionAbstraction::readDatum(CDataSource &source)
{
    readItem(source, *m_pItem);
}

void CV10VersionAbstraction::processDatum()
{
    m_pOutputItem = m_pFilter->handleItem(m_pItem);
}


void CV10VersionAbstraction::outputDatum(CDataSink& sink)
{
    if (m_pOutputItem) {
        writeItem(sink, *m_pOutputItem);
    }
}

uint32_t CV10VersionAbstraction::getDatumType() const
{
    if (m_pOutputItem) {
        return m_pOutputItem->type();
    } else {
        return m_pItem->type();
    }
}

void CV10VersionAbstraction::cleanup()
{
    if (m_pOutputItem != m_pItem) {
        delete m_pOutputItem;
        m_pOutputItem = nullptr;
    }

    m_pItem->setType(V10::VOID);
}

/////////////////////////////////////////////////////////////////

CV11VersionAbstraction::CV11VersionAbstraction()
    : m_pItem(new V11::CRingItem(VOID)),
      m_pOutputItem(nullptr)
{}

CV11VersionAbstraction::~CV11VersionAbstraction()
{
    if (m_pOutputItem != m_pItem) {
        // this is legal if m_pOutputItem == nullptr
        delete m_pOutputItem;
    }

    delete m_pItem;
}

void CV10VersionAbstraction::readDatum(CDataSource &source)
{
    readItem(source, *m_pItem);
}

void CV10VersionAbstraction::processDatum()
{
    m_pOutputItem = m_pFilter->handleItem(m_pItem);
}


void CV10VersionAbstraction::outputDatum(CDataSink& sink)
{
    if (m_pOutputItem) {
        writeItem(sink, *m_pOutputItem);
    }
}

uint32_t CV10VersionAbstraction::getDatumType() const
{
    if (m_pOutputItem) {
        return m_pOutputItem->type();
    } else {
        return m_pItem->type();
    }
}

void CV10VersionAbstraction::cleanup()
{
    if (m_pOutputItem != m_pItem) {
        delete m_pOutputItem;
        m_pOutputItem = nullptr;
    }

    m_pItem->setType(V10::VOID);
}






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

} // end Transform
} // end DAQ
