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

#include <CWienerMDGG16.h>
#include <CVMUSBReadoutList.h>
#include <VMEAddressModifier.h>

static const uint32_t LOGICAL_OR_AB = 0x00b8;
static const uint32_t LOGICAL_OR_CD = 0x00bc;


void CWienerMDGG16::addWriteLogicalORMaskAB(CVMUSBReadoutList& list, uint16_t mask)
{
  list.addWrite32(m_base+LOGICAL_OR_AB, VMEAMod::a24UserData, mask);
}

void CWienerMDGG16::addWriteLogicalORMaskCD(CVMUSBReadoutList& list, uint16_t mask)
{
  list.addWrite32(m_base+LOGICAL_OR_CD, VMEAMod::a24UserData, mask);
}

