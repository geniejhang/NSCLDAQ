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

#include <iostream>
#include "MADC32Registers.h"


using namespace std;

//////////////////////////////////////////////////////////////////////
// Local constants.


/////////////////////////////////////////////////////////////////////////////////
// Data that drives parameter validity checks.
static const char* GateModeValues[2] = {"common", "separate"};
static const char* TimingSourceValues[2] = {"vme", "external"};
static const char* InputCouplingValues[2] = {"AC","DC"};
static const char* PulserModes[3] = {"off","amplitude","pulseramp"};
// Legal values for the resolution...note in this case the default is explicitly defined as 8k

////////////////////////////////////////////////////////////////////////////////
// Constructors and implemented canonical operations:

/* These are largely trivial in nature: */

CMQDC32RdoHdwr::CMQDC32RdoHdwr() :
  CReadoutHardware(),
  m_logic(),
  m_pConfiguration(0) 
{}


/*! Copy construction involves a deep copy */

CMQDC32RdoHdwr::CMQDC32RdoHdwr(const CMQDC32RdoHdwr& rhs) :
  CReadoutHardware(rhs),
  m_pConfiguration(0)
{
  if (rhs.m_pConfiguration) {
    m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));
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

  m_pConfiguration = &configuration;

  // Create the configuration parameters.
  m_pConfiguration->addIntegerParameter("-base", 0);
  m_pConfiguration->addIntegerParameter("-id",  0, 0xff, 0);

  m_pConfiguration->addBooleanParameter("-timestamp", false);

  m_pConfiguration->addBooleanParameter("-usethresholds", false);
  m_pConfiguration->addIntListParameter("-thresholds",
                                        0, 0x1fff,  // min val, max val (13 bits)
                                        32, 32, 32, // min size, max size, default size
                                        0);  // def value


  // IRQ related details
  m_pConfiguration->addIntegerParameter("-ipl", 0, 7, 0);
  m_pConfiguration->addIntegerParameter("-vector", 0, 0xff, 0);
  m_pConfiguration->addIntegerParameter("-irqthreshold", 0, 0xffff, 1);
  m_pConfiguration->addBooleanParameter("-multievent", false);
  m_pConfiguration->addIntegerParameter("-maxtransfer", 0, 0xf, 1); 

  m_pConfiguration->addIntListParameter("-bankoffsets", 
                                        0, 0xff,
                                        2, 2, 2, 128);
  m_pConfiguration->addEnumParameter("-gatemode", GateModeValues, GateModeValues[0]);

  // the hold delays and widths have the same list constraints.
  // just different default values.
  m_pConfiguration->addIntListParameter("-holdwidths", 
                                        0, 0xff, // min val, max val 
                                        2, 2, 2, // min size, max size, default size
                                        15);  // def value
  m_pConfiguration->addIntListParameter("-holddelay",
                                        0, 0x3fff, // min val, max val 
                                        2, 2, 2, // min size, max size, default size
                                        0);  // def value

  // input coupling
  m_pConfiguration->addEnumParameter("-inputcoupling0", 
                                      InputCouplingValues, 
                                      InputCouplingValues[0]);
  m_pConfiguration->addEnumParameter("-inputcoupling1", 
                                      InputCouplingValues, 
                                      InputCouplingValues[0]);

  // test pulser
  m_pConfiguration->addEnumParameter("-pulser", PulserModes, PulserModes[0]);
  m_pConfiguration->addIntegerParameter("-pulseramp", 0, 0xff, 32);

  // the manual doesn't display this... do I not need it?
  //  m_pConfiguration->addBooleanParameter("-gategenerator", false); 

  // output configuration
  m_pConfiguration->addBooleanParameter("-ecltermination", true);
  m_pConfiguration->addBooleanParameter("-ecltiming", false);
  m_pConfiguration->addBooleanParameter("-nimtiming", false);

  // timing 
  m_pConfiguration->addEnumParameter("-timingsource", 
                                     TimingSourceValues, 
                                     TimingSourceValues[0]);
  m_pConfiguration->addIntegerParameter("-timingdivisor", 0, 0xffff, 15);


  // multiplicity filter
  m_pConfiguration->addIntListParameter("-multlowerlimits",
                                        0, 0x3f,
                                        2, 2, 2,
                                        0);
  m_pConfiguration->addIntListParameter("-multupperlimits",
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
  m_logic.resetAll(ctlr);

  unique_ptr<CVMUSBReadoutList> pList(controller.createReadoutList());

  // First disable the interrupts so that we can't get any spurious ones during init.
  m_logic.addDisableInterrupts(*pList);

  configureModuleID(*pList);

//  configureThresholds(list);
//  // there is already a delay inserted after the last threshold.
//
//  configureMarkerType(list);
//
//  configureMemoryBankSeparation(list);
//
//  configureGDG(list);
//  
//  configureTestPulser(list);
//
//  configureInputCoupling(list);
//
//  // Set the timing divisor, and clear the timestamp:
//  configureTimeDivisor(list);
//
//  // Turn on or off ECL termination.  In fact the module provides much more control
//  // over this featuer.. but for now we're all or nothing.
//  configureECLTermination(list);
//
//  // Control which external sources can provide the timebase for the timestamp:
//  configureECLTimeInput(list);
//
//  configureNIMTimeInput(list);
//
//  // Source of the timebase:
//  configureTimeBaseSource(list);
//  
//  // Ensure that busy is on the busy connector:
//
//  list.addWrite16(base + NIMBusyFunction, initamod, (uint16_t)0);
//  list.addDelay(MADCDELAY);
//
//  // Process -resolution, -irqthreshold and -multievent
//  list.addWrite16(base + WithdrawIrqOnEmpty, initamod, (uint16_t)1);
//  configureMultiEventMode(list);

  // Finally clear the converter and set the IPL which enables interrupts if
  // the IPL is non-zero, and does no harm if it is zero.

//  uint8_t     ipl         = m_pConfiguration->getIntegerParameter("-ipl");
//  uint8_t     ivector     = m_pConfiguration->getIntegerParameter("-vector");
//  int         irqThreshold= m_pConfiguration->getIntegerParameter("-irqthreshold");
//  list.addWrite16(base + Vector,   initamod, (uint16_t)ivector);
//  list.addDelay(MADCDELAY);
//  list.addWrite16(base + Ipl, initamod, (uint16_t)ipl);
//  list.addWrite16(base + IrqThreshold, initamod, (uint16_t)irqThreshold);
//  list.addDelay(MADCDELAY);

  m_logic.addResetReadout(*pList);
  m_logic.addInitializeFifo(*pList);
  m_logic.addWriteAcquisitionState(*pList,true);

  //  Execute the list to initialize the module:
  char readBuffer[100];		// really a dummy as these are all write...
  size_t bytesRead;
  int status = controller.executeList(*pList, readBuffer, sizeof(readBuffer), &bytesRead);
  if (status != 0) {
     throw string("List excecution to initialize an MADC32 failed");
   }

}

