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
#include "CMADC32.h"
#include "CReadoutModule.h"
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>

#include <tcl.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <string>
#include <vector>
#include <set>

using namespace std;

//////////////////////////////////////////////////////////////////////
// Local constants.

#define Const(name) static const int name =

// The address modifiers that will be used to access the module:

Const(initamod)  CVMUSBReadoutList::a32UserData;   //  setup using user data access.
Const(readamod)  CVMUSBReadoutList::a32UserBlock;  //  Read in block mode.

// Module address map; for the most part I'm only defining the registers
// we'll actually use.

Const(eventBuffer)          0;

Const(Thresholds)           0x4000;

Const(AddressSource)        0x6000;
Const(Address)              0x6002;
Const(ModuleId)             0x6004;
Const(Reset)                0x6008; // write anything here to reset the module.

Const(Ipl)                  0x6010;
Const(Vector)               0x6012;

Const(LongCount)            0x6030;
Const(DataFormat)           0x6032;
Const(ReadoutReset)         0x6034;
Const(MarkType)             0x6038;
Const(StartAcq)             0x603A;
Const(InitFifo)             0x603c;
Const(DataReady)            0x603e;

Const(BankOperation)        0x6040;
Const(Resolution)           0x6042;
Const(OutputFormat)         0x6044;

Const(HoldDelay0)           0x6050;
Const(HoldDelay1)           0x6052;
Const(HoldWidth0)           0x6054;
Const(HoldWidth1)           0x6056;
Const(EnableGDG)            0x6058;

Const(InputRange)           0x6060;
Const(ECLTermination)       0x6062;
Const(ECLGate1OrTiming)     0x6064;
Const(ECLFCOrTimeReset)     0x6066;
Const(NIMGate1OrTiming)      0x606a;
Const(NIMFCOrTimeReset)     0x606c;
Const(NIMBusyFunction)      0x606e;
Const(EventCounterReset)    0x6090;
Const(TimingSource)         0x6096;
Const(TimingDivisor)        0x6098;
Const(TimestampReset)       EventCounterReset; // As of firmware 5.

Const(TestPulser)           0x6070; // In order to ensure it's off !


/////////////////////////////////////////////////////////////////////////////////
// Data that drives parameter validity checks.

static CConfigurableObject::limit Zero(0);    // Useful for many of the integer limits.
static CConfigurableObject::limit Byte(0xff);
// Module id:

static CConfigurableObject::limit IdMax(255);
static CConfigurableObject::Limits IdLimits(Zero, IdMax);

// Interrupt priority level:

static CConfigurableObject::limit IPLMax(7);
static CConfigurableObject::Limits IPLLimit(Zero, IPLMax);

// Interrupt vector:

static CConfigurableObject::limit VectorMax(255);
static CConfigurableObject::Limits VectorLimit(Zero, IPLMax);

// List parameters have constraints on their sizes (HoldListSize),
// Value types, and parameters to the type checker (e.g. range limitations).
// These are encapsulated in isListParameter struct.
//
// Thresholds, are 32 element lists with values [0,0xfff].

static CConfigurableObject::limit ThresholdMax(0xfff);
static CConfigurableObject::Limits ThresholdLimits(Zero, ThresholdMax);
static CConfigurableObject::ListSizeConstraint ThresholdListSize = {32, 32};
static CConfigurableObject::TypeCheckInfo ThresholdValuesOk(CConfigurableObject::isInteger,
							    &ThresholdLimits);
static CConfigurableObject::isListParameter ThresholdValidity = 
  {ThresholdListSize,
   ThresholdValuesOk};


// hold delays are a two element integer array, with no limits.


static CConfigurableObject::ListSizeConstraint HoldListSize = {2, 2};
static CConfigurableObject::Limits HoldLimits(Zero, Byte);
static CConfigurableObject::TypeCheckInfo HoldValueOk(CConfigurableObject::isInteger,
						      &HoldLimits);
static CConfigurableObject::isListParameter HoldValidity =
  {HoldListSize,
   HoldValueOk};


// Note for all enums, the first item in the list is the default.


// Legal gatemode values for the enumerator:

const char* GateModeValues[2] = {"common", "separate"};
const int   GateModeNumValues = sizeof(GateModeValues)/sizeof(char*);

// Legal values for the adc input range:

const char* InputRangeValues[3] = {"4v", "8v", "10v"};
const int   InputRangeNumValues = sizeof(InputRangeValues)/sizeof(char*);

// Legal values for the timing source.

const char* TimingSourceValues[2] = {"vme", "external"};
const int   TimingSourceNumValues = sizeof(TimingSourceValues)/sizeof(char*);

// Legal values for the timing divisor (0-15)

static CConfigurableObject::limit divisorLimit(0xffff);
static CConfigurableObject::Limits DivisorLimits(Zero, divisorLimit);

////////////////////////////////////////////////////////////////////////////////
// Constructors and implemented canonical operations:

