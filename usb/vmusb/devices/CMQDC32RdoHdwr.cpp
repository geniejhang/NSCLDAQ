/*
    This software is Copyright by the Board of Trustees of Michigan


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
#include "CMQDC32RdoHdwr.h"
#include "CReadoutModule.h"
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>

#include <tcl.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <string>
#include <vector>
#include <set>
#include <memory>
#include <stdexcept>

#include <iostream>
#include "MQDC32Registers.h"


using namespace std;

//////////////////////////////////////////////////////////////////////
// Local constants.


/////////////////////////////////////////////////////////////////////////////////
// Data that drives parameter validity checks.
static const char* GateModeValues[] = {"common", "separate",0};
static const char* TimingSourceValues[] = {"vme", "external",0};
static const char* InputCouplingValues[] = {"AC","DC",0};
static const char* PulserModes[] = {"off","fixedamplitude","useramplitude",0};
static const char* NIMBusyModes[] = {"busy", "rcbus", "full", "overthreshold",0};
static const char* SyncModeValues[] = {"never","resetall","ctraonly", "ctrbonly", "external",0};
// Legal values for the resolution...note in this case the default is explicitly defined as 8k

////////////////////////////////////////////////////////////////////////////////
// Constructors and implemented canonical operations:

/* These are largely trivial in nature: */

CMQDC32RdoHdwr::CMQDC32RdoHdwr() :
  CReadoutHardware(),
  m_logic(),
  m_pConfig(0) 
{}


/*! Copy construction involves a deep copy */

CMQDC32RdoHdwr::CMQDC32RdoHdwr(const CMQDC32RdoHdwr& rhs) :
  CReadoutHardware(rhs),
  m_pConfig(0)
{
  if (rhs.m_pConfig) {
    m_pConfig = new CReadoutModule(*(rhs.m_pConfig));
  }
}

CMQDC32RdoHdwr::~CMQDC32RdoHdwr() {}

CMQDC32RdoHdwr&
CMQDC32RdoHdwr::operator=(const CMQDC32RdoHdwr& rhs)
{
  return *this;
}
/////////////////////////////////////////////////////////////////////////////////
// Object operations:
//