uint32_t CMQDC32RdoHdwr::getBase() {
  return m_pConfiguration->getUnsignedParameter("-base");
}

//
void CMQDC32RdoHdwr::configureModuleID(CVMUSBReadoutList& list) {
  uint16_t id = m_pConfiguration->getIntegerParameter("-id");
  m_logic.addWriteModuleID(list,id);
}

void CMQDC32RdoHdwr::configureThresholds(CVMUSBReadoutList& list) {
  if (m_pConfiguration->getBoolParameter("-usethresholds")) {
    vector<int> thresholds  = m_pConfiguration->getIntegerList("-thresholds");
    m_logic.addWriteThresholds(list,thresholds);
  } else {
    m_logic.addWriteThresholds(list,vector<int>(32,0));
  }
}


// timestamp --> extended always
void CMQDC32RdoHdwr::configureMarkerType(CVMUSBReadoutList& list) {
  bool        timestamp   = m_pConfiguration->getBoolParameter("-timestamp");
  m_logic.addWriteMarkerType(list,(uint16_t)(timestamp ? 3 : 0));
}


void CMQDC32RdoHdwr::configureMemoryBankSeparation(CVMUSBReadoutList& list) {
  string      gatemode    = m_pConfiguration->cget("-gatemode");
  if (gatemode == string("separate")) {
    m_logic.addWriteMemoryBankSeparation(list,1);
  }
  else {
    m_logic.addWriteMemoryBankSeparation(list,0);
  }									
}

