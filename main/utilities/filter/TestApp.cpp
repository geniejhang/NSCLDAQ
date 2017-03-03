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
#include <V11/CTransparentFilter.h>
#include <V11/CFilterAbstraction.h>

using namespace DAQ;

int main(int argc, char** argv)
{

  CFilterMain theApp(argc, argv);

  V11::CFilterAbstractionPtr pVersion(new V11::CFilterAbstraction);

  // Create a transparent filter
  V11::CFilterPtr pFilter(new V11::CTransparentFilter);

  pVersion->registerFilter(pFilter);

  theApp.setVersionAbstraction(pVersion);

  theApp();

  return 0;

}
