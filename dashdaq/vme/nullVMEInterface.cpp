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
#include "nullVMEInterface.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

// Implement test VME class.
// Constructor:

nullVMEInterface::nullVMEInterface() :
  m_fLocked(false)
{}

// Device type string.

string
nullVMEInterface::deviceType() const
{
  return string("NULL testing only");
}
// Device handle:

void*
nullVMEInterface::getDeviceHandle() const
{
  return NULL;
}
// Null address range:

CVMEAddressRange*
nullVMEInterface::createAddressRange(unsigned short addressModifier,
				     unsigned long  baseAddress,
				     size_t         bytes)
{
  return static_cast<CVMEAddressRange*>(NULL);
}
// Null PIO device.

CVMEPio*
nullVMEInterface::createPioDevice()
{
  return static_cast<CVMEPio*>(NULL);
}
// Null stored list.

CVMEList*
nullVMEInterface::createList()
{
  return static_cast<CVMEList*>(NULL);
}
// Null dma block transfer:

CVMEDMATransfer*
nullVMEInterface::createDMATransfer(unsigned short               addressModifier,
				    CVMEInterface::TransferWidth width,
				    unsigned long                base,
				    size_t                       units)
{
  return static_cast<CVMEDMATransfer*>(NULL);
}
// Check lock state:

bool
nullVMEInterface::locked() {
  return m_fLocked;
}
void
nullVMEInterface::onLock()
{
  m_fLocked = true;
}
void
nullVMEInterface::onUnlock()
{
  m_fLocked = false;
}
