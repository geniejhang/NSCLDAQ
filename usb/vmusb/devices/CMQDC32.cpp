
#include <CMQDC32.h>
#include <MQDC32Registers.h>
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#include <unistd.h>
#include <string>
#include <memory>

using namespace std;

void CMQDC32::resetAll(CVMUSB& ctlr) {

  doSoftReset(ctlr);

  unique_ptr<CVMUSBReadoutList> pList(ctlr.createReadoutList());
  addWriteAcquisitionState(*pList,0);
  addResetReadout(*pList);
  executeList(ctlr, *pList, 8);
}

void CMQDC32::doSoftReset(CVMUSB& ctlr) {
  unique_ptr<CVMUSBReadoutList> pList(ctlr.createReadoutList());
  addSoftReset(*pList);
  executeList(ctlr, *pList, 8);
  sleep(1);
}

void CMQDC32::addSoftReset(CVMUSBReadoutList& list) {
  list.addWrite16(m_base + Reset, initamod, 1);
}

void CMQDC32::addWriteAcquisitionState(CVMUSBReadoutList& list, bool state) 
{
  list.addWrite16(m_base + StartAcq, initamod, static_cast<uint16_t>(state));
}

void CMQDC32::addResetReadout(CVMUSBReadoutList& list) {
  list.addWrite16(m_base + ReadoutReset, initamod, 1);
}

void CMQDC32::addDisableInterrupts(CVMUSBReadoutList& list) 
{
  list.addWrite16(m_base + Ipl, initamod, 0);
  list.addDelay(MQDCDELAY);
}

void CMQDC32::addWriteIrqLevel(CVMUSBReadoutList& list, uint8_t level) 
{
  list.addWrite16(m_base + Ipl, initamod, level);
  list.addDelay(MQDCDELAY);
}

void CMQDC32::addWriteIrqVector(CVMUSBReadoutList& list, uint8_t level) 
{
  list.addWrite16(m_base + Vector, initamod, level);
  list.addDelay(MQDCDELAY);
}

void CMQDC32::addWriteIrqThreshold(CVMUSBReadoutList& list, uint16_t thresh) 
{
  list.addWrite16(m_base + IrqThreshold, initamod, thresh);
  list.addDelay(MQDCDELAY);
}


void CMQDC32::addWriteWithdrawIrqOnEmpty(CVMUSBReadoutList& list, bool on) 
{
  list.addWrite16(m_base + WithdrawIrqOnEmpty, initamod, (uint16_t)on);
  list.addDelay(MQDCDELAY);
}

void CMQDC32::addWriteModuleID(CVMUSBReadoutList& list, uint16_t id)
{
  list.addWrite16(m_base + ModuleId, initamod, id); // Module id.
  list.addDelay(MQDCDELAY);
}

void CMQDC32::addWriteThreshold(CVMUSBReadoutList& list, unsigned int chan, 
                                int thresh)
{
  uint32_t addr = m_base + Thresholds + chan*sizeof(uint16_t);
  list.addWrite16(addr, initamod, thresh);
  list.addDelay(MQDCDELAY);
}

void CMQDC32::addWriteThresholds(CVMUSBReadoutList& list,
                                 vector<int> thrs)
{
  for (size_t chan=0; chan<32; ++chan) {
    addWriteThreshold(list, chan, thrs.at(chan));
  }
}

void CMQDC32::addWriteIgnoreThresholds(CVMUSBReadoutList& list, bool off)
{
  list.addWrite16(m_base+IgnoreThresholds, initamod, uint16_t(off));
  list.addDelay(MQDCDELAY);
}
                                    

void CMQDC32::addWriteMarkerType(CVMUSBReadoutList& list, uint16_t type)
{
  list.addWrite16(m_base + MarkType, initamod, type); 
  list.addDelay(MQDCDELAY);
}

void CMQDC32::addWriteMemoryBankSeparation(CVMUSBReadoutList& list, 
                                           uint16_t type)
{
  list.addWrite16(m_base + BankOperation, initamod, type);
  list.addDelay(MQDCDELAY);
}

void CMQDC32::addWriteBankLimit0(CVMUSBReadoutList& list, uint8_t limit)
{
  list.addWrite16(m_base + BankLimit0, initamod, limit);
  list.addDelay(MQDCDELAY);
}

void CMQDC32::addWriteBankLimit1(CVMUSBReadoutList& list, uint8_t limit)
{
  list.addWrite16(m_base + BankLimit1, initamod, limit);
  list.addDelay(MQDCDELAY);
}

