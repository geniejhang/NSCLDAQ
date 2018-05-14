/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
/**
 * @file TestSource.cpp
 * @brief Implement the TestSource class.
 */
#include "TestSource.h"
#include <CDataSink.h>
#include <CDataSinkFactory.h>

#include <V12/CPhysicsEventItem.h>
#include <V12/CDataFormatItem.h>
#include <V12/CRingStateChangeItem.h>
#include <V12/CRingScalerItem.h>
#include <RingIOV12.h>

#include <os.h>

namespace DAQ {

/**
 * operator()
 *
 *  Entry point to the test.
 *  - Connect to the ring.
 *  - Insert a begin run, some data, some scalers and an end run.
 *  - Disconnect from the ring.
 */
void
TestSource::operator()() 
{
  CDataSinkPtr pRing(CDataSinkFactory().makeSink(m_ringName)); // Connect to the ring.
  dataFormat(*pRing);
  beginRun(*pRing, 1234, "This is the begin run");
  for (int i = 0; i < 1000; i++) {
    someEventData(*pRing, 500);
    Scaler(*pRing, 32, 5);
  }
  endRun(*pRing,  1234, "This is the end run");
}

/*------------------------------------------------------------------------------
** Private utilities.
*/

void
TestSource::dataFormat(CDataSink &ring)
{
  V12::CDataFormatItem format;
  writeItem(ring, format);
}

void
TestSource::beginRun(CDataSink &ring, int run, std::string title)
{
  m_elapsedTime = 0;

  V12::CRingStateChangeItem begin(m_timestamp, 0, V12::BEGIN_RUN,
                                  run, m_elapsedTime, time(NULL),
                                  title);

  writeItem(ring, begin);
}

void 
TestSource::endRun(CDataSink &ring, int run, std::string title)
{
  V12::CRingStateChangeItem end(m_timestamp, 0, V12::END_RUN,
                                run, m_elapsedTime, time(NULL),
                                title);

  writeItem(ring, end);
}

void 
TestSource::Scaler(CDataSink &ring, int nscalers, int nsec)
{
  V12::CRingScalerItem item(nscalers);
  item.setStartTime(m_elapsedTime);
  m_elapsedTime += nsec;
  item.setEndTime(m_elapsedTime);

  for (int i = 0; i < nscalers; i++) {
    item.setScaler(i, i*10);
  }

  item.setEventTimestamp(m_timestamp);
  item.setSourceId(0);

  writeItem(ring, item);
}

void
TestSource::someEventData(CDataSink &ring, int events)
{
    Buffer::ByteBuffer body;
    for (uint16_t i = 0; i < 30; i++) {
      body << i;
    }

    for (int i = 0; i < events; i++) {
    uint64_t timestamp = m_timestamp;
    m_timestamp += m_tsIncrement;

    V12::CPhysicsEventItem event(m_timestamp, 0);
    
    // Put the timestamp first:

    event.getBody() << timestamp;
    event.getBody() << body;


    writeItem(ring, event);
    if (m_delay) {
      Os::usleep(m_delay);
    }

  }
}

} // end DAQ
