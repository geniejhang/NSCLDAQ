/*******************************************************************************
*
* CAEN SpA - System Integration  Division
* Via Vetraia, 11 - 55049 - Viareggio ITALY
* +390594388398 - www.caen.it
*
  
    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	
     @file CJoergerClock.cpp
     @brief Implement the Joerger clock driver.
*/

#include <config.h>
#include "CJoergerClock.h"
#include "CReadoutModule.h"
#include <CCCUSB.h>
#include <CCCUSBReadoutList.h>

#include <tcl.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <string>
#include <set>

#include <iostream>

using namespace std;

static CConfigurableObject::limit One(1);
static CConfigurableObject::limit lastSlot(23);    // By CAMAC standard.
static CConfigurableObject::Limits slotLimits(One, lastSlot);


// FOr now restrict the channel values to from 0-65k

static CConfigurableObject::limit Zero(0);
static CConfigurableObject::limit FullScale(64*1024);
static CConfigurableObject::Limits valueLimits(Zero, FullScale);

/**
 *  Constructor
 *    The superclass has a configuration - set it to zero (I think the
 *    superclass construtor does this but there's a pile of legacy code
 *    in us subclasses that seems to do it too so:
 */
CJoergerClock::CJoergerClock() 
{
  m_pConfiguration = 0;
}
/**
 *  destructor 
 *   A No-OP.
 */
CJoergerClock::~CJoergerClock()
{}

/**
 * copy constructor
 *  If the configuration exists, copy constructd it into our configuration:
 *
 * @param rhs - the template we are copy constructing into *this.
 */
CJoergerClock::CJoergerClock(const CJoergerClock& rhs) 
{
  m_pConfiguration = 0;
  if (rhs.m_pConfiguration) {
    m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));
  }
}

/**
 * onAttach
 *   Called when the configuration is attached to us.  We establish our
 *   configuration parametesr.
 *
 * @param configuration - the configuration object we are supposed to use.
 */
void
CJoergerClock::onAttach(CReadoutModule& configuration)
{
  m_pConfiguration = &configuration;

  configuration.addParameter("-slot", CConfigurableObject::isInteger, &slotLimits, "0");
  configuration.addParameter("-chan0", CConfigurableObject::isInteger, &valueLimits, "0");
  configuration.addParameter("-chan1", CConfigurableObject::isInteger, &valueLimits, "0");
  configuration.addParameter("-chan2", CConfigurableObject::isInteger, &valueLimits, "0");
  configuration.addParameter("-chan3", CConfigurableObject::isInteger, &valueLimits, "0");
}
/**
 * Initialize
 *
 *  Initialize the module prior to 'data taking'
 *  - Ensure the slot's been programmed.
 *  - Program each of the channels.
 *
 * @param controller  - Object that communicates with the CCUSB controller.
 */
void
CJoergerClock::Initialize(CCCUSB& controller)
{
  // Pull the slot out and ensure it's been modified:

  int slot = getIntegerParameter("-slot");
  if (!slot) {
    throw "A Joerger clock module has not had its -slot configured";
  }

  // Now program the channels:

  checkedWrite16(controller, slot, 0, 17, getIntegerParameter("-chan0"), "Joerger clock setting channel 0");
  checkedWrite16(controller, slot, 1, 17, getIntegerParameter("-chan1"), "Joerger clock setting channel 1");
  checkedWrite16(controller, slot, 2, 17, getIntegerParameter("-chan2"), "Joerger clock setting channel 2");
  checkedWrite16(controller, slot, 3, 17, getIntegerParameter("-chan3"), "Joerger clock setting channel 3");

}
/**
 * addReadoutList
 *   We don't read anything, so this is a no-op.
 *
 * @param list - the list we are not adding to do:
 */
void
CJoergerClock::addReadoutList(CCCUSBReadoutList& list) {}

/**
 * clone
 *
 *   Produce a clone of this and return it.
 *
 * @return CJoergerClock*
 */
CReadoutHardware*
CJoergerClock::clone() const
{
  return new CJoergerClock(*this);
}