void CMQDC32RdoHdwr::configureGDG(CVMUSBReadoutList& list) {
  // If the gate generator is on, we need to program the hold delays and widths
  // as well as enable it.

  bool        gdg         = m_pConfiguration->getBoolParameter("-gategenerator");
  vector<int> holddelays  = m_pConfiguration->getIntegerList("-holddelays");
  vector<int> holdwidths  = m_pConfiguration->getIntegerList("-holdwidths");
  uint32_t base = getBase();
  if(gdg) {
    list.addWrite16(base + HoldDelay0, initamod, (uint16_t)holddelays[0]);
    list.addDelay(MADCDELAY);
    list.addWrite16(base + HoldDelay1, initamod, (uint16_t)holddelays[1]);
    list.addDelay(MADCDELAY);

    list.addWrite16(base + HoldWidth0, initamod, (uint16_t)holdwidths[0]);
    list.addDelay(MADCDELAY);
    list.addWrite16(base + HoldWidth1, initamod, (uint16_t)holdwidths[1]);
    list.addDelay(MADCDELAY);
    
    list.addWrite16(base + EnableGDG, initamod, (uint16_t)1);
    list.addDelay(MADCDELAY);
  } else {
    list.addWrite16(base + EnableGDG, initamod, (uint16_t)0);
    list.addDelay(MADCDELAY);
  }
}


void CMQDC32RdoHdwr::configureTestPulser(CVMUSBReadoutList& list) {
  int  modeIndex = m_pConfiguration->getEnumParameter("-pulser", PulserModes);
}

void CMQDC32RdoHdwr::configureInputCoupling(CVMUSBReadoutList& list) {
  uint32_t base = getBase();
  // Needs implementation!!
  list.addDelay(MADCDELAY);
}


void CMQDC32RdoHdwr::configureTimeDivisor(CVMUSBReadoutList& list) {
  int         timedivisor = m_pConfiguration->getIntegerParameter("-timingdivisor");
  uint32_t base = getBase();
  list.addWrite16(base + TimingDivisor, initamod, (uint16_t)timedivisor);
  list.addDelay(MADCDELAY);
  list.addWrite16(base + TimestampReset, initamod, (uint16_t)3); // Reset both counters.
  list.addDelay(MADCDELAY);
}

void CMQDC32RdoHdwr::configureECLTermination(CVMUSBReadoutList& list) {
  bool        termination = m_pConfiguration->getBoolParameter("-ecltermination");
  uint32_t base = getBase();
  if(termination) {
    list.addWrite16(base + ECLTermination, initamod, (uint16_t)0xf);
  }
  else {
    list.addWrite16(base + ECLTermination, initamod, (uint16_t)0);
  }
  list.addDelay(MADCDELAY);
}

void CMQDC32RdoHdwr::configureECLTimeInput(CVMUSBReadoutList& list) {
  bool        ecltimeinput= m_pConfiguration->getBoolParameter("-ecltiming");
  uint32_t base = getBase();
  if (ecltimeinput) {
    list.addWrite16(base + ECLGate1OrTiming, initamod, (uint16_t)1);
    list.addDelay(MADCDELAY);
    list.addWrite16(base + ECLFCOrTimeReset, initamod, (uint16_t)1);
    list.addDelay(MADCDELAY);
  }
  else {
    list.addWrite16(base + ECLGate1OrTiming, initamod, (uint16_t)0);
    list.addDelay(MADCDELAY);
    list.addWrite16(base + ECLFCOrTimeReset, initamod, (uint16_t)0);
    list.addDelay(MADCDELAY);
  }
}

void CMQDC32RdoHdwr::configureNIMTimeInput(CVMUSBReadoutList& list) {
  bool        nimtimeinput= m_pConfiguration->getBoolParameter("-nimtiming");
  uint32_t base = getBase();
  if (nimtimeinput) {
    list.addWrite16(base + NIMGate1OrTiming, initamod, (uint16_t)1);
    list.addDelay(MADCDELAY);
    list.addWrite16(base + NIMFCOrTimeReset, initamod, (uint16_t)1);
    list.addDelay(MADCDELAY);
  }
  else {
    list.addWrite16(base + NIMGate1OrTiming, initamod, (uint16_t)0);
    list.addDelay(MADCDELAY);
    list.addWrite16(base + NIMFCOrTimeReset, initamod, (uint16_t)0);
    list.addDelay(MADCDELAY);
  }
}


