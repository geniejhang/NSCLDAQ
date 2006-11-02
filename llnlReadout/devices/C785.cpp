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

// Implementation of the C785 class VM-USB support for the CAEN V785.


#include <config.h>
#include "C785.h"

#include "CReadoutModule.h"
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>

#include <tcl.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <string>
#include <set>

using namespace std;

////////////////////////////////////////////////////////////////////////
///////////////////////// Local constants //////////////////////////////
////////////////////////////////////////////////////////////////////////
#define Const(name) static const int name  = 

// Address modifiers used to access the module:

Const(initamod) CVMUSBReadoutList::a32UserData;
Const(readamod) CVMUSBReadoutList::a32UserBlock;

// Register map (offsets in bytes) for the V785
//   Not an exhaustive list as the various test registers are omitted.
//

Const(eventBuffer)      0;
Const(firmware)    0x1000;
Const(GEO)         0x1002;
Const(McastAddr)   0x1004;
Const(BSet1)       0x1006;
Const(BClear1)     0x1008;
Const(IPL)         0x100a;
Const(Vector)      0x100c;
Const(Status1)    0x100e;
Const(Control1)    0x1010;
Const(AderHigh)    0x1012;
Const(AderLow)     0x1014;
Const(RESET)       0x1018;	// Better to use Bset according to manual.
Const(McastCtl)    0x101a;
Const(EventTrig)   0x1020;
Const(Status2)     0x1022;
Const(EventCountL) 0x1024;
Const(EventCountH) 0x1026;
Const(IncrementEC) 0x1028;
Const(IncrementO)  0x102a;
Const(LoadTest)    0x102c;
Const(FastClear)   0x102e;
Const(BSet2)       0x1032;
Const(BClear2)     0x1034;
Const(LogicalCrate) 0x103c;
Const(ECountReset) 0x1040;
Const(Thresholds)   0x1080;	// Continues through 10bf, 32 D16 words.

// Prom locations: We'll want to be sure the module really is a V785:
// Our mnemonics are a bit different from the CAEN ones:

Const(BoardIDHSB) 0x8036;	// Highest significant byte of the id
Const(BoardIDMSB) 0x803a;	// Middle significant byte of the id.
Const(BoardIDLSB) 0x803e;	// Lowest significant byte of the id.

// One last set of constants. This is the maximum number of longwords  a
// module might store:

Const(MaxEventSize)  34;           // Longwords in the largest event.
Const(MEBDepth)      32;	   // Max events the adc can buffer.
Const(MaxLongwordsBuffered) (MaxEventSize*MEBDepth);

/////////////////////////////////////////////////////////////////////
///////// Parameters for validity checkers (e.g. ranges)  ///////////
/////////////////////////////////////////////////////////////////////

// -geo is a 5 bit unsigned field:

static CConfigurableObject::limit GeoLow(0);
static CConfigurableObject::limit GeoHigh(0x1f);
static CConfigurableObject::Limits GeoLimits(GeoLow, GeoHigh);

// -threshold requires 32 integers in the range [0, 0xff]

static CConfigurableObject::limit ThreshLow(0);  
static CConfigurableObject::limit ThreshHi(0xff);
static CConfigurableObject::Limits ThreshLimits(ThreshLow, ThreshHi);
static CConfigurableObject::limit  thresholdSize(32);
static CConfigurableObject::ListSizeConstraint threshListSize = {thresholdSize,
                                                                 thresholdSize};
CConfigurableObject::TypeCheckInfo threshValueOk(CConfigurableObject::isInteger,
						 &ThreshLimits);
static CConfigurableObject::isListParameter threshValidity = 
  {threshListSize,
   threshValueOk};

							      
// -ipl is in the range 0-7

static CConfigurableObject::limit iplLow(0);
static CConfigurableObject::limit iplHigh(7);
static CConfigurableObject::Limits iplRange(iplLow, iplHigh);

// - vector is in the range 0 0xff

static CConfigurableObject::limit vectorLow(0);
static CConfigurableObject::limit vectorHigh(0xff);
static CConfigurableObject::Limits vectorRange(vectorLow, vectorHigh);

// -highwater is in the range 0 through MEBDepth.

static CConfigurableObject::limit hwlow(0);
static CConfigurableObject::limit hwhigh(MEBDepth);
static CConfigurableObject::Limits hwRange(hwlow, hwhigh);

// -fastclear is in the range 0 through 0x3ff.

static CConfigurableObject::limit  fcLow(0);
static CConfigurableObject::limit  fcHigh(0x3ff);
static CConfigurableObject::Limits fcRange(fcLow, fcHigh);

//////////////////////////////////////////////////////////////////////
/////////////////// Canonical class/object operations ////////////////
//////////////////////////////////////////////////////////////////////

/*! 

   Construction is largely a no-op.   The configuration pointer
   is set by onAttach which is called by the base class constructor.
   It's nice to have some easy methods for a change.

*/
C785::C785() :
  m_pConfiguration(0)
 {}
