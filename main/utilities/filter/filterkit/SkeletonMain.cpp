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


#include "CTemplateFilter.cpp"

#include <CFatalException.h>
#include <CFilterMain.h>
#include <V12/CFilterAbstraction.h>

#include <iostream>
#include <memory>

using namespace DAQ;


/*! the main function
 *
 * Creates a CFilterMain object, configures it for use with V12 data,
 * and then executes its operator()() method.
 *
 *  \retval 0 for normal exit,
 *  \retval 1 for known fatal error,
 *  \retval 2 for unknown fatal error
*/
int main(int argc, char* argv[])
{
  int status = 0;

  try {

    // Create the main
    CFilterMain theApp(argc,argv);

    // The filter main is able to handle nscldaq 11.0 and 12.0 data
    // format. In order to process a specific format, you need to pass
    // in a object that defines how to handle a specific version. Here
    // the V12::CFilterAbstraction class is used to deal with 12.0 data.
    // If you want to deal with version 11.0, then you need to use
    // V11::CFilterAbstraction.
    V12::CFilterAbstractionPtr pVersion(new V12::CFilterAbstraction);
    theApp.setVersionAbstraction(pVersion);

    // The filter that you create must be compatible with the version of
    // data that is being used. A CTemplateFilter is derived from
    // V12::CFilter and thus is for processing V12 data. If you want
    // to handle V11 data, you need to use a filter derived from the
    // V11::CFilter class.
    std::shared_ptr<CTemplateFilter> pFilter(new CTemplateFilter);

    // Register the filter(s) here. Note that if more than
    // one filter will be registered, the order of registration
    // will define the order of execution. If multiple filters are
    // registered, the output of the first filter will become the
    // input of the second filter and so on. Note that the filter
    // is registered to the version abstraction rather than the
    // CFilterMain object.
    pVersion->registerFilter(pFilter);

    // Run the main loop
    theApp();

  } catch (CFatalException exc) {
    status = 1;
  } catch (...) {
    std::cout << "Caught unknown fatal error...!" << std::endl;
    status = 2;
  }

  return status;
}