void CMQDC32RdoHdwr::configureTimeBaseSource(CVMUSBReadoutList& list) {
  string      timesource  = m_pConfiguration->cget("-timingsource");
  uint32_t base = getBase();
  if(timesource == string("vme") ) {
    list.addWrite16(base + TimingSource, initamod, (uint16_t)0);
  }
  else {
    list.addWrite16(base + TimingSource, initamod, (uint16_t)1);
  }
  list.addDelay(MADCDELAY);
}


void CMQDC32RdoHdwr::configureMultiEventMode(CVMUSBReadoutList& list) {
  bool        multiEvent  = m_pConfiguration->getBoolParameter("-multievent");
  uint32_t base = getBase();
  if(multiEvent) {
    list.addWrite16(base + MultiEvent, initamod, (uint16_t)7);
  }
  else {
    list.addWrite16(base + MultiEvent, initamod, (uint16_t)0);
  }
  list.addDelay(MADCDELAY);
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
  // Need the base:

  uint32_t base = m_pConfiguration->getUnsignedParameter("-base");

  list.addFifoRead32(base + eventBuffer, readamod, (size_t)45);
  list.addWrite16(base + ReadoutReset, initamod, (uint16_t)1);
  list.addDelay(5);
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

  uint32_t base = m_pConfiguration->getIntegerParameter("-base");

  cerr << "Position: " << position << endl;

  // Compute the value of the control register..though we will first program
  // the addresses then the control register:

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

  controller.vmeWrite16(base + CbltAddress, initamod, (uint16_t)(cbltBase >> 24));
  controller.vmeWrite16(base + McstAddress, initamod, (uint16_t)(mcastBase >> 24));
  controller.vmeWrite16(base + CbltMcstControl, initamod, (uint16_t)(controlRegister));

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


  int irqThreshold   = m_pConfiguration->getIntegerParameter("-irqthreshold");
  int vector         = m_pConfiguration->getIntegerParameter("-vector");
  int ipl            = m_pConfiguration->getIntegerParameter("-ipl");
  bool timestamping  = m_pConfiguration->getBoolParameter("-timestamp");
  
  // Stop acquistiion
  // ..and clear buffer memory:
  controller.vmeWrite16(mcast + StartAcq, initamod, (uint16_t)0);
  controller.vmeWrite16(mcast + InitFifo, initamod, (uint16_t)0);

  // Set stamping

  if(timestamping) {
    // Oscillator sources are assumed to already be set.
    // Reset the timer:

    controller.vmeWrite16(mcast + MarkType,       initamod, (uint16_t)1); // Show timestamp, not event count.
    controller.vmeWrite16(mcast + TimestampReset, initamod, (uint16_t)3); // reset all counter.
  }
  else {
    controller.vmeWrite16(mcast + MarkType,       initamod, (uint16_t)0); // Use Eventcounter.
    controller.vmeWrite16(mcast + EventCounterReset, initamod, (uint16_t)0); // Reset al event counters.
  }
  // Set multievent mode
  
  controller.vmeWrite16(mcast + MultiEvent, initamod, (uint16_t)3);      // Multi event mode 3.
  controller.vmeWrite16(mcast + IrqThreshold, initamod, (uint16_t)irqThreshold);
  controller.vmeWrite16(mcast + MaxTransfer, initamod,  (uint16_t)rdoSize);

  // Set the IRQ

  controller.vmeWrite16(mcast + Vector, initamod, (uint16_t)vector);
  controller.vmeWrite16(mcast + Ipl,    initamod, (uint16_t)ipl);
  controller.vmeWrite16(mcast + IrqThreshold, initamod, (uint16_t)irqThreshold);

  // Init the buffer and start data taking.

  controller.vmeWrite16(mcast + InitFifo, initamod, (uint16_t)0);
  controller.vmeWrite16(mcast + ReadoutReset, initamod, (uint16_t)0);
  controller.vmeWrite16(mcast + StartAcq , initamod, (uint16_t)1);
}


///////////////////////////////////////////////////////////////////////////////////////////////
//
// Private utilities:
//

