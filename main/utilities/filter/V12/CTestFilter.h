#ifndef DAQ_V12_CTESTFILTER_H
#define DAQ_V12_CTESTFILTER_H

#include <V12/CFilter.h>

#include <string>
#include <vector>

namespace DAQ {
namespace V12 {

class CRingItem;

class CTestFilter;
using CTestFilterUPtr = std::unique_ptr<CTestFilter>;
using CTestFilterPtr  = std::shared_ptr<CTestFilter>;


class CTestFilter : public CFilter {
private:
  std::vector<std::string> m_history;
  int m_nProcessed;

public:
  CTestFilter();

  std::vector<std::string> getHistory();
  int getNProcessed();

  virtual CFilterUPtr clone() const;

  virtual CRingStateChangeItemPtr handleStateChangeItem(CRingStateChangeItemPtr);

  virtual CRingScalerItemPtr handleScalerItem(CRingScalerItemPtr );

  virtual CRingTextItemPtr handleTextItem(CRingTextItemPtr);

  virtual CPhysicsEventItemPtr handlePhysicsEventItem(CPhysicsEventItemPtr );

  virtual CRingPhysicsEventCountItemPtr handlePhysicsEventCountItem(CRingPhysicsEventCountItemPtr );

  virtual CRingItemPtr handleRingItem(CRingItemPtr);

  virtual CAbnormalEndItemPtr handleAbnormalEndItem(CAbnormalEndItemPtr pItem);
  virtual CDataFormatItemPtr handleDataFormatItem(CDataFormatItemPtr pItem);
  virtual CGlomParametersPtr handleGlomParameters(CGlomParametersPtr pItem);

  virtual void initialize();
  virtual void finalize();
};

} // end V12
} // end DAQ

#endif
