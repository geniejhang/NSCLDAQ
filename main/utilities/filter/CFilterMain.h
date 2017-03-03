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




#ifndef DAQ_CFILTERMAIN_H
#define DAQ_CFILTERMAIN_H

#include <CFatalException.h>
#include <CFilterVersionAbstraction.h>
#include <CFilterMediator.h>

#include <memory>
#include <vector>
#include <cstdint>

struct gengetopt_args_info;

namespace DAQ {

class CDataSource;
class CDataSink;

class CFilterMain
{
  
  private:
    CFilterMediatorPtr m_pMediator; //!< The mediator
    struct gengetopt_args_info* m_argsInfo; //!< The parsed options

  public:
    /**! Constructor
      Constructs a mediator object with a CCaesarFilter
      as the default filter. 
    
      \param argc the number of command line args
      \param argv the command line args

      \throw can throw a CFatalException

     */
    CFilterMain(int argc, char** argv);

    /**! Destructor
      This does absolutely nothing.
    */
    ~CFilterMain();

    /**! Main loop 
      Executes the CMediator::mainLoop() 
    */
    void operator()();

    void setVersionAbstraction(CFilterVersionAbstractionPtr pAbstraction);

    /**! Retrieve the mediator
     *
     * Ownership of the mediator remains with the CFilterMain instance.
     *
     * \returns ptr to the mediator
     */
    CFilterMediatorPtr getMediator() { return m_pMediator; }

    void setMediator(CFilterMediatorPtr pMediator) { m_pMediator = pMediator; }


  private:
    // Private utility functions 
    std::unique_ptr<DAQ::CDataSource> constructDataSource();
    std::unique_ptr<DAQ::CDataSink> constructDataSink();

    std::vector<std::uint16_t> constructExcludesList();
    std::vector<std::uint16_t> constructSampleList();
    
};

} // end DAQ

#endif // DAQ_CFILTERMAIN_H
