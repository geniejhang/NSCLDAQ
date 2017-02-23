#include "CTestPredicate.h"

namespace DAQ {
namespace Transform {

CTestPredicate::CTestPredicate(const std::string &name)
{
    m_log.push_back(name);
}

CPredicatedMediator::Action
CTestPredicate::preInputUpdate(CPredicatedMediator &transform)
{
    m_log.push_back("preInputUpdate");
}

CPredicatedMediator::Action
CTestPredicate::postInputUpdate(CPredicatedMediator &transform, int type)
{
    std::string msg = "postInputUpdate:";
    msg += std::to_string(type);
    m_log.push_back(msg);
}

CPredicatedMediator::Action
CTestPredicate::preOutputUpdate(CPredicatedMediator &transform, int type)
{
    std::string msg = "preOutputUpdate:";
    msg += std::to_string(type);
    m_log.push_back(msg);
}

CPredicatedMediator::Action
CTestPredicate::postOutputUpdate(CPredicatedMediator &transform, int type)
{
    std::string msg = "postOutputUpdate:";
    msg += std::to_string(type);
    m_log.push_back(msg);
}

void CTestPredicate::reset()
{
    m_log.push_back("reset");
}

} // end Transform
} // end DAQ
