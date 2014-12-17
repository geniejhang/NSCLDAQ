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
#include <WienerMDGG16Registers.h>

using namespace WienerMDGG16;


void CWienerMDGG16::addWriteLogicalORMaskAB(CVMUSBReadoutList& list, 
                                            uint32_t mask)
{
  list.addWrite32(m_base+Regs::Logical_OR_AB, VMEAMod::a24UserData, mask);
}

void CWienerMDGG16::addWriteLogicalORMaskCD(CVMUSBReadoutList& list, 
                                            uint32_t mask)
{
  list.addWrite32(m_base+Regs::Logical_OR_CD, VMEAMod::a24UserData, mask);
}

void CWienerMDGG16::addReadLogicalORMaskAB(CVMUSBReadoutList& list)
{
  list.addRead32(m_base+Regs::Logical_OR_AB, VMEAMod::a24UserData);
}

void CWienerMDGG16::addReadLogicalORMaskCD(CVMUSBReadoutList& list)
{
  list.addRead32(m_base+Regs::Logical_OR_CD, VMEAMod::a24UserData);
}

void CWienerMDGG16::addWriteECLOutput(CVMUSBReadoutList& list, 
                                      uint32_t value)
{
  list.addWrite32(m_base+Regs::ECL_Output, VMEAMod::a24UserData, value);
}

void CWienerMDGG16::addReadECLOutput(CVMUSBReadoutList& list)
{
  list.addRead32(m_base+Regs::ECL_Output, VMEAMod::a24UserData);
}

void CWienerMDGG16::addReadFirmware(CVMUSBReadoutList& list)
{
  list.addRead32(m_base+Regs::FirmwareID, VMEAMod::a24UserData);
}

void CWienerMDGG16::addReadGlobal(CVMUSBReadoutList& list)
{
  list.addRead32(m_base+Regs::Global, VMEAMod::a24UserData);
}

