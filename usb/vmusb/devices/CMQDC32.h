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

#ifndef __CMQDC32_H
#define __CMQDC32_h

#ifndef __CREADOUTHARDWARE_H
#include "CReadoutHardware.h"
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif


// Forward class definitions:

class CVMUSB;
class CVMUSBReadoutList;


struct CVMUSBResult {
  size_t nBytes;
  std::vector<uint8_t> data;
};

class CMQDC32 
{
  private:
    uint32_t m_base;

  public:
    CMQDC32() = default;
    CMQDC32(const CMQDC32& rhs) = default;
    ~CMQDC32() = default;

    void setBase(uint32_t base) { m_base = base; }
    uint32_t getBase(uint32_t base) const { return m_base; }

  public:
    // Interactive methods
    void resetAll(CVMUSB& ctlr);
    void doSoftReset(CVMUSB& ctlr);
    void addSoftReset(CVMUSBReadoutList& list);
    void addWriteAcquisitionState(CVMUSBReadoutList& list, bool on);
    void addResetReadout(CVMUSBReadoutList& list);

    void addDisableInterrupts(CVMUSBReadoutList& list);
    void addWriteIrqLevel(CVMUSBReadoutList& list, uint8_t level);
    void addWriteIrqVector(CVMUSBReadoutList& list, uint8_t level);
    void addWriteIrqThreshold(CVMUSBReadoutList& list, uint16_t thresh);
    void addWriteWithdrawIrqOnEmpty(CVMUSBReadoutList& list, bool on);

    // Stack building methods
    void addWriteModuleID(CVMUSBReadoutList& list, uint16_t id);

    void addWriteThreshold(CVMUSBReadoutList& list, unsigned int chan, 
                           int thresh);
    void addWriteThresholds(CVMUSBReadoutList& list, 
                            std::vector<int> thrs);
    void addWriteIgnoreThresholds(CVMUSBReadoutList& list, bool ignore);

    void addWriteMarkerType(CVMUSBReadoutList& list, uint16_t type);

    void addWriteMemoryBankSeparation(CVMUSBReadoutList& list, uint16_t type);

    void addWriteBankLimit0(CVMUSBReadoutList& list, uint8_t val);
    void addWriteBankLimit1(CVMUSBReadoutList& list, uint8_t val);
    void addWriteBankLimits(CVMUSBReadoutList& list, uint8_t limit0, 
                            uint8_t limit1);

    void addWriteExpTrigDelay0(CVMUSBReadoutList& list, uint16_t val);
    void addWriteExpTrigDelay1(CVMUSBReadoutList& list, uint16_t val);
    void addWriteExpTrigDelays(CVMUSBReadoutList& list, uint16_t val0, 
                               uint16_t val1);

    void addWritePulserState(CVMUSBReadoutList& list, uint16_t state);

    void addWriteTimeDivisor(CVMUSBReadoutList& list, uint16_t divisor);
    void addResetTimestamps(CVMUSBReadoutList& list);

    void addWriteInputCoupling(CVMUSBReadoutList& list, uint16_t type);

    void addWriteECLTermination(CVMUSBReadoutList& list, uint16_t type);

    void addWriteECLGate1Input(CVMUSBReadoutList& list, uint16_t type);
    void addWriteECLFCInput(CVMUSBReadoutList& list, uint16_t type);

    void addWriteNIMGate1Input(CVMUSBReadoutList& list, uint16_t type);
    void addWriteNIMFCInput(CVMUSBReadoutList& list, uint16_t type);
    void addWriteNIMBusyInput(CVMUSBReadoutList& list, uint16_t type);

    void addWriteTimeBaseSource(CVMUSBReadoutList& list, uint16_t val);
    void addWriteMultiEventMode(CVMUSBReadoutList& list, uint16_t val);

    void addInitializeFifo(CVMUSBReadoutList& list);


  private:
    CVMUSBResult executeList(CVMUSB& ctlr, CVMUSBReadoutList& list, const size_t maxBytes);
};



#endif
