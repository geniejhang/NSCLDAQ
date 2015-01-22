#ifndef __CV977TRIGGERSTATUS_H
#define __CV977TRIGGERSTATUS_H

/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

class CExperiment;

/*!
   This class manages a trigger and a status object for the production readout
   framework.  Constructing this class results in a new trigger and status
   being registered that use the V977 as follows:
   - input 0 when gated is the trigger for the readout.
   - output0 is the busy signal for the readout
   - output1 is the module clears.


*/
class CV977TriggerStatus
{
  // Constructors and other canonicals.

public:
  static void Register(CExperiment& rExperiment, 
		       uint32_t      baseAddress,
		       unsigned int  crate = 0);      


};


#endif
