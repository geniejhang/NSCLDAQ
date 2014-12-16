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

#ifndef __CWienerMDGG16_H
#define __CWienerMDGG16_H

#include <stdint.h>

class CVMUSBReadoutList;

class CWienerMDGG16 {

  private:
    uint32_t m_base;

  public:
    void setBase(uint32_t base) { m_base = base; }
    uint32_t getBase() const { return m_base; }

    void addWriteLogicalORMaskAB(CVMUSBReadoutList& list, uint32_t mask);
    void addWriteLogicalORMaskCD(CVMUSBReadoutList& list, uint32_t mask);

    void addReadLogicalORMaskAB(CVMUSBReadoutList& list);
    void addReadLogicalORMaskCD(CVMUSBReadoutList& list);
                               
    void addWriteECLOutput(CVMUSBReadoutList& list, uint32_t val);
    void addReadECLOutput(CVMUSBReadoutList& list);
};

#endif
