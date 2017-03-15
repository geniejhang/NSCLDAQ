#ifndef DAQ_V11_CFILTER_H
#define DAQ_V11_CFILTER_H

#include <V11/CRingItem.h>
#include <V11/CRingStateChangeItem.h>
#include <V11/CRingScalerItem.h>
#include <V11/CRingTextItem.h>
#include <V11/CPhysicsEventItem.h>
#include <V11/CRingPhysicsEventCountItem.h>
#include <V11/CRingFragmentItem.h>
#include <V11/CDataFormatItem.h>
#include <V11/CAbnormalEndItem.h>
#include <V11/CGlomParameters.h>

#include <memory>

namespace DAQ {
namespace V11 {

class CFilter;

using CFilterUPtr = std::unique_ptr<CFilter>;
using CFilterPtr  = std::shared_ptr<CFilter>;

/**! \class Version 11 CFilter
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

    // Virtual constructor
    virtual CFilter* clone() const=0;

    // The default handlers
    virtual CRingItem* handleRingItem(CRingItem* pItem)
    {
       return pItem;
    }

    virtual CRingItem* handleStateChangeItem(CRingStateChangeItem* pItem)
    {
       return static_cast<CRingItem*>(pItem);
    }

    virtual CRingItem* handleScalerItem(CRingScalerItem* pItem)
    {
       return static_cast<CRingItem*>(pItem);
    }

    virtual CRingItem* handleTextItem(CRingTextItem* pItem)
    {
       return static_cast<CRingItem*>(pItem);
    }

    virtual CRingItem* handlePhysicsEventItem(CPhysicsEventItem* pItem)
    {
       return static_cast<CRingItem*>(pItem);
    }

    virtual CRingItem*
    handlePhysicsEventCountItem(CRingPhysicsEventCountItem* pItem)
    {
       return static_cast<CRingItem*>(pItem);
    }

    virtual CRingItem* handleFragmentItem(CRingFragmentItem* pItem)
    {
       return static_cast<CRingItem*>(pItem);
    }

    virtual CRingItem* handleAbnormalEndItem(CAbnormalEndItem* pItem)
    {
        return pItem;
    }

    virtual CRingItem* handleGlomParameters(CGlomParameters* pItem)
    {
        return pItem;
    }

    virtual CRingItem* handleDataFormatItem(CDataFormatItem* pItem)
    {
        return pItem;
    }

    // Initialization procedures to run before any ring items are processed
    virtual void initialize() {}
    // Finalization procedures to run after all ring items have been processed
    virtual void finalize() {}
};



} // end V11
} // end DAQ

#endif // DAQ_V11_CFILTER_H
