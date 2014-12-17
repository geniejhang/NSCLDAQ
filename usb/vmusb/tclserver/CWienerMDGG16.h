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

#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>

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

    void addReadFirmware(CVMUSBReadoutList& list);
    void addReadGlobal(CVMUSBReadoutList& list);

    uint32_t readFirmware(CVMUSB& ctlr);
    uint32_t readGlobal(CVMUSB& ctlr);

  private:
    template <class T> T executeList(CVMUSB& ctlr, CVMUSBReadoutList& list);

};



template <class T>
T CWienerMDGG16::executeList(CVMUSB& ctlr, CVMUSBReadoutList& list)
{
  size_t nRead=0;
  T buffer;
  int status = ctlr.executeList(list, &buffer, sizeof(buffer), &nRead);
  if (status<0) {
    std::string errmsg ("CWienerMDGG16::readGlobal() failed during ");
    errmsg += "executeList() with status " + std::to_string(status);
    throw errmsg;
  }

  if (nRead != sizeof(buffer)) {
    std::string errmsg ("CWienerMDGG16::executeList() read back fewer");
    errmsg += " bytes than were expected.";
    throw errmsg;
  }

  return buffer;
}

#endif
