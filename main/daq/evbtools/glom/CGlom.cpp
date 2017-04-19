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

CGlom::CGlom(CDataSinkPtr pSink)
    : m_firstTimestamp(0),
      m_lastTimestamp(0),
      m_timestampSum(0),
      m_sourceId(0),
      m_fragmentCount(0),
      m_currentType(V12::UNDEFINED),
      m_firstEvent(true),
      m_nobuild(false),
      m_timestampPolicy(V12::CGlomParameters::first),
      m_stateChangeNesting(0),
      m_firstBarrier(true),
      m_accumulatedItems(),
      m_pSink(pSink)
{
}


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
 * Flush the physics event that has been accumulated
 * so far.
 *
 * If nothing has been accumulated, this is a noop.
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
            eventTimestamp = (m_timestampSum/m_fragmentCount);
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

        m_firstEvent        = true;
        m_fragmentCount     = 0;
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
 *  This function is the meat of the program.  It
 *  glues fragments together (header and payload)
 *  into a dynamically resized chunk of memory pointed
 *  to by pAccumulatedEvent where  totalEventSize
 *  is the number of bytes that have been accumulated
 *  so far.
 *
 *  firstTimestamp is the timestamp of the first fragment
 *  in the acccumulated data.though it is only valid if
 *  firstEvent is false.
 *
 *  Once the event timestamp exceeds the coincidence
 *  interval from firstTimestamp, the event is flushed
 *  and the process starts all over again.
 *
 * @param dt - Coincidence interval in timestamp ticks.
 * @param pFrag - Pointer to the next event fragment.
 */
void
CGlom::accumulateEvent(uint64_t dt, V12::CRingItemPtr pItem)
{
    uint64_t timestamp = pItem->getEventTimestamp();

    // If firstEvent...our timestamp starts the interval:

    if (m_firstEvent) {
        m_firstTimestamp = timestamp;
        m_firstEvent     = false;
        m_fragmentCount  = 0;
        m_timestampSum   = 0;
        m_currentType    = pItem->type();
    }
    m_lastTimestamp    = timestamp;
    m_fragmentCount++;
    m_timestampSum    += timestamp;

    // Figure out how much we're going to add to the
    // event:

    m_accumulatedItems.push_back(pItem);

}

void CGlom::outputEventFormat()
{
    V12::CDataFormatItem format;
    writeItem(*m_pSink, format);
}


void CGlom::handleItem(V12::CRingItemPtr pItem)
{
    if ( pItem->type() == V12::BEGIN_RUN && m_firstBarrier) {
        outputGlomParameters(m_dtInt, m_nobuild);
        m_firstBarrier = false;
    }

    if (m_nobuild
            || (!m_firstEvent && ((pItem->getEventTimestamp() - m_firstTimestamp) > m_dtInt))
            || (!m_firstEvent && (( 0x7fff & m_currentType) != (0x7fff & pItem->type()) ))
            ) {
        flushEvent();
    }

    accumulateEvent(m_dtInt, pItem);
}

int CGlom::operator ()()
{
    /*
     main loop.. .get fragments and handle them.
     accumulateEvent - for non-barriers.
  */

    m_fragmentCount = 0;
    try {
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
            // We have a fragment:

            V12::CRingItemPtr pItem = V12::CRingItemFactory::createRingItem(p->s_pBody,
                                                                            p->s_pBody + p->s_header.s_size);

            handleItem(pItem);
        }
    }
    catch (std::string msg) {
        std::cerr << "glom: " << msg << std::endl;
    }
    catch (const char* msg) {
        std::cerr << "glom: " << msg << std::endl;
    }
    catch (int e) {
        std::string msg = "glom: Integer error: ";
        msg += strerror(e);
        std::cerr << msg << std::endl;
    }
    catch (std::exception& except) {
        std::string msg = "glom: ";
        msg += except.what();
        std::cerr << msg << std::endl;
    }
    catch(...) {
        std::cerr << "Unanticipated exception caught\n";

    }
    // Out of main loop because we need to exit.

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
