/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
      NSCLDAQ Development Group 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


#ifndef CDATASINK_H
#define CDATASINK_H

#include <CRingItem.h>
#include <DataSink.h>

using CDataSink = NSCLDAQ::DataSink<CRingItem>;
#endif
