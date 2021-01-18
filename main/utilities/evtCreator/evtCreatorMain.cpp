/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2021.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
     Giordano Cerizza
     NSCL/FRIB
     Michigan State University
     East Lansing, MI 48824-1321
*/

#include <string>
#include <CDataSinkFactory.h>
#include <CDataSink.h>
#include <CRingItem.h>            
#include <CRingStateChangeItem.h>
#include <iostream>
#include <time.h>
#include <random>
#include "evtCreatorMain.h"
#include "evtCreatorargs.h"

EvtCreatorMain::EvtCreatorMain() :
  m_tstart(0),
  m_tdiff(0),
  m_pSink(0),
  m_nParams(0),
  m_nEvents(0),
  m_url("")
{
}

EvtCreatorMain::~EvtCreatorMain()
{}

void
EvtCreatorMain::beginRun(CDataSink& sink)
{
  char title[100];
  sprintf(title, "BEGIN of the syntetic data run");
  m_tstart = time(NULL);
  CRingStateChangeItem item(BEGIN_RUN, 0, 0, time(NULL), title);
  sink.putItem(item);
}

void
EvtCreatorMain::endRun(CDataSink& sink)
{
  char title[100];
  sprintf(title, "END of the syntetic data run");
  m_tdiff = time(NULL)-m_tstart;
  CRingStateChangeItem item(END_RUN, 0, m_tdiff, time(NULL), title);
  sink.putItem(item);
}

void
EvtCreatorMain::event(CDataSink& sink)
{
  std::random_device rd;
  std::default_random_engine generator;

  CRingItem item(PHYSICS_EVENT);
  uint16_t* pBody = (uint16_t*)(item.getBodyCursor());
  *pBody++ = m_nParams*sizeof(uint16_t) + sizeof(uint32_t)/sizeof(uint16_t);
  for (int i=0; i < m_nParams; i++) {
    double mean = 50+i*100;
    double sigma = 25+i*10;
    std::normal_distribution<double> distribution(mean, sigma);
    generator.seed( rd() );
    int number = (int)distribution(generator);
    *pBody++ = number;
  }
  item.setBodyCursor(pBody);
  item.updateSize();

  sink.putItem(item);
}

int
EvtCreatorMain::operator()(int argc, char**argv)
{
  parseArguments(argc, argv);
  runCreator();
  return 1;
}

void
EvtCreatorMain::runCreator()
{
  try {
    CDataSinkFactory factory;
    m_pSink = factory.makeSink(m_url);

  }
  catch (CException& e) {
    std::cerr << "Please run evtCreator --help for options" << std::endl;
  }

  // Begin run
  beginRun(*m_pSink);
  // Event generation
  for (int i =0; i < m_nEvents; i++)
    event(*m_pSink);
  // End run
  endRun(*m_pSink);        

}

/*
** Parse the command line arguments, stuff them where they need to be
** and check them for validity:
** - Number of parameters has to be defined
** - Number of events has to be defined
** - Ring/file has to be defined
**
** Parameters:
**   argc  - Count of command words.
**   argv  - Array of pointers to the command words.
*/
void
EvtCreatorMain::parseArguments(int argc, char** argv)
{
  gengetopt_args_info parsed;
  cmdline_parser(argc, argv, &parsed);

  if (parsed.n_params_arg) 
    m_nParams = parsed.n_params_arg;
  
  if (parsed.n_events_arg)
    m_nEvents = parsed.n_events_arg;  
  
  if (parsed.sink_arg)
    m_url = parsed.sink_arg;
}
