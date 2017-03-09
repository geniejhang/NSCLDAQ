#ifndef DAQ_V11_CTESTFILTER_H
#define DAQ_V11_CTESTFILTER_H

#include <V11/CFilter.h>

#include <string>
#include <vector>

namespace DAQ {
namespace V11 {

class CRingItem;

class CTestFilter;
using CTestFilterUPtr = std::unique_ptr<CTestFilter>;
using CTestFilterPtr = std::shared_ptr<CTestFilter>;


class CTestFilter : public CFilter {
private:
  std::vector<std::string> m_history;
  int m_nProcessed;

public:
  CTestFilter();

  std::vector<std::string> getHistory();
  int getNProcessed();

  virtual CTestFilter *clone() const;

  virtual CRingItem *handleStateChangeItem(CRingStateChangeItem *);

  virtual CRingItem *handleScalerItem(CRingScalerItem *);

  virtual CRingItem *handleTextItem(CRingTextItem *);

  virtual CRingItem *handlePhysicsEventItem(CPhysicsEventItem *);

  virtual CRingItem *handlePhysicsEventCountItem(CRingPhysicsEventCountItem *);

  virtual CRingItem *handleFragmentItem(CRingFragmentItem *);
  virtual CRingItem *handleRingItem(CRingItem *);

  virtual CRingItem *handleAbnormalEndItem(CAbnormalEndItem *pItem);
  virtual CRingItem *handleDataFormatItem(CDataFormatItem *pItem);
  virtual CRingItem *handleGlomParameters(CGlomParameters *pItem);

  virtual void initialize();
  virtual void finalize();
};

} // end V11
} // end DAQ

#endif
