#ifndef DAQ_V12_CTESTFILTER_H
#define DAQ_V12_CTESTFILTER_H

#include <V12/CFilter.h>

#include <string>
#include <vector>

namespace DAQ {
namespace V12 {

// forward declarations
class CRingItem;
class CTestFilter;

// Some useful smart pointer typedefs
using CTestFilterUPtr = std::unique_ptr<CTestFilter>;
using CTestFilterPtr  = std::shared_ptr<CTestFilter>;

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
