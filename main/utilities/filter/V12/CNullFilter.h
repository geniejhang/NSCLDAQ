

#ifndef DAQ_V12_CNULLFILTER_H
#define DAQ_V12_CNULLFILTER_H

#include <V12/CFilter.h>
#include <make_unique.h>

namespace DAQ {
namespace V12 {

class CNullFilter;
using CNullFilterUPtr = std::unique_ptr<CNullFilter>;
using CNullFilterPtr  = std::shared_ptr<CNullFilter>;

/**! A filter whose handlers always return NULL.
*
* This is really intended for use in testing. 
*/
class CNullFilter : public CFilter {
  public :
    CFilterUPtr clone() const { return make_unique<CNullFilter>(*this); }

    CRingScalerItemPtr handleScalerItem(CRingScalerItemPtr) {
      return 0;
    }

    CRingTextItemPtr handleTextItem(CRingTextItemPtr) {
      return 0;
    }

    CRingPhysicsEventCountItemPtr handlePhysicsEventCountItem(CRingPhysicsEventCountItemPtr) {
      return 0;
    }

    CPhysicsEventItemPtr handlePhysicsEventItem(CPhysicsEventItemPtr) {
      return 0;
    }
    CRingStateChangeItemPtr handleStateChangeItem(CRingStateChangeItemPtr) {
      return 0;
    }

    CRingItemPtr handleRingItem(CRingItemPtr) {
      return 0;
    }

    CDataFormatItemPtr handleDataFormatItem(CDataFormatItemPtr) {
        return nullptr;
    }

    CAbnormalEndItemPtr handleAbnormalEndItem(CAbnormalEndItemPtr pItem)
    {
        return nullptr;
    }

    CCompositeRingItemPtr handleCompositeItem(CCompositeRingItemPtr pItem)
    {
        return nullptr;
    }
};


} // end V12
} // end DAQ


#endif
