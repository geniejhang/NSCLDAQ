#ifndef DAQ_V12_CONESHOTLOGICFILTER_H
#define DAQ_V12_CONESHOTLOGICFILTER_H

#include <COneShotHandler.h>
#include <V12/CFilter.h>
#include <make_unique.h>

namespace DAQ {
namespace V12 {

class CFilterAbstraction;

class COneShotLogicFilter;
using COneShotLogicFilterUPtr = std::unique_ptr<COneShotLogicFilter>;
using COneShotLogicFilterPtr  = std::shared_ptr<COneShotLogicFilter>;

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

    template<class T> T handleItem(T pItem);

    const COneShotHandler& getOneShotLogic() const;

};


template<class T>
T COneShotLogicFilter::handleItem(T pItem)  {
    if (m_handler.waitingForBegin()) {
        return T();
    } else {
        return pItem;
    }
}

} // end V12
} // end DAQ

#endif // CONESHOTLOGICFILTER_H
