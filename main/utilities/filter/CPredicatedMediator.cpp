
#include <CPredicatedMediator.h>


namespace DAQ {

CPredicatedMediator::CPredicatedMediator(std::shared_ptr<CDataSource> pSource, std::shared_ptr<CDataSink> pSink)
    : CBaseMediator(pSource, pSink) {}

} // end DAQ