void CMQDC32::addWriteBankLimits(CVMUSBReadoutList& list, uint8_t limit0, uint8_t limit1)
{
  addWriteBankLimit0(list,limit0);
  addWriteBankLimit1(list,limit1);
}

void CMQDC32::addWriteExpTrigDelay0(CVMUSBReadoutList& list, uint16_t limit)
{
  list.addWrite16(m_base + ExpTrigDelay0, initamod, limit);
  list.addDelay(MQDCDELAY);
}

void CMQDC32::addWriteExpTrigDelay1(CVMUSBReadoutList& list, uint16_t limit)
{
  list.addWrite16(m_base + ExpTrigDelay1, initamod, limit);
  list.addDelay(MQDCDELAY);
}

void CMQDC32::addWriteExpTrigDelays(CVMUSBReadoutList& list, uint16_t limit0, uint16_t limit1)
{
  addWriteExpTrigDelay0(list,limit0);
  addWriteExpTrigDelay1(list,limit1);
}

void CMQDC32::addWritePulserState(CVMUSBReadoutList& list, uint16_t state)
{
  list.addWrite16(m_base+TestPulser, initamod, state);
  list.addDelay(MQDCDELAY);
}

void CMQDC32::addWriteInputCoupling(CVMUSBReadoutList& list, uint16_t type)
{
  list.addWrite16(m_base+InputCoupling, initamod, type);
  list.addDelay(MQDCDELAY);
}

void CMQDC32::addWriteTimeDivisor(CVMUSBReadoutList& list, uint16_t divisor)
{
  list.addWrite16(m_base + TimingDivisor, initamod, divisor);
  list.addDelay(MQDCDELAY);
}

void CMQDC32::addResetTimestamps(CVMUSBReadoutList& list) 
{
  list.addWrite16(m_base + TimestampReset, initamod, uint16_t(3)); // Reset both counters.
  list.addDelay(MQDCDELAY);
}

void CMQDC32::addWriteECLTermination(CVMUSBReadoutList& list, uint16_t type){
  list.addWrite16(m_base + ECLTermination, initamod, type);
  list.addDelay(MQDCDELAY);
}

void CMQDC32::addWriteECLGate1Input(CVMUSBReadoutList& list, uint16_t type)
{
  list.addWrite16(m_base + ECLGate1, initamod, type);
  list.addDelay(MQDCDELAY);
}

void CMQDC32::addWriteECLFCInput(CVMUSBReadoutList& list, uint16_t type)
{
  list.addWrite16(m_base + ECLFC, initamod, type);
  list.addDelay(MQDCDELAY);
}

void CMQDC32::addWriteNIMGate1Input(CVMUSBReadoutList& list, uint16_t type){
  list.addWrite16(m_base + NIMGate1, initamod, type);
  list.addDelay(MQDCDELAY);
}

void CMQDC32::addWriteNIMFCInput(CVMUSBReadoutList& list, uint16_t type)
{
  list.addWrite16(m_base + NIMFC, initamod, type);
  list.addDelay(MQDCDELAY);
}

void CMQDC32::addWriteNIMBusyInput(CVMUSBReadoutList& list, uint16_t type) {
  list.addWrite16(m_base + NIMBusy, initamod, type);
  list.addDelay(MQDCDELAY);
}

void CMQDC32::addWriteTimeBaseSource(CVMUSBReadoutList& list, uint16_t val){
  list.addWrite16(m_base + TimingSource, initamod, val);
  list.addDelay(MQDCDELAY);
}

void CMQDC32::addWriteMultiEventMode(CVMUSBReadoutList& list, uint16_t val){
  list.addWrite16(m_base + MultiEvent, initamod, val);
  list.addDelay(MQDCDELAY);
}

void CMQDC32::addInitializeFifo(CVMUSBReadoutList& list)
{
  list.addWrite16(m_base + InitFifo, initamod, 1);
  list.addDelay(MQDCDELAY);
}

CVMUSBResult CMQDC32::executeList(CVMUSB& ctlr, CVMUSBReadoutList& list, const size_t maxBytes)
{
  CVMUSBResult res;

  uint8_t data[maxBytes];
  int status = ctlr.executeList(list, data, maxBytes, &res.nBytes);
  if (status<0) {
    throw (string("CMQDC32::executeList failed with status=") + to_string(status));
  }
  
  // insert the data into the list
  res.data.insert(res.data.begin(), data, data+res.nBytes);

  return res;
}
