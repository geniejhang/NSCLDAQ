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


#include "CInfiniteMediator.h"

#include <iostream>
#include <CDataSource.h>
#include <CDataSink.h>
#include <CBufferDecoder.h>
#include <CBuffer.h>
#include <CBufferIO.h>

#include <CRingStateChangeItem.h>
#include <CRingScalerItem.h>
#include <CRingTextItem.h>
#include <CPhysicsEventItem.h>
#include <CRingPhysicsEventCountItem.h>
#include <CRingFragmentItem.h>

/**! Constructor

  Constructs the mediator object. This object owns its referenced members.

  \param source a pointer to a CDataSource
  \param filter a pointer to a CFilter
  \param sink a pointer to a CDataSink

*/
CInfiniteMediator::CInfiniteMediator(CDataSource* source, CBufferDecoder* decoder, CDataSink* sink)
: CMediator(source,decoder,sink)
{}

/**! Destructor
  Delete the owned objects. It is possible that these were never initialized,
  so it is important to verify that they point to something other than NULL.

  Following a delete the pointers are reset to 0. This protects against double
  frees when a user decides to call an objects destructor explicitly and then
  it goes out of scope.
*/
CInfiniteMediator::~CInfiniteMediator() 
{
}

/**! The main loop
  This is the workhorse of the application. Items are retrieved 
  from the source, passed to the filter, and then the item returned 
  by the filter is sent to the
  sink. During each iteration through this process, the item retrieved
  from the source and the filter are properly managed.
*/
void CInfiniteMediator::mainLoop()
{
  
  // Dereference our pointers before entering
  // the main loop
  CDataSource& source = *getDataSource();
  CDataSink& sink = *getDataSink();
  CBufferDecoder& decoder = *getBufferDecoder();

  while  (1) {
    // create a new buffer
    CBuffer buffer;

    // fill it
    source >> buffer;

    // send it to the decoder to parse and process 
    CBuffer newBuffer = decoder.onBuffer(buffer);

    // output the data
    sink << buffer;
  }

}

void CInfiniteMediator::initialize() 
{
}

void CInfiniteMediator::finalize() 
{
}
