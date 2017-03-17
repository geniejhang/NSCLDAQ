#ifndef DAQ_V12_CFILTER_H
#define DAQ_V12_CFILTER_H

#include <V12/CRingItem.h>
#include <V12/CRingStateChangeItem.h>
#include <V12/CRingScalerItem.h>
#include <V12/CRingTextItem.h>
#include <V12/CPhysicsEventItem.h>
#include <V12/CRingPhysicsEventCountItem.h>
#include <V12/CCompositeRingItem.h>
#include <V12/CGlomParameters.h>
#include <V12/CAbnormalEndItem.h>
#include <V12/CDataFormatItem.h>

namespace DAQ {
namespace V12 {


class CFilter;
using CFilterUPtr = std::unique_ptr<CFilter>;
using CFilterPtr  = std::shared_ptr<CFilter>;


/**! \class Version 12 CFilter
 *
  This class is a base class for all filter objects. It provides a
  default definitions for all of its methods that does nothing more
  than return the pointer passed as an argument. For this reason, one
  could consider this as a transparent filter that can be inserted into
  the data stream.

  All derived methods must return a newly allocated CRingItem. There is
  a one-to-one relationship between object input to the filter and objects
  output from the filter. The user is not responsible for delete the
  object passed into each method. In fact, doing so will cause a segmentation
  fault.
*/
class CFilter
{
  public:

    // Virtual base class destructor
    virtual ~CFilter() {}

    virtual CFilterUPtr clone() const = 0;

    // The default handlers
    virtual CRingItemPtr handleRingItem(CRingItemPtr pItem)
    {
       return pItem;
    }

    virtual CRingStateChangeItemPtr handleStateChangeItem(CRingStateChangeItemPtr pItem)
    {
       return pItem;
    }

    virtual CRingScalerItemPtr handleScalerItem(CRingScalerItemPtr pItem)
    {
       return pItem;
    }

    virtual CRingTextItemPtr handleTextItem(CRingTextItemPtr pItem)
    {
       return pItem;
    }

    virtual CPhysicsEventItemPtr handlePhysicsEventItem(CPhysicsEventItemPtr pItem)
    {
       return pItem;
    }

    virtual CRingPhysicsEventCountItemPtr
    handlePhysicsEventCountItem(CRingPhysicsEventCountItemPtr pItem)
    {
       return pItem;
    }

    virtual CGlomParametersPtr handleGlomParameters(CGlomParametersPtr pItem) {
        return pItem;
    }

    virtual CCompositeRingItemPtr handleCompositeItem(CCompositeRingItemPtr pItem)
    {
        return pItem;
    }

    virtual CDataFormatItemPtr handleDataFormatItem(CDataFormatItemPtr pItem)
    {
        return pItem;
    }

    virtual CAbnormalEndItemPtr handleAbnormalEndItem(CAbnormalEndItemPtr pItem)
    {
        return pItem;
    }

    // Initialization procedures to run before any ring items are processed
    virtual void initialize() {}
    // Finalization procedures to run after all ring items have been processed
    virtual void finalize() {}
};




} // V12
} // DAQ

#endif // DAQ_V12_CFILTER_H
