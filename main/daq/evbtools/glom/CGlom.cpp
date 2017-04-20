#include "CGlom.h"

#include <fragment.h>
#include <fragio.h>


#include <V12/DataFormat.h>
#include <V12/CRingItem.h>
#include <V12/CRingItemFactory.h>
#include <V12/CAbnormalEndItem.h>
#include <V12/CDataFormatItem.h>
#include <V12/CAbnormalEndItem.h>
#include <V12/CGlomParameters.h>
#include <V12/CCompositeRingItem.h>
#include <RingIOV12.h>

#include <CDataSink.h>
#include <CDataSinkFactory.h>
#include <io.h>

#include <iostream>
#include <cstdlib>
#include <exception>

namespace DAQ {

/*!
 * \brief Constructor
 *
 * \param pSink   the data sink
 */
CGlom::CGlom(CDataSinkPtr pSink)
    : m_firstTimestamp(0),
      m_lastTimestamp(0),
      m_timestampSum(0),
      m_sourceId(0),
      m_currentType(V12::UNDEFINED),
      m_nobuild(false),
      m_timestampPolicy(V12::CGlomParameters::first),
      m_stateChangeNesting(0),
      m_firstBarrier(true),
      m_accumulatedItems(),
      m_pSink(pSink)
{
}

/*!
 * \brief Destructor
 *
 * Flush all events that have not been flushed to date
 */
CGlom::~CGlom()
{
    flushEvent();
}

/**
 * outputGlomParameters
 *
 * Output a GlomParameters ring item that describes how we are operating.
 *
 * @param dt - Build time interval.
 * @param building - True if building.
 */
void
CGlom::outputGlomParameters(uint64_t dt, bool building)
{
    V12::CGlomParameters item(dt, building, m_timestampPolicy);
    writeItem(*m_pSink, item);
}

/**
 * flushEvent
 *
 * Flush the items that have been correlated together
 * so far.
 *
 * If nothing has been accumulated, this is a noop.
 *
 * The items that have been correlated together will become the child of
 * a composite ring item. The composite ring item will be given a source
 * id according to m_sourceId and a timestamp based on the timestamp policy.
 * The composite ring item will be outputted.
 *
 */
void
CGlom::flushEvent()
{
    using namespace ::DAQ::V12;
    if (m_accumulatedItems.size() > 0) {

        // Figure out which timestamp to use in the generated event:

        uint64_t eventTimestamp;
        switch (m_timestampPolicy) {
        case CGlomParameters::first :
            eventTimestamp = m_firstTimestamp;
            break;
        case CGlomParameters::last :
            eventTimestamp = m_lastTimestamp;
            break;
        case CGlomParameters::average :
            eventTimestamp = (m_timestampSum/m_accumulatedItems.size());
            break;
        default:
            // Default to earliest...but should not occur:
            eventTimestamp = m_firstTimestamp;
            break;
        }

        CCompositeRingItem builtItem;
        builtItem.setType(COMPOSITE_BIT | m_accumulatedItems.front()->type());
        builtItem.setEventTimestamp(eventTimestamp);
        builtItem.setSourceId(m_sourceId);

        builtItem.setChildren(m_accumulatedItems);

        writeItem(*m_pSink, builtItem);

        if (builtItem.type() == COMP_BEGIN_RUN)       m_stateChangeNesting++;
        if (builtItem.type() == COMP_END_RUN)         m_stateChangeNesting--;
        if (builtItem.type() == COMP_ABNORMAL_ENDRUN) m_stateChangeNesting = 0;

        m_currentType       = UNDEFINED;
        m_accumulatedItems.clear();
    }
}

/**
 * emitAbnormalEnd
 *    Emits an abnormal end run item.
 */
void CGlom::emitAbnormalEnd()
{
    V12::CAbnormalEndItem item;
    writeItem(*m_pSink, item);
}

/**
 * acumulateEvent
 *
 * Append ring item to the set of accumulated items that correlate together.
 * If it is the first,
 * store the timestamp and type and also reset the timestamp sum. For every
 * time this is called, the timestamp sum and last timestamp are updated.
 *
 * @param dt - Coincidence interval in timestamp ticks.
 * @param pFrag - Pointer to the next event fragment.
 */
void
CGlom::accumulateEvent(uint64_t dt, V12::CRingItemPtr pItem)
{
    uint64_t timestamp = pItem->getEventTimestamp();

    // Reset/set some data to be ready for next set of correlatable items
    if (m_accumulatedItems.size() == 0) {
        m_firstTimestamp = timestamp;
        m_timestampSum   = 0;
        m_currentType    = pItem->type();
    }
    m_lastTimestamp    = timestamp;
    m_timestampSum    += timestamp;

    m_accumulatedItems.push_back(pItem);

}

/*!
 * \brief CGlom::outputEventFormat
 *
 * Write a generic CDataFormatItem to the sink.
 */
void CGlom::outputEventFormat()
{
    V12::CDataFormatItem format;
    writeItem(*m_pSink, format);
}

/**
 * \brief Handle ring items
 *
 *  This function is the meat of the program.  It
 *  manages whether data should be flushed or correlated
 *  with other items before it.
 *
 *  Once the event timestamp exceeds the coincidence
 *  interval from firstTimestamp, the event is flushed
 *  and the process starts all over again.
 *
 * If a begin run is observed and it is the first during the
 * program or since an end run, an EVB_GLOM_INFO item will be outputted.
 *
 * @param dt - Coincidence interval in timestamp ticks.
 * @param pFrag - Pointer to the next event fragment.
 */
void CGlom::handleItem(V12::CRingItemPtr pItem)
{
    if ( pItem->type() == V12::BEGIN_RUN && m_firstBarrier) {
        flushEvent();
        outputGlomParameters(m_dtInt, m_nobuild);
        m_firstBarrier = false;
    }

    // in case the glom program run persistently over many runs, it is important that
    // a new glom parameters item be outputted at the start of the next run. Therefore,
    // arm the logic for outputting the glom info when an end run item is observed.
    if ( pItem->type() == V12::END_RUN) {
        m_firstBarrier = true;
    }

    bool firstEvent = (m_accumulatedItems.size() == 0);
    if (m_nobuild
            || (!firstEvent && ((pItem->getEventTimestamp() - m_firstTimestamp) > m_dtInt))
            || (!firstEvent && (( 0x7fff & m_currentType) != (0x7fff & pItem->type()) ))
            ) {
        flushEvent();
    }

    accumulateEvent(m_dtInt, pItem);
}


/*!
 * \brief Main loop
 *
 * In the call operator (i.e. the main loop), the sequence:
 *
 * 1. Read a fragment
 * 2. handle item
 *
 * Will be repeated over and over again until either an error occurs or CFragIO::readFragment
 * returns a nullptr. The latter condition occurs for an EOF condition.
 */
void CGlom::operator ()()
{
    /*
     main loop.. .get fragments and handle them.
     accumulateEvent - for non-barriers.
  */

    while (1) {
        std::unique_ptr<EVB::Fragment> p(CFragIO::readFragment(STDIN_FILENO));

        // If error or EOF flush the event and break from
        // the loop:

        if (!p) {
            flushEvent();
            std::cerr << "glom: EOF on input\n";
            if(m_stateChangeNesting) {
                emitAbnormalEnd();
            }
            break;
        }

        V12::CRingItemPtr pItem = V12::CRingItemFactory::createRingItem(p->s_pBody,
                                                                        p->s_pBody + p->s_header.s_size);

        handleItem(pItem);
    }

}


void CGlom::disableBuilding(bool nobuild)
{
  m_nobuild = nobuild;
}

void CGlom::setCorrelationTime(uint64_t dt)
{
    m_dtInt = dt;
}

void CGlom::setTimestampPolicy(V12::CGlomParameters::TimestampPolicy policy)
{
    m_timestampPolicy = policy;
}

void CGlom::setFirstBarrier(bool expectingBarrier)
{
    m_firstBarrier = expectingBarrier;
}


} // end DAQ
