#ifndef DAQ_CPREDICATEDMEDIATOR_H
#define DAQ_CPREDICATEDMEDIATOR_H

#include <CBaseMediator.h>

namespace DAQ {

class CPredicate;
using CPredicatePtr = std::shared_ptr<CPredicate>;

/*!
 * \brief The CPredicatedMediator class
 *
 * The CPredicatedMediator class is just a mediator that incorporates
 * a predicate into it.
 */
class CPredicatedMediator : public CBaseMediator {

public:
    enum Action { ABORT, SKIP, CONTINUE };

public:
    CPredicatedMediator(std::shared_ptr<CDataSource> pSource = nullptr,
                        std::shared_ptr<CDataSink> pSink = nullptr);

    virtual ~CPredicatedMediator() {}

    virtual void mainLoop() = 0;

    virtual void initialize() = 0;
    virtual void finalize() = 0;

    virtual void setPredicate(CPredicatePtr pPredicate) = 0;
    virtual CPredicatePtr getPredicate() = 0;
};

}

#endif // DAQ_CPREDICATEDMEDIATOR_H
