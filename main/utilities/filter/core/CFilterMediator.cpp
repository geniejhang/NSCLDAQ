#include "CFilterMediator.h"
#include <CFilterVersionAbstraction.h>

namespace DAQ {


/*!
 * \brief Constructor
 *
 * \param pSource   the data source
 * \param pSink     the data sink
 *
 * By default, the mediator has no predicate.
 */
CFilterMediator::CFilterMediator(CDataSourcePtr pSource, CDataSinkPtr pSink)
    : CPredicatedMediator(pSource, pSink), m_abort(false)
{
}

/*!
 * \brief CFilterMediator::mainLoop
 *
 * The mainLoop is where the bulk of the logic happens. In essence the following
 * happens over and over again until the predicate indicates that it is
 * time to abort.
 *
 * 1. Call CPredicate::preInputUpdate()
 * 2. Call CVersionAbstraction::readDatum()
 * 3. Call CPredicate::postInputUpdate()
 * 4. Call CVersionAbstraction::processDatum()
 * 5. Call CPredicate::preOutputUpdate()
 * 6. Call CVersionAbstraction::writeDatum()
 * 7. Call CPredicate::postOutputUpdate()
 * 8. Call CVersionAbstraction::cleanUp()
 *
 *
 * If the CFilterMediator::setAbort() is called by any step in this sequency, the
 * looping will return after step 8. Also, if the predicate returns ABORT during any
 * sequence, the sequence immediately stops being executed. If instead, the predicate
 * returns SKIP, the sequence immediately returns to step 1.
 *
 * \throws std::runtime_error if called being passing the object a predicate
 *
 */
void CFilterMediator::mainLoop()
{

    // Dereference our pointers before entering
    // the main loop
    CDataSource& source = *getDataSource();
    CDataSink& sink = *getDataSink();

    if (m_pPredicate == nullptr) {
        throw std::runtime_error("CFilterMediator::mainLoop() cannot continue "
                                 "without a predicate.");
    }

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
            m_pVsnAbstraction->cleanUp();
            continue;
        } else if (action == CPredicatedMediator::ABORT) {
            m_pVsnAbstraction->cleanUp();
            break;
        }

        m_pVsnAbstraction->processDatum();

        // Only send an item if it is not null.
        // The user could return null to prevent sending data
        // to the sink
        action = m_pPredicate->preOutputUpdate(*this, m_pVsnAbstraction->getDatumType());
        if (action == CPredicatedMediator::SKIP) {
            m_pVsnAbstraction->cleanUp();
            continue;
        } else if (action == CPredicatedMediator::ABORT) {
            m_pVsnAbstraction->cleanUp();
            break;
        }

        m_pVsnAbstraction->outputDatum(sink);

        action = m_pPredicate->postOutputUpdate(*this, m_pVsnAbstraction->getDatumType());
        if (action == CPredicatedMediator::SKIP) {
            m_pVsnAbstraction->cleanUp();
            continue;
        } else if (action == CPredicatedMediator::ABORT) {
            m_pVsnAbstraction->cleanUp();
            break;
        }

        m_pVsnAbstraction->cleanUp();

        if (m_abort) break;
    }

}

/*!
 * \brief CFilterMediator::initialize
 *
 * Reinitializes the abort bit to false and calls the initialize method of
 * the version abstraction.
 */
void CFilterMediator::initialize()
{
    m_abort = false;
  m_pVsnAbstraction->initialize();
}

/*!
 * \brief CFilterMediator::finalize
 *
 * Calls the finalize method of the version abstraction.
 */
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


/*!
 * \brief Sets the version abstraction
 *
 * \param pAbstraction  the version abstraction
 *
 * A handshake is performed between the abstraction object and this object. This
 * object stores the abstraction but also passes itself to the abstraction
 * via its setFilterMediator method. In this way, this object and its version abstraction
 * subsequently know about each other. Note that there is a 1-to-1 relationship between the
 * filter mediator and its version abstraction.
 */
void CFilterMediator::setVersionAbstraction(CFilterVersionAbstractionPtr pAbstraction)
{
    m_pVsnAbstraction = pAbstraction;
    m_pVsnAbstraction->setFilterMediator(*this);
}


void CFilterMediator::setExcludeList(const std::string &excludeList)
{
    m_pVsnAbstraction->setExcludeList(excludeList);
}


void CFilterMediator::setSampleList(const std::string &sampleList)
{
    m_pVsnAbstraction->setSampleList(sampleList);
}

void CFilterMediator::setAbort() {
    m_abort = true;
}

} // end DAQ