/*!
   Attach the module to its configuration.
   This is called when the object has been created by the configuration software.
   we will register our configuration parameters, validators and limits.
   A pointer to the configuration object is retained so that The module configuration
   can be gotten when we need it.

   \param configuration - The Readout module that will hold our configuration.

*/
void
CMQDC32RdoHdwr::onAttach(CReadoutModule& configuration)
{

  m_pConfig = &configuration;

  // Create the configuration parameters.
  m_pConfig->addIntegerParameter("-base", 0);
  m_pConfig->addIntegerParameter("-id",  0, 0xff, 0);

  m_pConfig->addBooleanParameter("-timestamp", false);

  m_pConfig->addBooleanParameter("-usethresholds", false);
  m_pConfig->addIntListParameter("-thresholds",
                                        0, MQDC32::Thresholds::Max, // min val, max val (13 bits)
                                        32, 32, 32, // min size, max size, default size
                                        0);  // def value


  // IRQ related details
  m_pConfig->addIntegerParameter("-ipl", 0, 7, 0);
  m_pConfig->addIntegerParameter("-vector", 0, 0xff, 0);
  m_pConfig->addIntegerParameter("-irqthreshold", 0, 0xffff, 1);
  m_pConfig->addBooleanParameter("-multievent", false);

  m_pConfig->addIntListParameter("-bankoffsets", 
                                        0, MQDC32::BankOffsets::Max,
                                        2, 2, 2, 128);
  m_pConfig->addEnumParameter("-gatemode", GateModeValues, GateModeValues[0]);

  // the hold delays and widths have the same list constraints.
  // just different default values.
  m_pConfig->addIntListParameter("-gatelimits", 
                                        0, MQDC32::GateLimit::Max, // min val, max val 
                                        2, 2, 2, // min size, max size, default size
                                        MQDC32::GateLimit::Max);  // no limit 
  m_pConfig->addIntListParameter("-exptrigdelays",
                                        0, MQDC32::ExpTrigDelay::Max, // min val, max val 
                                        2, 2, 2, // min size, max size, default size
                                        0);  // def value

  // input coupling
  m_pConfig->addEnumParameter("-inputcoupling0", 
                                      InputCouplingValues, 
                                      InputCouplingValues[0]);
  m_pConfig->addEnumParameter("-inputcoupling1", 
                                      InputCouplingValues, 
                                      InputCouplingValues[0]);

  // test pulser
  m_pConfig->addEnumParameter("-pulser", PulserModes, PulserModes[0]);
  m_pConfig->addIntegerParameter("-pulseramp", 0, 0xff, 32);

  // output configuration
  m_pConfig->addBooleanParameter("-ecltermination", true);
  m_pConfig->addBooleanParameter("-ecltiming", false);
  m_pConfig->addBooleanParameter("-nimtiming", false);
  m_pConfig->addEnumParameter("-nimbusy", NIMBusyModes, NIMBusyModes[0]);

  // timing 
  m_pConfig->addEnumParameter("-timingsource", 
                                     TimingSourceValues, 
                                     TimingSourceValues[0]);
  m_pConfig->addIntegerParameter("-timingdivisor", 0, 0xffff, 15);
  m_pConfig->addEnumParameter("-syncmode",SyncModeValues,SyncModeValues[1]);

  // multiplicity filter
  m_pConfig->addIntListParameter("-multlowerlimits",
                                        0, 0x3f,
                                        2, 2, 2,
                                        0);
  m_pConfig->addIntListParameter("-multupperlimits",
      0, 0x3f,
      2, 2, 2,
      32);

}
/*!
   Initialize the module prior to data taking.  We will get the initialization
   data from the configuration.  Unfortunately, there's no way to verify the
   base address we were given actually points to a module.

   \param CVMUSB&controller   References a VMSUB controller that will be used
          to initilize the module (the module is in a VME crate connected to that
          VMUSB object.
*/

unique_ptr<CVMUSBReadoutList> getList(CVMUSB& ctlr) 
{
  return unique_ptr<CVMUSBReadoutList>(ctlr.createReadoutList());
}

void execList(CVMUSB& ctlr, unique_ptr<CVMUSBReadoutList>& list) 
{
  auto res = ctlr.executeList(*list,128);
  if (res.size()==0) {
    throw std::runtime_error("Failure while executing list.");
  }
}

void
CMQDC32RdoHdwr::Initialize(CVMUSB& controller)
{
  // Locate the module and reset it and the fifo.
  // These operations are the only individual operations and are done
  // in case time is needed between reset and the next operations on the module.
  // The remaining operations on the module will be done in 
  // a list so that they don't take so much time.


  CVMUSB& ctlr = controller;
  m_logic.setBase(getBase());
  //  m_logic.resetAll(ctlr);
  {
    unique_ptr<CVMUSBReadoutList> pList(controller.createReadoutList());
    m_logic.addSoftReset(*pList);
    m_logic.addWriteAcquisitionState(*pList,0);
    auto res = controller.executeList(*pList,128);
    if (res.size()==0) {
      throw std::runtime_error("Failure while executing list.");
    }
  }

  unique_ptr<CVMUSBReadoutList> pList(controller.createReadoutList());

  // First disable the interrupts so that we can't get any spurious ones during init.
  m_logic.addDisableInterrupts(*pList);

  configureModuleID(*pList);

  // Configure the IRQ and buffering mode
  configureIrq(*pList);
  configureMultiEventMode(*pList);

  // configure conversion / acquisition parameters
  configureThresholds(*pList);
  configureGates(*pList);
  configureBankOffsets(*pList);
  //
  //  // 
  //  configureMemoryBankSeparation(*pList);
  //
  //  // configure inputs/outputs
  //  configureECLInputs(*pList);
  //  configureNIMInputs(*pList);
  //  configureNIMBusy(*pList);
  configureInputCoupling(*pList);
  //  configureECLTermination(*pList);
  //  //  ... timestamp
  //  configureTimeBaseSource(*pList);
  //  configureTimeDivisor(*pList);
  //  configureMarkerType(*pList);
  configureCounterReset(*pList);
  //

  // see page 29 of MQDC manual for starting the readout.
  // 1. Fifo reset
  // 2. Readout reset
  // 3. start acquisition
  m_logic.addInitializeFifo(*pList);
  m_logic.addResetReadout(*pList);
  m_logic.addWriteAcquisitionState(*pList,true);

  // test pulser
  configureTestPulser(*pList);
  auto result = ctlr.executeList(*pList, 8);
  if (result.size()==0) {
    throw std::runtime_error("Failure while executing list.");
  }

}

