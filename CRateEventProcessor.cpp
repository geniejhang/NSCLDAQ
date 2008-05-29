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

#include <config.h>
#include "CRateEventProcessor.h"
#include <Event.h>

/*  These are all trivial methods:
 */

CRateEventProcessor::CRateEventProcessor() {}
CRateEventProcessor::~CRateEventProcessor() {}


Bool_t
CRateEventProcessor:: operator()(const Address_t pEvent,
				 CEvent&         rEvent,
				 CAnalyzer&      rAnalyzer,
				 CBufferDecoder& rDecoder)
{
  rEvent[0] = 1;
  return kfTRUE;
}
