

#ifndef DAQ_V12_CNULLFILTER_H
#define DAQ_V12_CNULLFILTER_H

#include <V12/CFilter.h>
#include <make_unique.h>

namespace DAQ {
namespace V12 {

// forward declarations
class CNullFilter;

// Useful typedefs for smart pointers
using CNullFilterUPtr = std::unique_ptr<CNullFilter>;
using CNullFilterPtr  = std::shared_ptr<CNullFilter>;

/**! A filter whose handlers always return nullptr.
*
* This is really intended for use in testing. 
*/
class CNullFilter : public CFilter {
  public :
    CFilterUPtr clone() const { return make_unique<CNullFilter>(*this); }

    CRingScalerItemPtr handleScalerItem(CRingScalerItemPtr) {
      return nullptr;
    }

    CRingTextItemPtr handleTextItem(CRingTextItemPtr) {
      return nullptr;
    }

    CRingPhysicsEventCountItemPtr handlePhysicsEventCountItem(CRingPhysicsEventCountItemPtr) {
      return nullptr;
    }

    CPhysicsEventItemPtr handlePhysicsEventItem(CPhysicsEventItemPtr) {
      return nullptr;
    }
    CRingStateChangeItemPtr handleStateChangeItem(CRingStateChangeItemPtr) {
      return nullptr;
    }

    CRingItemPtr handleRingItem(CRingItemPtr) {
      return nullptr;
    }

    CDataFormatItemPtr handleDataFormatItem(CDataFormatItemPtr) {
        return nullptr;
    }

    CAbnormalEndItemPtr handleAbnormalEndItem(CAbnormalEndItemPtr pItem)
    {
        return nullptr;
    }

    CGlomParametersPtr handleGlomParameters(CGlomParametersPtr pItem)
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
