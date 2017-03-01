#ifndef CPREDICATEDMEDIATOR_H
#define CPREDICATEDMEDIATOR_H

#include <CBaseMediator.h>

namespace DAQ {
namespace Transform {

    class CPredicate;

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

        virtual void setPredicate(std::shared_ptr<CPredicate> pPredicate) = 0;
        virtual std::shared_ptr<CPredicate> getPredicate() = 0;
    };
}
}

#endif // CPREDICATEDMEDIATOR_H