uint32_t CMQDC32RdoHdwr::getBase() {
  return m_pConfig->getUnsignedParameter("-base");
}

//
void CMQDC32RdoHdwr::configureModuleID(CVMUSBReadoutList& list) {
  uint16_t id = m_pConfig->getIntegerParameter("-id");
  m_logic.addWriteModuleID(list,id);
}

void CMQDC32RdoHdwr::configureThresholds(CVMUSBReadoutList& list) {
  if (m_pConfig->getBoolParameter("-usethresholds")) {

    // make sure that we honor thresholds that we are given
    m_logic.addWriteIgnoreThresholds(list,false);

    // pass the user's thresholds to the stack
    auto thresholds  = m_pConfig->getIntegerList("-thresholds");
    m_logic.addWriteThresholds(list,thresholds);
  } else {
    // set all thresholds to 0 (disables them)
    m_logic.addWriteThresholds(list,vector<int>(32,0));
    // ignore thresholds. period.
    m_logic.addWriteIgnoreThresholds(list,true);
  }
}


// timestamp --> extended always
void CMQDC32RdoHdwr::configureMarkerType(CVMUSBReadoutList& list) {
  using namespace MQDC32::MarkerType;
  if (m_pConfig->getBoolParameter("-timestamp")) {
    m_logic.addWriteMarkerType(list, Timestamp46Bit);
  } else {
    m_logic.addWriteMarkerType(list, EventCount);
  }
}


void CMQDC32RdoHdwr::configureMemoryBankSeparation(CVMUSBReadoutList& list) {
  string      gatemode    = m_pConfig->cget("-gatemode");
  if (gatemode == string("separate")) {
    m_logic.addWriteMemoryBankSeparation(list,1);
  }
  else {
    m_logic.addWriteMemoryBankSeparation(list,0);
  }									
}

void CMQDC32RdoHdwr::configureGates(CVMUSBReadoutList& list) {

  auto exptrigdelays = m_pConfig->getIntegerList("-exptrigdelays");
  auto gatelimits = m_pConfig->getIntegerList("-gatelimits");

  m_logic.addWriteExpTrigDelays(list, exptrigdelays);
  m_logic.addWriteGateLimits(list, gatelimits);
}

void CMQDC32RdoHdwr::configureBankOffsets(CVMUSBReadoutList& list) {
  auto offsets = m_pConfig->getIntegerList("-bankoffsets"); 
  m_logic.addWriteBankOffsets(list, offsets);
}


void CMQDC32RdoHdwr::configureTestPulser(CVMUSBReadoutList& list) {
  using namespace MQDC32::Pulser;

  int  modeIndex = m_pConfig->getEnumParameter("-pulser", PulserModes);
  int  amplitude = m_pConfig->getUnsignedParameter("-pulseramp");
  switch (modeIndex) {
    case 0:
      // off
      m_logic.addWritePulserState(list, Off);
      break;
    case 1:
      // fixed amplitude
      cout << "Fixed amplitude" << endl;
      m_logic.addWritePulserState(list, FixedAmplitude);
      break;
    case 2:
      // user defined amplitude
      m_logic.addWritePulserState(list,UserAmplitude);
      m_logic.addWritePulserAmplitude(list, amplitude);
      break;
    default:
      // do nothing.
      break;
  }
}

