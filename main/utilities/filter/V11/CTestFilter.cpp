
#include <V11/CTestFilter.h>
#include <vector>
#include <string>

using namespace std;

namespace DAQ {
namespace V11 {

CTestFilter::CTestFilter() : m_history(), m_nProcessed(0) {}


std::vector<std::string> CTestFilter::getHistory() { return m_history; }

int CTestFilter::getNProcessed() { return m_nProcessed; }



CTestFilter *CTestFilter::clone() const { return new CTestFilter(*this); }



/*!
 * \return a pointer to a new BEGIN_RUN item
 */
CRingItem *CTestFilter::handleStateChangeItem(CRingStateChangeItem *) {
  ++m_nProcessed;
   m_history.push_back("handleStateChangeItem");
  return new CRingStateChangeItem(BEGIN_RUN);
}


CRingItem *CTestFilter::handleScalerItem(CRingScalerItem *) {
  ++m_nProcessed;
    m_history.push_back("handleScalerItem");
  return new CRingScalerItem(200);
}


CRingItem *CTestFilter::handleTextItem(CRingTextItem *) {
  ++m_nProcessed;
    m_history.push_back("handleTextItem");
  std::vector<std::string> str_vec;
  str_vec.push_back("0000");
  str_vec.push_back("1111");
  str_vec.push_back("2222");
  return new CRingTextItem(PACKET_TYPES, str_vec);
}



CRingItem *CTestFilter::handlePhysicsEventItem(CPhysicsEventItem *) {
  ++m_nProcessed;
    m_history.push_back("handlePhysicsEventItem");
  return new CPhysicsEventItem(4096);
}


CRingItem *
CTestFilter::handlePhysicsEventCountItem(CRingPhysicsEventCountItem *) {
  ++m_nProcessed;
    m_history.push_back("handlePhysicsEventCountItem");
  return new CRingPhysicsEventCountItem(static_cast<uint64_t>(4),
                                        static_cast<uint32_t>(1001));
}


CRingItem *CTestFilter::handleFragmentItem(CRingFragmentItem *) {
  ++m_nProcessed;
    m_history.push_back("handleFragmentItem");

  return new CRingFragmentItem(
      static_cast<uint64_t>(10101), static_cast<uint32_t>(1),
      static_cast<uint32_t>(2), reinterpret_cast<void *>(new char[2]),
      static_cast<uint32_t>(3));
}


CRingItem *CTestFilter::handleRingItem(CRingItem *) {
  ++m_nProcessed;
    m_history.push_back("handleRingItem");

  return new CRingItem(100);
}



CRingItem *CTestFilter::handleAbnormalEndItem(CAbnormalEndItem *pItem)
{
    ++m_nProcessed;
    m_history.push_back("handleAbnormalEndItem");
    return new CAbnormalEndItem();
}


CRingItem *CTestFilter::handleGlomParameters(CGlomParameters *pItem)
{
    ++m_nProcessed;
    m_history.push_back("handleGlomParameters");
    return new CGlomParameters(123, true, CGlomParameters::average);
}


CRingItem *CTestFilter::handleDataFormatItem(CDataFormatItem *pItem)
{
    ++m_nProcessed;
    m_history.push_back("handleDataFormatItem");
    return new CDataFormatItem();
}

void CTestFilter::initialize() { m_history.push_back("initialize"); }

void CTestFilter::finalize() { m_history.push_back("finalize"); }

} // end V11
} // end DAQ
