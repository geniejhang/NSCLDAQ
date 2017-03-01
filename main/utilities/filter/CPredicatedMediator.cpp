
#include <CPredicatedMediator.h>


namespace DAQ {
namespace Transform {

CPredicatedMediator::CPredicatedMediator(std::shared_ptr<CDataSource> pSource, std::shared_ptr<CDataSink> pSink)
    : CBaseMediator(pSource, pSink) {}

} // end Transform
} // end DAQ
