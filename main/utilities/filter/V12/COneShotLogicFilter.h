#ifndef DAQ_V12_CONESHOTLOGICFILTER_H
#define DAQ_V12_CONESHOTLOGICFILTER_H

#include <COneShotHandler.h>
#include <V12/CFilter.h>
#include <make_unique.h>

namespace DAQ {
namespace V12 {

// Forward declarations
class CFilterAbstraction;
class COneShotLogicFilter;

// Typedefs for smart pointers
using COneShotLogicFilterUPtr = std::unique_ptr<COneShotLogicFilter>;
using COneShotLogicFilterPtr  = std::shared_ptr<COneShotLogicFilter>;


/*!
 * \brief Filter for implementing the oneshot logic
 *
 * The bookkeeping for the oneshot logic is implemented in DAQ::COneShotHandler.
 * This provides the extra logic to feed the oneshot handler with its data
 * and also to take actions when certain criteria are determined by the oneshot
 * handler. There is a similar class called DAQ::V11::COneShotLogicFilter.
 *
 * If the user selects the --oneshot option when invoking their filter, a filter
 * of this type will be added to the composite filter in use.
 */
class COneShotLogicFilter : public CFilter
{
private:
    COneShotHandler m_handler;
    CFilterAbstraction* m_pAbstraction;

public:
    COneShotLogicFilter(int nSources, CFilterAbstraction& abstraction);
    COneShotLogicFilter(const COneShotLogicFilter& ) = default;
    CFilterUPtr clone() const {
        return DAQ::make_unique<COneShotLogicFilter>(*this);
    }

    CRingItemPtr handleRingItem(CRingItemPtr pItem);

    CAbnormalEndItemPtr handleAbnormalEndItem(CAbnormalEndItemPtr pItem);
    CDataFormatItemPtr handleDataFormatItem(CDataFormatItemPtr pItem);
    CGlomParametersPtr handleGlomParameters(CGlomParametersPtr pItem);
    CRingPhysicsEventCountItemPtr handlePhysicsEventCountItem(CRingPhysicsEventCountItemPtr pItem);
    CPhysicsEventItemPtr handlePhysicsEventItem(CPhysicsEventItemPtr pItem);
    CRingScalerItemPtr handleScalerItem(CRingScalerItemPtr pItem);
    CRingStateChangeItemPtr handleStateChangeItem(CRingStateChangeItemPtr pItem);
    CRingTextItemPtr handleTextItem(CRingTextItemPtr pItem);
    CCompositeRingItemPtr handleCompositeItem(CCompositeRingItemPtr pItem);

    const COneShotHandler& getOneShotLogic() const;

private:
    template<class T> T handleItem(T pItem);

};

/*! \brief Generic handler for most item types.
 *
 * The code here embodies the logic that if no begin item type has been observed
 * so far, nullptr will be returned. Otherwise, the item itself will be returned.
 *
 * All template parameters will be a type of shared_ptr.
 *
 * \param pItem     is a shared pointer type holder a pointer to a specialized ring item
 *
 * \retval nullptr (i.e. empty shared_ptr)
 * \retval the same pointer passed in.
 */
template<class T>
T COneShotLogicFilter::handleItem(T pItem)  {
    if (m_handler.waitingForBegin()) {
        return T(); // return a null shared_ptr
    } else {
        return pItem;
    }
}

} // end V12
} // end DAQ

#endif // CONESHOTLOGICFILTER_H
