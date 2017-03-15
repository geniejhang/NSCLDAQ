#ifndef DAQ_TRANSFORM_CTESTPREDICATE_H
#define DAQ_TRANSFORM_CTESTPREDICATE_H

#include <CPredicate.h>

#include <string>
#include <vector>

namespace DAQ {
namespace Transform {

/*!
 * \brief The CTestPredicate class
 *
 * A simple class used for testing purposes. The CTestPredicate logs all of the
 * calls make to it as a list of strings.
 */
class CTestPredicate : public CPredicate
{
private:
    std::vector<std::string> m_log;

public:
    CTestPredicate(const std::string& name);
    CTestPredicate(const CTestPredicate&) = default;
    ~CTestPredicate() = default;

    CPredicatedMediator::Action preInputUpdate(CPredicatedMediator &transform);
    CPredicatedMediator::Action postInputUpdate(CPredicatedMediator &transform, int type);
    CPredicatedMediator::Action preOutputUpdate(CPredicatedMediator &transform, int type);
    CPredicatedMediator::Action postOutputUpdate(CPredicatedMediator &transform, int type);
    void reset();

    std::vector<std::string> getLog() const { return m_log; }
};

} // end Transform
} // end DAQ

#endif // DAQ_TRANSFORM_CTESTPREDICATE_H
