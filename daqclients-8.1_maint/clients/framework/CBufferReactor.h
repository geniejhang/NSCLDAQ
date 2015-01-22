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


//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//
// Copyright (c) NSCL All rights reserved 2001

#ifndef __CBUFFERREACTOR_H  //Required for current class
#define __CBUFFERREACTOR_H

//
// Include files:
//

                               //Required for base classes
#ifndef __CREACTOR_H     //CReactor
#include "CReactor.h"
#endif
#ifndef __SPECTRODAQ_HEADER
#include <spectrodaq.h>
#define __SPECTRODAQ_HEADER
#endif

#ifndef __CBUFFERMONITOR_H
#include <CBufferMonitor.h>
#endif

#ifndef __STL_STRING
#include <string>
#define __STL_STRING
#endif

#ifndef __CEVENTMONITOR_H
#include <CEventMonitor.h>
#endif
// Forward class definition.



 
/*!
Base class for SpectroDaq buffer receipt.
This object must be subclassed to provide
application specific processing.
*/

class CBufferReactor  : public CReactor        
{
  // Constructors and related functions.

public:
  CBufferReactor ();
  CBufferReactor(const string& rName);
  CBufferReactor(const char*   pName);
 ~ CBufferReactor ( );  
 
  
  //! Copy Constructor disallowed.
private:
  CBufferReactor (const CBufferReactor& aCBufferReactor );
public:
  //! Operator= Assignment Operator disallowed.
private:
  CBufferReactor& operator= (const CBufferReactor& aCBufferReactor);
public:
  int operator== (const CBufferReactor& aCBufferReactor) const;
 
  // operations on the class:
public:
  virtual void OnEvent(CEventMonitor& rMonitor);
  virtual void OnBuffer(CBufferMonitor& rMonitor, 
			DAQWordBufferPtr pBuffer);

};

// For most compilers, the implementation file must be accessible from the
// header to instantiate particular classes.
//

typedef CBufferReactor  CWordBufferReactor;


#endif
