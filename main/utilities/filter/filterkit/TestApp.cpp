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


#include <CFilterMain.h>
#include <V12/CTransparentFilter.h>
#include <V12/CFilterAbstraction.h>

using namespace DAQ;

int main(int argc, char** argv)
{

  CFilterMain theApp(argc, argv);

  V12::CFilterAbstractionPtr pVersion(new V12::CFilterAbstraction);

  // Create a transparent filter
  V12::CFilterPtr pFilter(new V12::CTransparentFilter);

  pVersion->registerFilter(pFilter);

  theApp.setVersionAbstraction(pVersion);

  theApp();

  return 0;

}
