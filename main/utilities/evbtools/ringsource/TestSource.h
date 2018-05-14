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

#ifndef TESTSOURCE_H
#define TESTSOURCE_H


/**
 * @file TestSource.h
 * @brief Definition for the TestSource class which produces data into the ringk.
 */
#include <string>
#include <cstdint>
#include <unistd.h>

namespace DAQ {

class CDataSink;

/**
 * @class TestSource
 *
 *  The TestSource class, when instantiated provides a wide variety of test data and
 *  edge conditions for the ring data source.
 *
 */
class TestSource 
{
  std::string m_ringName;
  int         m_size;
  int         m_elapsedTime;
  int         m_tsIncrement;
  uint64_t    m_timestamp;
  useconds_t  m_delay;

public:
  TestSource(std::string ringName, int eventSize) :
    m_ringName(ringName), m_size(eventSize), m_elapsedTime(0), 
    m_tsIncrement(2), m_timestamp(0), m_delay(0)
  {}

public:
  void setTimestampIncrement(int newIncr) {m_tsIncrement = newIncr;}
  void setDelay(uint64_t newDelay) {m_delay = newDelay; }

public:
  void operator()();
    
  // private utilities:

  void dataFormat(CDataSink& ring);
  void beginRun(CDataSink& ring, int run, std::string title);
  void someEventData(CDataSink& rings, int events);
  void endRun(CDataSink& ring, int run, std::string title);
  void Scaler(CDataSink& ring, int nsclers, int nsec);
  
};

} // end DAQ

#endif

