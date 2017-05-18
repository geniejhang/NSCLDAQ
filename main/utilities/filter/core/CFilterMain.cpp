/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


static const char* Copyright = "(C) Copyright Michigan State University 2014, All rights reserved";

#include "CFilterMain.h"
#include "CFilterMediator.h"
#include "CDataSourceFactory.h"
#include "CDataSinkFactory.h"
#include <CPredicate.h>
#include <CCompositePredicate.h>
#include <CProcessCountPredicate.h>
#include "CFatalException.h"
#include "filterargs.h"
#include <Exception.h>

#include <string>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <stdexcept>

using namespace DAQ;

/**! Constructor
  Constructs a mediator object with a CCompositeFilter
  as the default filter. We also set up the proper 
  skip and processing counts that user supplied.

  \throw can throw a CFatalException

*/
CFilterMain::CFilterMain(int argc, char** argv)
  : m_pMediator(),
  m_argsInfo(new gengetopt_args_info)
{
  cmdline_parser(argc,argv,m_argsInfo);  

  try {

      m_pMediator.reset(new CFilterMediator);

      auto pPred = std::make_shared<CCompositePredicate>();

    // Set up the data source 
    auto pSource = constructDataSource();
    m_pMediator->setDataSource(std::move(pSource));

    // Set up the sink source 
    auto pSink = constructDataSink();
    m_pMediator->setDataSink(std::move(pSink));


    auto pProcessPred = std::make_shared<CProcessCountPredicate>();
    // set up the skip and count args
    if (m_argsInfo->skip_given) {
        pProcessPred->setNumberToSkip(m_argsInfo->skip_arg);
    }
    if (m_argsInfo->count_given) {
        pProcessPred->setNumberToProcess(m_argsInfo->count_arg);
    }

    pPred->addPredicate(pProcessPred);
    m_pMediator->setPredicate(pPred);

  } catch (CException& exc) {
    std::cerr << exc.ReasonText() << std::endl;
    std::cerr << exc.WasDoing() << std::endl;
    throw CFatalException();
  } 
  catch (std::string msg) {
    std::cerr << "String exception caught: " << msg << std::endl;
    throw CFatalException();
  }
  catch (const char* msg) {
    std::cerr << "const char* exception caught: " << msg << std::endl;
    throw CFatalException();
  }
  catch (std::exception& e) {
    std::cerr << "Std::exception caught: " << e.what() << std::endl;
    throw CFatalException();
  }
  catch (...) {
    std::cout << "Unanticipated exception type\n";
    throw CFatalException();
  }
}


CFilterMain::~CFilterMain()
{
  delete m_argsInfo;
}


void CFilterMain::setVersionAbstraction(CFilterVersionAbstractionPtr pAbstraction)
{
    m_pMediator->setVersionAbstraction(pAbstraction);

    if (m_argsInfo->oneshot_given) {
        pAbstraction->setOneShotMode(m_argsInfo->number_of_sources_arg);
    }
}


void CFilterMain::printUsageString() const
{
    cmdline_parser_print_help();
}

/////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////// Private utilities

/**! Construct a data source
    A data source must provide a sample and an excludes list along
    with a uri. The default value for this is stdin. 
  
    \exception May propagate CErrnoException,s CURLFormatException,
        or CFatalException
*/
std::unique_ptr<CDataSource> CFilterMain::constructDataSource()
{
  // Set up default source type
  std::string source_name("-");
  if (m_argsInfo->source_given) {
    source_name = std::string(m_argsInfo->source_arg);
  } 

  // Set up the sampling
  if (m_argsInfo->sample_given) {
      m_pMediator->setSampleList(m_argsInfo->sample_arg);
  }
  
  // Set up the excludes
  if (m_argsInfo->exclude_given) {
      m_pMediator->setExcludeList(m_argsInfo->exclude_arg);
  }
  
  std::unique_ptr<CDataSource> pSource(CDataSourceFactory().makeSource(source_name));
  return pSource;
}

/**! Set up the data sink
  
    Based on user's argument --sink, generates the appropriate source
    type.
*/
std::unique_ptr<CDataSink> CFilterMain::constructDataSink()
{
  // Set up default source type
  std::string sink_name("-");
  if (m_argsInfo->sink_given) {
    sink_name = std::string(m_argsInfo->sink_arg);
  } 

  std::unique_ptr<CDataSink> sink(CDataSinkFactory().makeSink(sink_name));
  return sink;
}

/**! The main loop
    This is just a wrapper around the mediator's mainLoop. It is here
    that the processing occurs in the application. */
void CFilterMain::operator()()
{
  try {
    m_pMediator->initialize();

    // allow the finalize operations to be called if an exception is 
    // thrown from the main loop. Consider the arrival of an 
    // ABNORMAL_ENDRUN
    try {

        m_pMediator->mainLoop();

    } catch (CException& exc) {
      std::cerr << exc.WasDoing() << " : " << exc.ReasonText() << std::endl;
    } catch (std::exception& exc) {
      std::cerr << "Caught std::exception thrown from main loop. " << exc.what() << std::endl;
    } catch (...) {
      std::cerr << "Caught unknown exception thrown from main loop. ";
    }

    std::cerr << "Shutting down filter." << std::endl;

    m_pMediator->finalize();

  } catch (CException& exc) {
    std::cerr << exc.WasDoing() << " : " << exc.ReasonText() << std::endl;
    throw CFatalException(); 
  }

}