C785::C785(const C785& rhs) :
  m_pConfiguration(0)
{
  if (rhs.m_pConfiguration) {
    m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));
  }
}
C785::~C785() {}

C785&
C785::operator=(const C785& rhs) {
  return *this;
}


///////////////////////////////////////////////////////////////////////
//////////////////////// object operations ////////////////////////////
///////////////////////////////////////////////////////////////////////


/*!
   This is called when the configuration object is attached to us.
   what we need to do is create all of the parametrs and set their default
   values  The parameters that have default values are:

\verbatim
   Parameter        Default
   -thresholds      list of 32 0's.
   -smallthresholds false
   -ipl             6 (interrupt 6 is expected by the rdo thread.
   -vector          0x80
   -highwater       MEBDepth*3/4  (3/4 full event buffer).
   -fastclear       0
   -supressrange    true
\endverbatim

   All others have no default values.   If, during initialization one of those
   is fetched an string exception of the sort:
   "Required configuration parameter $parameter_name was not supplied."

   \param configuration : CReadoutModule&
       The Readout module which will hold our configuration.
*/

void
C785::onAttach(CReadoutModule& configuration)
{
  m_pConfiguration = &configuration;

  // Create the parameters... setting defaults as neeed.

  m_pConfiguration->addParameter("-base", CConfigurableObject::isInteger,
				 NULL, "0");
  m_pConfiguration->addParameter("-geo", CConfigurableObject::isInteger,
				 &GeoLimits);
  m_pConfiguration->addParameter("-thresholds", CConfigurableObject::isIntList,
				 &threshValidity,
		 "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0");
  m_pConfiguration->addParameter("-smallthresholds", CConfigurableObject::isBool,
				 NULL, "false");
  m_pConfiguration->addParameter("-ipl", CConfigurableObject::isInteger,
				 &iplRange, "6");
  m_pConfiguration->addParameter("-vector", CConfigurableObject::isInteger,
				 &vectorRange, "0x80");

  // The high water mark default is computed for cleanliness.

  char hwdefault[100];
  sprintf(hwdefault, "%d", MEBDepth*3/4);
  m_pConfiguration->addParameter("-highwater", CConfigurableObject::isInteger,
				 &hwRange, string(hwdefault));

  m_pConfiguration->addParameter("-fastclear", CConfigurableObject::isInteger,
				 &fcRange, "0");
  m_pConfiguration->addParameter("-supressrange", CConfigurableObject::isBool,
				 NULL, "true");
}
/*!
    Initialize the module prior to data taking. We get the parameters
    from the configuration and
    - Ensure this module is a V785 by reading the module type.
    - Program the module according to the configuration.
    - Program the module to BERR when empty of events.

    Note that it will be up to the caller to ensure that our vector is used
    to trigger the list that will eventually be added to by addReadoutList.
    This is probably best done by having the vector/ipl set programmatically
    rather than by the user through his/her configuration file.

    \param controller : CVMUSB&
       Reference to a VM-USB controller object that will be used to program
       the module.  For the most part we'll do this programming with single
       shot operations, rather than a list as this is not performance critical.

    \throw string
        If the module is not a V785 as determined by reading it's module id
	from its configuration prom, then we will throw this exception
	the string will be of the form:
	"Module config-name at base base-address is not a V785."
*/
void
C785::Initialize(CVMUSB& controller)
{
  uint32_t base = getIntegerParameter("-base");	// Get the base address.
  int      type = getModuleType(controller, base);

  if ((type != 785)  && (type != 775)) { // 775 for testing!!.
    char message[100];
    string name = m_pConfiguration->getName();
    sprintf(message, "Module %s at base 0x%x is not a 785",
	    name.c_str(), base);
    throw string(message);
  }
  // Ok now we've established that we have a V785 we can program it.
  // programming access use the address modifier initamod:
  
  // first reset it to a nice power up configuration... this is done by
  // flipping the the SOFT.RESET bit in the bit set 1 register.
  // sett fig 4.12 in the V785 manual.

  controller.vmeWrite16(base+BSet1, initamod, 0x80);
  controller.vmeWrite16(base+BClear1, initamod, 0x80);

  // Set the GEOgraphical address of the module.

  uint16_t geo = getIntegerParameter("-geo");
  controller.vmeWrite16(base+GEO, initamod, geo);

  // Set the thresholds for the module:
  // As well as the meaning of the thresholds.

  vector<uint16_t> thresholds;
  getThresholds(thresholds);
  for (int chan = 0; chan < 32; chan++) {
    uint16_t threshold = thresholds[chan];
    controller.vmeWrite16(base+Thresholds+chan*sizeof(uint16_t), initamod, 
			  threshold);
  }
  if (getBoolParameter("-smallthresholds")) {
    controller.vmeWrite16(base+BSet2, initamod, 0x100);	// small thresholds.
  } 
  else {
    controller.vmeWrite16(base+BClear2, initamod, 0x100); // large thresholds.
  }
  
  // Set the interrupt characteristics of the module:

  uint16_t ipl     = getIntegerParameter("-ipl");
  uint16_t IVector = getIntegerParameter("-vector");
  uint16_t WhenIRQ = getIntegerParameter("-highwater");

  controller.vmeWrite16(base+IPL,       initamod, ipl);
  controller.vmeWrite16(base+Vector,    initamod, IVector);
  controller.vmeWrite16(base+EventTrig, initamod, WhenIRQ);

  // Set the fast clear window:

  uint16_t fclearValue = getIntegerParameter("-fastclear");
  controller.vmeWrite16(base+FastClear, initamod, fclearValue);

  // Set the supression.

  bool supressed = getBoolParameter("-supressrange");
  if (supressed) {
    controller.vmeWrite16(base+BSet2, initamod, 0x38);
  }
  else {
    controller.vmeWrite16(base+BClear2, initamod, 0x38);
  }

  // Finally, ensure that at the end of a readout we'll get a BERR, rather than
  // we'll just write control regiseter 1 the way we want it:
  // Bits:      CAEN Mnemonic   Meaning:
  // 4          BLNKKEND  (off)  In block transfer send data until empty not just 1 evt
  // 0x10       PROG RESET(off) Front panel reset only clears MEB.
  // 0x20       BERR ENABLE(on) BERR after end of data rather than inv data.
  // 0x40       ALIGN64 (off)   Alighn output to 64 bit boundaries.
  // All this makes the mask:
  //    0x24
  //
  controller.vmeWrite16(base+Control1, initamod, 0x20);


 
}
/*!
   Add the module read to a readout list.  In this case we will
   add MaxLongwordsBuffered*2 reads in fifo mode to the list.
   The *2 is because we're still acquiring data when the read is
   happening...and we really want to drain the device
   \param list : CVMUSBReadoutList&
       Reference to the readout list to append to.

*/
void
C785::addReadoutList(CVMUSBReadoutList& list)
{
  list.addFifoRead32(static_cast<uint32_t>(getIntegerParameter("-base")),
		     readamod, static_cast<size_t>(MaxLongwordsBuffered*2));
}

