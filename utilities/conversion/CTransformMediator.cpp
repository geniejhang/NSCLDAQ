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


#include "CTransformMediator.h"

#include <iostream>
#include <CDataSource.h>
#include <CDataSink.h>
#include <CDataHandler.h>
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
template<class Transform>
CTransformMediator::CTransformMediator(CDataSource* source, 
                                       CDataSink* sink, 
                                       Transform transform)
: CMediator(source,sink),
  m_transform(transform)
{}

/**! Destructor
  Delete the owned objects. It is possible that these were never initialized,
  so it is important to verify that they point to something other than NULL.

  Following a delete the pointers are reset to 0. This protects against double
  frees when a user decides to call an objects destructor explicitly and then
  it goes out of scope.
*/
template<class Transform>
CTransformMediator::~CTransformMediator() 
{
}

/**! The main loop
  This is the workhorse of the application. Items are retrieved 
  from the source, passed to the filter, and then the item returned 
  by the filter is sent to the
  sink. During each iteration through this process, the item retrieved
  from the source and the filter are properly managed.
*/
template<class Transform>
void CTransformMediator::mainLoop()
{
  
  // Dereference our pointers before entering
  // the main loop

  while (1) {
    processOne();
  }

}

void CTransformMediator::initialize() 
{
}

void CTransformMediator::finalize() 
{
}

void CTransformMediator::processOne()
{
  CDataSource& source    = *getDataSource();
  CDataSink&   sink      = *getDataSink();
  CPredicate&  predicate = *getPredicate();

  using T1 = typename Transform::InitialType;
  using T2 = typename Transform::FinalType;

  T1 item1;
  source >> item1;

  updatePredicate();

  if (predicate()) {

    T2 item2 = m_transform(item1);

    sink << item2;
  }
}