/* These are largely trivial in nature: */

CMADC32::CMADC32() :
  m_pConfiguration(0) 
{}


/*! Copy construction involves a deep copy */

CMADC32::CMADC32(const CMADC32& rhs) :
  m_pConfiguration(0)
{
  if (rhs.m_pConfiguration) {
    m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));
  }
}

CMADC32::~CMADC32() {}

CMADC32&
CMADC32::operator=(const CMADC32& rhs)
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
CMADC32::onAttach(CReadoutModule& configuration)
{

  m_pConfiguration = &configuration;

  // Create the configuration parameters.

  m_pConfiguration->addParameter("-base", CConfigurableObject::isInteger,
				 NULL, "0");
  m_pConfiguration->addParameter("-id",   CConfigurableObject::isInteger,
				 &IdLimits, "0");
  m_pConfiguration->addParameter("-ipl", CConfigurableObject::isInteger,
				 &IPLLimit, "0");
  m_pConfiguration->addParameter("-vector", CConfigurableObject::isInteger,
				 &VectorLimit, "0");
  m_pConfiguration->addParameter("-timestamp", CConfigurableObject::isBool,
				 NULL, "false");

  // Create the enumeration and register the -gatemode parameter.

  static CConfigurableObject::isEnumParameter ValidGateMode;
  for (int i=0; i < GateModeNumValues; i++) {
    ValidGateMode.insert(GateModeValues[i]);
  }
  m_pConfiguration->addParameter("-gatemode", CConfigurableObject::isEnum,
				 &ValidGateMode, GateModeValues[0]);

  // the hold delays and widths have the same list constraints.
  // just different default values.

  m_pConfiguration->addParameter("-holddelays", CConfigurableObject::isIntList,
				 &HoldValidity, "15");
  m_pConfiguration->addParameter("-holdwidths", CConfigurableObject::isIntList,
				 &HoldValidity, "20");

  m_pConfiguration->addParameter("-gategenerator", CConfigurableObject::isBool,
				 NULL, "false");
  // Input range:

  static CConfigurableObject::isEnumParameter ValidInputRange;
  for (int i = 0; i < InputRangeNumValues; i++) {
    ValidInputRange.insert(InputRangeValues[i]);
  }
  m_pConfiguration->addParameter("-inputrange", CConfigurableObject::isEnum,
				 &ValidInputRange, InputRangeValues[0]);


  m_pConfiguration->addParameter("-ecltermination", CConfigurableObject::isBool,
				 NULL, "true");
  m_pConfiguration->addParameter("-ecltiming",      CConfigurableObject::isBool,
				 NULL, "false");
  m_pConfiguration->addParameter("-nimtiming",      CConfigurableObject::isBool,
				 NULL, "false");
  // The timing source enum..

  static CConfigurableObject::isEnumParameter ValidTimingSource;
  for (int i = 0; i < TimingSourceNumValues; i++) {
    ValidTimingSource.insert(TimingSourceValues[i]);
  }
  m_pConfiguration->addParameter("-timingsource", CConfigurableObject::isEnum,
				 &ValidTimingSource, TimingSourceValues[0]);

  m_pConfiguration->addParameter("-timingdivisor", CConfigurableObject::isInteger,
				 &DivisorLimits, "15");


  m_pConfiguration->addParameter("-thresholds", CConfigurableObject::isIntList,
				 &ThresholdValidity, "0");


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
CMADC32::Initialize(CVMUSB& controller)
{
  // Locate the module and reset it and the fifo.
  // These operations are the only individual operations and are done
  // in case time is needed between reset and the next operations on the module.
  // The remaining operations on the module will be done in 
  // a list so that they don't take so damned much time.
 

  uint32_t base = m_pConfiguration->getUnsignedParameter("-base");
  controller.vmeWrite16(base + Reset,    initamod, 1);
  controller.vmeWrite16(base + StartAcq, initamod, 0);
  controller.vmeWrite16(base + ReadoutReset, initamod, 1);

  CVMUSBReadoutList list;	// Initialization instructions will be added to this.

  // First disable the interrupts so that we can't get any spurious ones during init.

  list.addWrite16(base + Ipl, initamod, 0);

  // Now retrieve the configuration parameters:

  uint16_t    id          = m_pConfiguration->getIntegerParameter("-id");
  uint8_t     ipl         = m_pConfiguration->getIntegerParameter("-ipl");
  uint8_t     ivector     = m_pConfiguration->getIntegerParameter("-vector");
  bool        timestamp   = m_pConfiguration->getBoolParameter("-timestamp");
  string      gatemode    = m_pConfiguration->cget("-gatemode");
  vector<int> holddelays  = m_pConfiguration->getIntegerList("-holddelays");
  vector<int> holdwidths  = m_pConfiguration->getIntegerList("-holdwidths");
  bool        gdg         = m_pConfiguration->getBoolParameter("-gategenerator");
  string      inputrange  = m_pConfiguration->cget("-inputrange");
  bool        termination = m_pConfiguration->getBoolParameter("-ecltermination");
  bool        ecltimeinput= m_pConfiguration->getBoolParameter("-ecltiming");
  bool        nimtimeinput= m_pConfiguration->getBoolParameter("-nimtiming");
  string      timesource  = m_pConfiguration->cget("-timingsource");
  int         timedivisor = m_pConfiguration->getIntegerParameter("-timingdivisor");
  vector<int> thresholds  = m_pConfiguration->getIntegerList("-thresholds");

  // Write the thresholds.

  for (int i =0; i < 32; i++) {
    list.addWrite16(base + Thresholds + i*sizeof(uint16_t), initamod, thresholds[i]);
  }

  list.addWrite16(base + ModuleId, initamod, id); // Module id.
  list.addWrite16(base + Vector,   initamod, ivector);

  list.addWrite16(base + MarkType, initamod, timestamp ? 1 : 0); 

  if (gatemode == string("separate")) {
    list.addWrite16(base + BankOperation, initamod, 1);
  }
  else {
    list.addWrite16(base  + BankOperation, initamod, 0);
  }
  // If the gate generator is on, we need to program the hold delays and widths
  // as well as enable it.

  if(gdg) {
    list.addWrite16(base + HoldDelay0, initamod, holddelays[0]);
    list.addWrite16(base + HoldDelay1, initamod, holddelays[1]);

    list.addWrite16(base + HoldWidth0, initamod, holdwidths[0]);
    list.addWrite16(base + HoldWidth1, initamod, holdwidths[1]);
    
    list.addWrite16(base + EnableGDG, initamod, 1);
  } else {
    list.addWrite16(base + EnableGDG, initamod, 0);
  }
  
  // Set the input range:

  if (inputrange == string("4v")) {
    list.addWrite16(base + InputRange, initamod, 0);
  }
  else if (inputrange == string("8v")) {
    list.addWrite16(base + InputRange, initamod, 1);
  }
  else {			// 10V
    list.addWrite16(base + InputRange, initamod, 2);
  }

  // Set the timing divisor, and clear the timestamp:


  list.addWrite16(base + TimingDivisor, initamod, timedivisor);
  list.addWrite16(base + TimestampReset, initamod, 3); // Reset both counters.


  // Turn on or off ECL termination.  In fact the module provides much more control
  // over this featuer.. but for now we're all or nothing.

  if(termination) {
    list.addWrite16(base + ECLTermination, initamod, 0xf);
  }
  else {
    list.addWrite16(base + ECLTermination, initamod, 0);
  }

  // Control which external sources can provide the timebase for the timestamp:

  if (ecltimeinput) {
    list.addWrite16(base + ECLGate1OrTiming, initamod, 1);
    list.addWrite16(base + ECLFCOrTimeReset, initamod, 1);
  }
  else {
    list.addWrite16(base + ECLGate1OrTiming, initamod, 0);
    list.addWrite16(base + ECLFCOrTimeReset, initamod, 0);
  }

  if (nimtimeinput) {
    list.addWrite16(base + NIMGate1OrTiming, initamod, 1);
    list.addWrite16(base + NIMFCOrTimeReset, initamod, 1);
  }
  else {
    list.addWrite16(base + NIMGate1OrTiming, initamod, 0);
    list.addWrite16(base + NIMFCOrTimeReset, initamod, 0);
  }

  // Source of the timebase:

  if(timesource == string("vme") ) {
    list.addWrite16(base + TimingSource, initamod, 0);
  }
  else {
    list.addWrite16(base + TimingSource, initamod, 1);
  }
  

  // Finally clear the converter and set the IPL which enables interrupts if
  // the IPL is non-zero, and does no harm if it is zero.

  list.addWrite16(base + ReadoutReset, initamod, 0);
  list.addWrite16(base + InitFifo,     initamod, 0);

  list.addWrite16(base + Ipl, initamod, ipl);


  // Now reset again and start daq:

  list.addWrite16(base + ReadoutReset, initamod, 1);
  list.addWrite16(base + StartAcq, initamod, 1 );


  //  Execute the list to initialize the module:


  char readBuffer[100];		// really a dummy as these are all write...
  size_t bytesRead;
  int status = controller.executeList(list, readBuffer, sizeof(readBuffer), &bytesRead);
  if (status != 0) {
     throw string("List excecution to initialize an MADC32 failed");
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
CMADC32::addReadoutList(CVMUSBReadoutList& list)
{
  // Need the base:

  int base = m_pConfiguration->getUnsignedParameter("-base");

  list.addFifoRead32(base + eventBuffer, readamod, 45);
  list.addWrite16(base + ReadoutReset, initamod, 1);
  list.addDelay(5);
}

// Cloning supports a virtual copy constructor.

CReadoutHardware*
CMADC32::clone() const
{
  return new CMADC32(*this);
}
