/*!
           Base class for SpectroDaq buffer receipt.
           This object must be subclassed to provide
           application specific processing.
*/
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//
// Copyright (c) NSCL All rights reserved 2001

////////////////////////// FILE_NAME.cpp /////////////////////////////////////////////////////


#include <config.h>
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

#include "CBufferReactor.h"    				
#include <spectrodaq.h>
#include <CEventMonitor.h>
#include <CBufferMonitor.h>
#include <CIncompatibleMonitor.h>
#include <string.h>
#include <typeinfo>


// Constructors.

/*!
   Construct a buffer reactor with a default name.
   Note that buffer reactors are templated by the type of data
   contained in the buffer.
 */
CBufferReactor :: CBufferReactor () :
  CReactor()
{
  AppendClassInfo();
} 
/*!
   Construct a named buffer reactor using an STL string parameter
   for the object name.
   \param rName - the name of the reactor.
 */

CBufferReactor::CBufferReactor(const string& rName) :
  CReactor(rName)
{
  AppendClassInfo();
}
/*!
  Construct a named buffer reactor using a ASCIZ (C) string parameter
  for the object name.
  \param pName - Pointer to an ASCIZ string naming the buffer.
 
  */
CBufferReactor::CBufferReactor(const char* pName) :
  CReactor(pName)
{
  AppendClassInfo();
}
 CBufferReactor::~CBufferReactor ( )  //Destructor - Delete dynamic objects
{
}


      //! Operator== Equality comparison:
int 
CBufferReactor::operator== (const CBufferReactor& aCBufferReactor) const
{ 
  return ( (CReactor::operator== (aCBufferReactor)));
}

// Functions for class CBufferReactor

/*!
   Called when the event monitor declares an event.
   The event monitor must be descended from CBufferMonitor or 
   this function will throw a CIncompatibleMonitor exception.
    The OnBuffer virtual member is called with a pointer to the buffer.
   \param rMonitor - Reference to the monitor which declared the event.
 */
void 
CBufferReactor::OnEvent(CEventMonitor& rMonitor)
{
  CBufferMonitor* pMonitor;
  try {
    pMonitor = dynamic_cast<CBufferMonitor*>(&rMonitor);
  }
  catch (bad_cast& rexcept) {
    throw
      CIncompatibleMonitor(rMonitor,
			   "CBufferReactor::OnEvent failed Monitor cast");
  }
  OnBuffer(*pMonitor, pMonitor->getBufferPointer());
  // pMonitor->getBuffer().Release();
}
/*!
 Called when a buffer has been received by a buffer monitor.
 In normal use, the user will subclass CBufferReactor and override
 this no-op member.
 \param rMonitor - The buffer monitor which received the buffer.
 \param pBuffer  - A DAQBufferPtr of the appropriate type into the
                   buffer received.
  */
void 
CBufferReactor::OnBuffer(CBufferMonitor& rMonitor, 
			 DAQWordBufferPtr pBuffer)
{
}

