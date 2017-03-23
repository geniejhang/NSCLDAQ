
#ifndef DAQ_V12_CSOURCECOUNTERFILTER_H
#define DAQ_V12_CSOURCECOUNTERFILTER_H

#include <V12/CFilter.h>
#include <V12/DataFormat.h>
#include <V12/CRingItem.h>
#include <V12/CRingStateChangeItem.h>
#include <V12/CPhysicsEventItem.h>
#include <V12/CRingTextItem.h>
#include <V12/CRingItemFactory.h>
#include <V12/CRingPhysicsEventCountItem.h>

#include <make_unique.h>

#include <stdint.h>
#include <map>
#include <string>
#include <iosfwd>
#include <algorithm>
#include <memory>

namespace DAQ {
namespace V12 {

/*!
 * \brief The CSourceCounterFilter class
 *
 * The CSourceCounterFilter is responsible for processing a stream of
 * ring items and keeping track of how many of each item type was observed for
 * each source id. Composite ring items are processed one build layer deep.
 *
 * The class is designed to function in a V12 filter program. The finalize
 * method causes the result file to be written.
 */
class CSourceCounterFilter : public CFilter
{
  private:
    std::map<uint32_t, std::map<uint32_t, uint32_t> > m_counters;

    std::string  m_outputFile;
    bool         m_builtData;


  public:

    CSourceCounterFilter(std::string outputFile);
    virtual ~CSourceCounterFilter();

    CFilterUPtr clone() const { return DAQ::make_unique<CSourceCounterFilter>(*this);}

    void setBuiltData(bool val) { m_builtData = val;}

    // The default handlers
    virtual CRingItemPtr handleRingItem(CRingItemPtr pItem);
    virtual CRingStateChangeItemPtr handleStateChangeItem(CRingStateChangeItemPtr pItem);
    virtual CRingScalerItemPtr handleScalerItem(CRingScalerItemPtr pItem);
    virtual CRingTextItemPtr handleTextItem(CRingTextItemPtr pItem);
    virtual CPhysicsEventItemPtr handlePhysicsEventItem(CPhysicsEventItemPtr pItem);
    virtual CRingPhysicsEventCountItemPtr
      handlePhysicsEventCountItem(CRingPhysicsEventCountItemPtr pItem);
    virtual CDataFormatItemPtr handleDataFormatItem(CDataFormatItemPtr pItem);
    virtual CAbnormalEndItemPtr handleAbnormalEndItem(CAbnormalEndItemPtr pItem);

    virtual CCompositeRingItemPtr handleCompositeItem(CCompositeRingItemPtr pItem);
    virtual void finalize();

  private:
    bool counterExists(uint32_t type);
    void setupCounters(uint32_t id);
    void incrementCounter(CRingItem* pItem);
    void incrementCounter(uint32_t id, uint32_t type);

    void printCounters(std::ostream& stream) const;
    std::string translate(uint32_t type) const;
  

    template<class ItemPtr> ItemPtr handleItem(ItemPtr pItem);
};


template<class ItemPtr>
ItemPtr CSourceCounterFilter::handleItem(ItemPtr pItem)
{
    incrementCounter(pItem->getSourceId(), pItem->type());
    return pItem;
}


} // end V12
} // end DAQ

#endif


