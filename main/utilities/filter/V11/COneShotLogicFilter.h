#ifndef DAQ_V11_CONESHOTLOGICFILTER_H
#define DAQ_V11_CONESHOTLOGICFILTER_H

#include <COneShotHandler.h>
#include <V11/CFilter.h>

namespace DAQ {
namespace V11 {

// forward declarations
class CFilterAbstraction;
class COneShotLogicFilter;

// Some convenient typedefs
using COneShotLogicFilterUPtr = std::unique_ptr<COneShotLogicFilter>;
using COneShotLogicFilterPtr  = std::shared_ptr<COneShotLogicFilter>;

/*!
 * \brief One shot logic specific to V11 data
 *
 * The COneShotLogicFilter is a filter that processes data and
 * helps implement the logic for the --oneshot option in filters.
 * The bookkeeping is done by an object of the COneShotHandler class.
 *
 * If the user selects the --oneshot option, a filter of this type will
 * be added to the set of filters run.
 */
class COneShotLogicFilter : public CFilter
{
private:
    COneShotHandler m_handler;
    CFilterAbstraction* m_pAbstraction;

public:
    COneShotLogicFilter(int nSources, CFilterAbstraction& abstraction);
    COneShotLogicFilter(const COneShotLogicFilter& ) = default;
    COneShotLogicFilter* clone() const {
        return new COneShotLogicFilter(*this);
    }

    CRingItem* handleRingItem(CRingItem* pItem);

    CRingItem* handleAbnormalEndItem(CAbnormalEndItem* pItem);
    CRingItem* handleDataFormatItem(CDataFormatItem *pItem);
    CRingItem* handleFragmentItem(CRingFragmentItem *pItem);
    CRingItem* handleGlomParameters(CGlomParameters *pItem);
    CRingItem* handlePhysicsEventCountItem(CRingPhysicsEventCountItem *pItem);
    CRingItem* handlePhysicsEventItem(CPhysicsEventItem *pItem);
    CRingItem* handleScalerItem(CRingScalerItem *pItem);
    CRingItem* handleStateChangeItem(CRingStateChangeItem *pItem);
    CRingItem* handleTextItem(CRingTextItem *pItem);

    const COneShotHandler& getOneShotLogic() const;

};

} // end V11
} // end DAQ

#endif // CONESHOTLOGICFILTER_H
