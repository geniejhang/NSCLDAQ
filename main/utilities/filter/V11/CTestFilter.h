#ifndef DAQ_V11_CTESTFILTER_H
#define DAQ_V11_CTESTFILTER_H

#include <V11/CFilter.h>

#include <string>
#include <vector>

namespace DAQ {
namespace V11 {

class CRingItem;
class CTestFilter;

// Some useful typedefs
using CTestFilterUPtr = std::unique_ptr<CTestFilter>;
using CTestFilterPtr = std::shared_ptr<CTestFilter>;


/*!
 * \brief The CTestFilter class
 *
 * The CTestFilter class keeps track of the number of items processed
 * and also a record of the call history. The call history can be
 * retrieved with the getHistory() method.
 *
 * Also, each handler method returns a newly allocated object to the caller
 * that has the same characteristics every time. See the implementation of each
 * method for the details of each returned type.
 */
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