void CMQDC32RdoHdwr::configureInputCoupling(CVMUSBReadoutList& list) {
  uint16_t coupling0 = m_pConfig->getEnumParameter("-inputcoupling0",InputCouplingValues); 
  uint16_t coupling1 = m_pConfig->getEnumParameter("-inputcoupling1",InputCouplingValues); 

  uint16_t coupling = (coupling0|(coupling1<<1));
  m_logic.addWriteInputCoupling(list, coupling);
}


void CMQDC32RdoHdwr::configureTimeDivisor(CVMUSBReadoutList& list) {
  uint16_t timedivisor = m_pConfig->getUnsignedParameter("-timingdivisor");

  m_logic.addWriteTimeDivisor(list, timedivisor);
  m_logic.addResetTimestamps(list);
}

void CMQDC32RdoHdwr::configureECLTermination(CVMUSBReadoutList& list) {
  if (m_pConfig->getBoolParameter("-ecltermination")) {
    // terminate all!
    m_logic.addWriteECLTermination(list, 0xf);
  }
  else {
    // terminate nothing
    m_logic.addWriteECLTermination(list, 0);
  }
}

void CMQDC32RdoHdwr::configureECLInputs(CVMUSBReadoutList& list) {
  
  using namespace MQDC32;

  if (m_pConfig->getBoolParameter("-ecltiming")) {
    m_logic.addWriteECLGate1Input(list, ECLGate1::Oscillator);
    m_logic.addWriteECLFCInput(list,  ECLFC::ResetTstamp);
  } else {
    m_logic.addWriteECLGate1Input(list, ECLGate1::Gate);
    m_logic.addWriteECLFCInput(list,  ECLFC::FastClear);
  }
}

void CMQDC32RdoHdwr::configureNIMInputs(CVMUSBReadoutList& list) {
  
  using namespace MQDC32;
  if (m_pConfig->getBoolParameter("-nimtiming")) {
    m_logic.addWriteNIMGate1Input(list, NIMGate1::Oscillator); 
    m_logic.addWriteNIMFCInput(list, NIMFC::ResetTstamp);
  }
  else {
    m_logic.addWriteNIMGate1Input(list, NIMGate1::Gate);
    m_logic.addWriteNIMFCInput(list, NIMFC::ResetTstamp);
  }
}

void CMQDC32RdoHdwr::configureNIMBusy(CVMUSBReadoutList& list) {
  int mode = m_pConfig->getEnumParameter("-nimbusy",NIMBusyModes);

  using namespace MQDC32::NIMBusy;

  switch (mode) {
    case 0:
      m_logic.addWriteNIMBusyOutput(list, Busy);
      break;
    case 1:
      m_logic.addWriteNIMBusyOutput(list, RCBus);
      break;
    case 2:
      m_logic.addWriteNIMBusyOutput(list, Full);
      break;
    case 3:
      m_logic.addWriteNIMBusyOutput(list, OverThreshold);
      break;
    default:
      // by default we just set it to busy
      m_logic.addWriteNIMBusyOutput(list, Busy);
      break;
  }
}


void CMQDC32RdoHdwr::configureTimeBaseSource(CVMUSBReadoutList& list) {
  uint16_t id = m_pConfig->getEnumParameter("-timingsource",TimingSourceValues);
  m_logic.addWriteTimeBaseSource(list,id);
}

void CMQDC32RdoHdwr::configureIrq(CVMUSBReadoutList& list) {
  uint8_t     ipl         = m_pConfig->getIntegerParameter("-ipl");
  uint8_t     ivector     = m_pConfig->getIntegerParameter("-vector");
  m_logic.addWriteIrqVector(list, ivector);
  m_logic.addWriteIrqLevel(list, ipl);
  m_logic.addWriteWithdrawIrqOnEmpty(list,true);
}