/*!
   Create a dynamically allocated copy of *this.
*/
CReadoutHardware*
C785::clone() const
{
  return new C785(*this);
}

/////////////////////////////////////////////////////////////////////
//////////////////// Private utility functions //////////////////////
/////////////////////////////////////////////////////////////////////

// Return the value of an integer parameter.
// Parameters:
//    std::string name - name of the parameter.
// Returns:
//    value
// Throws a string exception (from cget) if there is no such parameter.
// caller is responsible for ensuring the parameter is an int.
//
unsigned int
C785::getIntegerParameter(string name)
{
  string sValue =  m_pConfiguration->cget(name);
  unsigned int    value  = strtoul(sValue.c_str(), NULL, 0);

  return value;
}
//  Return the value of a bool parameter.
// Parameters:
//    std::string name - name of the parameter.
// Returns:
//   true if the value is one of: true, yes, 1, on, enabled.
bool
C785::getBoolParameter(string name)
{
  string sValue = m_pConfiguration->cget(name);
  set<string> trueValues;
  trueValues.insert("true");
  trueValues.insert("yes");
  trueValues.insert("yes");
  trueValues.insert("1");
  trueValues.insert("on");
  trueValues.insert("enabled");

  return trueValues.count(sValue) != 0;
}
// Get the values of the thresholds  The thresholds should be a list
// of integer values.. which we will stuff into a vector.
// Parameters:
//    vector<uint16_t>& thresholds  - a vector to which the thresholds will be appended.
// 
void
C785::getThresholds(vector<uint16_t>& thresholds)
{
  int    argc;
  const char **argv;
  string sValue = m_pConfiguration->cget("-thresholds");
  Tcl_SplitList(NULL, sValue.c_str(), &argc, &argv);

  assert(argc == 32);		// Validator should have done this.

  for(int i =0; i < 32; i++) {
    thresholds.push_back(static_cast<uint16_t>(strtol(argv[i], NULL, 0)));
  }
  

  Tcl_Free((char*)argv);

 
}
// Get the module type.  This is done by reading the three module type
// bytes from the module's configuratino prom.
//
// Parameters:
//    CVMUSB&  controller - USB controller to use to access VME bus.
//    uint32_t base       - Base address of module.
//
int
C785::getModuleType(CVMUSB& controller, uint32_t base)
{
  uint16_t basel, basem, baseh;

 controller.vmeRead16(base + BoardIDLSB, initamod, &basel);
 basel &= 0xff;			// only the bottom byte matters.

 controller.vmeRead16(base + BoardIDMSB, initamod, &basem);
 basem &= 0xff;

 controller.vmeRead16(base + BoardIDHSB, initamod, &baseh);
 baseh &= 0xff;

 return (basel | (basem << 8) | (baseh << 16));

}
