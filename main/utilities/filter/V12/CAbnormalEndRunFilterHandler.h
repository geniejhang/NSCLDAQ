

#ifndef DAQ_V12_ABNORMALENDRUNFILTERHANDLER_H
#define DAQ_V12_ABNORMALENDRUNFILTERHANDLER_H

#include <V12/CFilter.h>
#include <stdexcept>

namespace DAQ {

class CDataSink;

namespace V12 {

class CRingItem;
class CAbnormalEndItem;


/**! \brief Filter providing logic for handling ABNORMAL_ENDRUN items
 *
 * The ABNORMAL_ENDRUN item is supposed to be outputted when something
 * bad has happened. Its purpose is the flush through the data stream
 * and kill off every process it encounters. So the unique thing about this
 * is that once observed, it must be passed on and only then can the
 * process exit. THis does essentially that. It is kind of specialized
 * because it takes matters into its own hands by performing the write
 * to the data sink itself. The user should set this up by doing the 
 * following in their main:
 *
 * #include <CAbnormalEndRunFilterHandler.h>
 * #include <CMediator.h>
 *
 * int main(int argc, char* argv[]) {
 *  // ...
 * CFilterMain theApp(argc,argv);
 * CAbnormalEndRunFilterHandler abnHandler(*(theApp.getMediator()->getDataSink()));
 * main.registerFilter(&abnHandler);
 *  // ...
 * }
 */
class CAbnormalEndRunFilterHandler : public CFilter 
{

  private:
    DAQ::CDataSink& m_sink;

  public:
    CAbnormalEndRunFilterHandler(DAQ::CDataSink& sink )
       : m_sink(sink) {}

    CAbnormalEndRunFilterHandler(const CAbnormalEndRunFilterHandler& rhs);

    CFilterUPtr clone() const;

  private:
    CAbnormalEndRunFilterHandler& operator=(const CAbnormalEndRunFilterHandler& rhs);

    /*! \brief Checks for ABNORMAL_ENDRUN presence
     *
     * If the ring item is an ABNORMAL_ENDRUN, it sets a flag to ensure
     * that an exception is thrown on the next iteration. Note that 
     * the next iteration may not come... in which case this would probably
     * just exit normally.
     *
     */
    template<class RingPtr>
    RingPtr handleAnyRingItem(RingPtr pItem);
    CRingStateChangeItemPtr handleStateChangeItem(CRingStateChangeItemPtr pItem);
    CRingScalerItemPtr handleScalerItem(CRingScalerItemPtr pItem);
    CRingTextItemPtr handleTextItem(CRingTextItemPtr pItem);
    CPhysicsEventItemPtr handlePhysicsEventItem(CPhysicsEventItemPtr pItem);
    CRingPhysicsEventCountItemPtr handlePhysicsEventCountItem(CRingPhysicsEventCountItemPtr pItem);
    CGlomParametersPtr handleGlomParameters(CGlomParametersPtr pItem);
    CCompositeRingItemPtr handleCompositeItem(CCompositeRingItemPtr pItem);
    CDataFormatItemPtr handleDataFormatItem(CDataFormatItemPtr pItem);
    CAbnormalEndItemPtr handleAbnormalEndItem(CAbnormalEndItemPtr item);

};

// All of the functionality of the other are in the handleRingItem.
template<class RingPtr>
RingPtr
CAbnormalEndRunFilterHandler::handleAnyRingItem(RingPtr pItem) {

    if (pItem->type() == ABNORMAL_ENDRUN) {
        writeItem(m_sink, *pItem);

        throw std::runtime_error("Found an abnormal end run item. Shutting down!");
    }

    return pItem;
}


} // end V12
} // end DAQ
#endif