void CMQDC32RdoHdwr::configureMultiEventMode(CVMUSBReadoutList& list) {
  using namespace MQDC32::TransferMode;

  uint16_t nUnits = m_pConfig->getUnsignedParameter("-irqthreshold");
  if(m_pConfig->getBoolParameter("-multievent")) {

    m_logic.addWriteIrqThreshold(list, nUnits);
    m_logic.addWriteTransferCount(list, nUnits);
    m_logic.addWriteMultiEventMode(list, Limited);
  } else {
    m_logic.addWriteIrqThreshold(list, 1);
    m_logic.addWriteTransferCount(list, 1);
    m_logic.addWriteMultiEventMode(list, Limited);

    // warn the user if their -maxtransfer value has been overwritten
    if (nUnits!=1) {
      std::cerr << "User's values for -irqthreshold and -maxtransfer options ";
      std::cerr << "has been overridden ";
      std::cerr << "to be 1 for proper single event readout.";
      std::cerr << std::endl;
    }
  }
}


void CMQDC32RdoHdwr::configureMultiplicity(CVMUSBReadoutList& list) {
  auto lower = m_pConfig->getIntegerList("-multlowerlimits");
  auto upper = m_pConfig->getIntegerList("-multupperlimits");

  m_logic.addWriteLowerMultLimits(list, lower);
  m_logic.addWriteUpperMultLimits(list, upper);
}

void
CMQDC32RdoHdwr::configureCounterReset(CVMUSBReadoutList& list) 
{
  using namespace MQDC32::CounterReset;
  int modeIndex = m_pConfig->getEnumParameter("-syncmode", SyncModeValues);
  switch (modeIndex) {
    case 0:
     m_logic.addWriteCounterReset(list, Never); 
     break; 
    case 1:
     m_logic.addWriteCounterReset(list, (CTRA|CTRB)); 
     break; 
    case 2:
     m_logic.addWriteCounterReset(list, CTRA); 
     break; 
    case 3:
     m_logic.addWriteCounterReset(list, CTRB); 
     break; 
    case 4:
     m_logic.addWriteCounterReset(list, External); 
     break; 
    default:
     // do nothing.
     break; 
  }
}

/*!
  Add instructions to read out the ADC for a event. Since we're only working in
  single even tmode, we'll just read 'too many' words and let the
  BERR terminate for us.  This ensures that we'll have that 0xfff at the end of 
  the data.
  \param list  - The VMUSB read9out list that's being built for this stack.
*/
void
CMQDC32RdoHdwr::addReadoutList(CVMUSBReadoutList& list)
{
  if (m_pConfig->getBoolParameter("-multievent")) {
    uint32_t maxTransfers = m_pConfig->getUnsignedParameter("-irqthreshold");
    m_logic.addFifoRead(list,maxTransfers+40);
  } else {
    m_logic.addFifoRead(list,40);
  }
  
  m_logic.addResetReadout(list);
  list.addDelay(5);
}

void
CMQDC32RdoHdwr::onEndRun(CVMUSB& ctlr) {
  unique_ptr<CVMUSBReadoutList> pList(ctlr.createReadoutList());

  m_logic.addWriteAcquisitionState(*pList,false);
  m_logic.addResetReadout(*pList);

  ctlr.executeList(*pList,8);
}

// Cloning supports a virtual copy constructor.

CReadoutHardware*
CMQDC32RdoHdwr::clone() const
{
  return new CMQDC32RdoHdwr(*this);
}
////////////////////////////////////////////////////////////////////////////////////////////
//
// Code here provides support for the madcchain pseudo module that use these devices
// in CBLT mode.
//

