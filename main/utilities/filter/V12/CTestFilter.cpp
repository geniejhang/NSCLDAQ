
#include <V12/CTestFilter.h>

#include <make_unique.h>

#include <vector>
#include <string>

using namespace std;

namespace DAQ {
namespace V12 {

CTestFilter::CTestFilter() : m_history(), m_nProcessed(0) {}

std::vector<std::string> CTestFilter::getHistory() { return m_history; }
int CTestFilter::getNProcessed() { return m_nProcessed; }

CFilterUPtr CTestFilter::clone() const {
    return DAQ::make_unique<CTestFilter>(*this);
}

CRingStateChangeItemPtr CTestFilter::handleStateChangeItem(CRingStateChangeItemPtr ) {
  ++m_nProcessed;
   m_history.push_back("handleStateChangeItem");
  return std::make_shared<CRingStateChangeItem>(BEGIN_RUN);
}

CRingScalerItemPtr CTestFilter::handleScalerItem(CRingScalerItemPtr ) {
  ++m_nProcessed;
    m_history.push_back("handleScalerItem");
  return std::make_shared<CRingScalerItem>(200);
}

CRingTextItemPtr CTestFilter::handleTextItem(CRingTextItemPtr ) {
  ++m_nProcessed;
    m_history.push_back("handleTextItem");
  std::vector<std::string> str_vec;
  str_vec.push_back("0000");
  str_vec.push_back("1111");
  str_vec.push_back("2222");
  return std::make_shared<CRingTextItem>(PACKET_TYPES, str_vec);
}

CPhysicsEventItemPtr CTestFilter::handlePhysicsEventItem(CPhysicsEventItemPtr ) {
  ++m_nProcessed;
    m_history.push_back("handlePhysicsEventItem");
  return std::make_shared<CPhysicsEventItem>();

}

CRingPhysicsEventCountItemPtr CTestFilter::handlePhysicsEventCountItem(CRingPhysicsEventCountItemPtr) {
  ++m_nProcessed;
    m_history.push_back("handlePhysicsEventCountItem");
  return std::make_shared<CRingPhysicsEventCountItem>(static_cast<uint64_t>(4),
                                        static_cast<uint32_t>(1001));
}

CRingItemPtr CTestFilter::handleRingItem(CRingItemPtr ) {
  ++m_nProcessed;
    m_history.push_back("handleRingItem");

  auto pItem = std::make_shared<CRawRingItem>();
  pItem->setType(100);
  return pItem;
}

CAbnormalEndItemPtr CTestFilter::handleAbnormalEndItem(CAbnormalEndItemPtr pItem)
{
    ++m_nProcessed;
    m_history.push_back("handleAbnormalEndItem");
    return std::make_shared<CAbnormalEndItem>();
}

CGlomParametersPtr CTestFilter::handleGlomParameters(CGlomParametersPtr pItem)
{
    ++m_nProcessed;
    m_history.push_back("handleGlomParameters");
    return std::make_shared<CGlomParameters>(123, true, CGlomParameters::average);
}

CDataFormatItemPtr CTestFilter::handleDataFormatItem(CDataFormatItemPtr pItem)
{
    ++m_nProcessed;
    m_history.push_back("handleDataFormatItem");
    return std::make_shared<CDataFormatItem>();
}

void CTestFilter::initialize() { m_history.push_back("initialize"); }

void CTestFilter::finalize() { m_history.push_back("finalize"); }

} // end V12
} // end DAQ
