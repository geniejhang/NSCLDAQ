

#ifndef DAQ_V11_CNULLFILTER_H
#define DAQ_V11_CNULLFILTER_H

#include <V11/CFilter.h>

namespace DAQ {
namespace V11 {

class CNullFilter;
using CNullFilterUPtr = std::unique_ptr<CNullFilter>;
using CNullFilterPtr = std::shared_ptr<CNullFilter>;

/**! A filter whose handlers always return NULL.
*
* This is really intended for use in testing. 
*/
class CNullFilter : public CFilter {
  public :
    CNullFilter* clone() const { return new CNullFilter(*this); }

    CRingItem* handleScalerItem(CRingScalerItem*) {
      return 0;
    }

    CRingItem* handleTextItem(CRingTextItem*) {
      return 0;
    }

    CRingItem* handleFragmentItem(CRingFragmentItem*) {
      return 0;
    }
    CRingItem* handlePhysicsEventCountItem(CRingPhysicsEventCountItem*) {
      return 0;
    }

    CRingItem* handlePhysicsEventItem(CPhysicsEventItem*) {
      return 0;
    }
    CRingItem* handleStateChangeItem(CRingStateChangeItem*) {
      return 0;
    }

    CRingItem* handleRingItem(CRingItem*) {
      return 0;
    }

};


} // end V11
} // end DAQ


#endif