/*!
   Set up the chain/mcast addresses.
   @param controller - Handle to VM_USB controller object.
   @param position   - A value from the position enumerator CMQDC32RdoHdwr::ChainPosition
   @param cbltBase   - Base address for CBLT transfers.
   @param mcastBase  - Base address for multicast transfers.

   Note that both mcast and cblt are enabled for now.
*/
void
CMQDC32RdoHdwr::setChainAddresses(CVMUSB&                controller,
    CMQDC32RdoHdwr::ChainPosition position,
    uint32_t               cbltBase,
    uint32_t               mcastBase)
{

  uint32_t base = m_pConfig->getIntegerParameter("-base");

  cerr << "Position: " << position << endl;

  // Compute the value of the control register..though we will first program
  // the addresses then the control register:
  using namespace MQDC32;

  uint16_t controlRegister = MCSTENB | CBLTENB; // This much is invariant.
  switch (position) {
  case first:
    controlRegister |= FIRSTENB | LASTDIS;
    cerr << "First\n";
    break;
  case middle:
    controlRegister |= FIRSTDIS | LASTDIS;
    cerr << "Middle\n";
    break;
  case last:
    controlRegister |= FIRSTDIS | LASTENB;
    cerr << "Last\n";
    break;
  }
  cerr << "Setting chain address with " << hex << controlRegister << dec << endl;

  // program the registers, note that the address registers take only the top 8 bits.

  controller.vmeWrite16(base + Reg::CbltAddress, initamod, (uint16_t)(cbltBase >> 24));
  controller.vmeWrite16(base + Reg::McstAddress, initamod, (uint16_t)(mcastBase >> 24));
  controller.vmeWrite16(base + Reg::CbltMcstControl, initamod, (uint16_t)(controlRegister));

}

/*!
   Set up data taking for CBLT readout with the timestamp parameters we are using and
   the mcast address for the chain
   @param controller - CVMUSB controller reference.
   @param mcast  - Multicast address used to program the chain.
   @param rdoSize - Words per module.
*/
void
CMQDC32RdoHdwr::initCBLTReadout(CVMUSB& controller, uint32_t mcast, int rdoSize)
{
  // We need our timing source
  // IRQThreshold
  // VECTOR
  // IPL
  // Timestamp on/off

  // Assumptions:  Internal oscillator reset if using timestamp
  //               ..else no reset.
  //               most modulep arameters are already set up.


  int irqThreshold   = m_pConfig->getIntegerParameter("-irqthreshold");
  int vector         = m_pConfig->getIntegerParameter("-vector");
  int ipl            = m_pConfig->getIntegerParameter("-ipl");
  bool timestamping  = m_pConfig->getBoolParameter("-timestamp");
  
  using namespace MQDC32;
  // Stop acquistiion
  // ..and clear buffer memory:
  controller.vmeWrite16(mcast + Reg::StartAcq, initamod, (uint16_t)0);
  controller.vmeWrite16(mcast + Reg::InitFifo, initamod, (uint16_t)0);

  // Set stamping

  if(timestamping) {
    // Oscillator sources are assumed to already be set.
    // Reset the timer:

    controller.vmeWrite16(mcast + Reg::MarkType,       initamod, (uint16_t)1); // Show timestamp, not event count.
    controller.vmeWrite16(mcast + Reg::TimestampReset, initamod, (uint16_t)3); // reset all counter.
  }
  else {
    controller.vmeWrite16(mcast + Reg::MarkType,       initamod, (uint16_t)0); // Use Eventcounter.
    controller.vmeWrite16(mcast + Reg::EventCounterReset, initamod, (uint16_t)0); // Reset al event counters.
  }
  // Set multievent mode
  
  controller.vmeWrite16(mcast + Reg::MultiEvent, initamod, (uint16_t)3);      // Multi event mode 3.
  controller.vmeWrite16(mcast + Reg::IrqThreshold, initamod, (uint16_t)irqThreshold);
  controller.vmeWrite16(mcast + Reg::MaxTransfer, initamod,  (uint16_t)rdoSize);

  // Set the IRQ

  controller.vmeWrite16(mcast + Reg::Vector, initamod, (uint16_t)vector);
  controller.vmeWrite16(mcast + Reg::Ipl,    initamod, (uint16_t)ipl);
  controller.vmeWrite16(mcast + Reg::IrqThreshold, initamod, (uint16_t)irqThreshold);

  // Init the buffer and start data taking.

  controller.vmeWrite16(mcast + Reg::InitFifo, initamod, (uint16_t)0);
  controller.vmeWrite16(mcast + Reg::ReadoutReset, initamod, (uint16_t)0);
  controller.vmeWrite16(mcast + Reg::StartAcq , initamod, (uint16_t)1);
}


///////////////////////////////////////////////////////////////////////////////////////////////
//
// Private utilities:
//

