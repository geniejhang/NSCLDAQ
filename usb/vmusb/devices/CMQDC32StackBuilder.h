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

#ifndef __CMQDC32StackBuilder_H
#define __CMQDC32StackBuilder_h

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


namespace MQDC32 {

class CMQDC32StackBuilder 
{
  private:
    uint32_t m_base;

  public:
    CMQDC32StackBuilder() = default;
    CMQDC32StackBuilder(const CMQDC32StackBuilder& rhs) = default;
    ~CMQDC32StackBuilder() = default;

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

    // Thresholds
    void addWriteThreshold(CVMUSBReadoutList& list, unsigned int chan, 
                           int thresh);
    void addWriteThresholds(CVMUSBReadoutList& list, 
                            std::vector<int> thrs);
    void addWriteIgnoreThresholds(CVMUSBReadoutList& list, bool ignore);


    void addWriteMarkerType(CVMUSBReadoutList& list, uint16_t type);

    void addWriteMemoryBankSeparation(CVMUSBReadoutList& list, uint16_t type);

    void addWriteGateLimit0(CVMUSBReadoutList& list, uint8_t val);
    void addWriteGateLimit1(CVMUSBReadoutList& list, uint8_t val);
    void addWriteGateLimits(CVMUSBReadoutList& list, std::vector<int> limits);

    void addWriteExpTrigDelay0(CVMUSBReadoutList& list, uint16_t val);
    void addWriteExpTrigDelay1(CVMUSBReadoutList& list, uint16_t val);
    void addWriteExpTrigDelays(CVMUSBReadoutList& list, std::vector<int> values);

    void addWriteBankOffsets(CVMUSBReadoutList& list, std::vector<int> values);

    void addWritePulserState(CVMUSBReadoutList& list, uint16_t state);
    void addWritePulserAmplitude(CVMUSBReadoutList& list, uint8_t amp);

    void addWriteTimeDivisor(CVMUSBReadoutList& list, uint16_t divisor);
    void addResetTimestamps(CVMUSBReadoutList& list);

    void addWriteInputCoupling(CVMUSBReadoutList& list, uint16_t type);

    void addWriteECLTermination(CVMUSBReadoutList& list, uint16_t type);

    void addWriteECLGate1Input(CVMUSBReadoutList& list, uint16_t type);
    void addWriteECLFCInput(CVMUSBReadoutList& list, uint16_t type);

    void addWriteNIMGate1Input(CVMUSBReadoutList& list, uint16_t type);
    void addWriteNIMFCInput(CVMUSBReadoutList& list, uint16_t type);
    void addWriteNIMBusyOutput(CVMUSBReadoutList& list, uint16_t type);

    void addWriteTimeBaseSource(CVMUSBReadoutList& list, uint16_t val);
    void addWriteMultiEventMode(CVMUSBReadoutList& list, uint16_t val);
    void addWriteTransferCount(CVMUSBReadoutList& list, uint16_t val);

    void addInitializeFifo(CVMUSBReadoutList& list);

    void addWriteLowerMultLimits(CVMUSBReadoutList& list, std::vector<int> values);
    void addWriteUpperMultLimits(CVMUSBReadoutList& list, std::vector<int> values);

    void addFifoRead(CVMUSBReadoutList& list, size_t transfers);
};

} // end of namespace

#endif
