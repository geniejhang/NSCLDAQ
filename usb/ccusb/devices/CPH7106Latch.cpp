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
	
     @file CPH7106Latch.cpp
     @brief implement the event readout support class for the PH7106 
            discriminator/latch.
*/
#include <config.h>

#include "CPH7106Latch.h"

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


/*---------------------------------------------------------------------------------*
*  Local constants:
*/

static CConfigurableObject::limit Zero(0);
static CConfigurableObject::limit K(1023);
static CConfigurableObject::limit FullRange(0xffff);
static CConfigurableObject::limit One(1);
static CConfigurableObject::limit lastSlot(23);    // By CAMAC standard.

static CConfigurableObject::Limits thresholdLimits(Zero, K);
static CConfigurableObject::Limits maskLimits(Zero, FullRange);
static CConfigurableObject::Limits slotLimits(One, lastSlot);

/**
 *  constructor
 *     Just set the configuration to null:
*/

CPH7106Latch::CPH7106Latch()
{
  m_pConfiguration = 0;		// set at OnAttach.
}

/**
   destructor is a no-op.
*/
CPH7106Latch::~CPH7106Latch()
{
}

/**
   Copy construction will copy the configuration and its valuds if they have been produced
   yet by the rhs

   @param rhs - the object being copy constructed into *this.
*/
CPH7106Latch::CPH7106Latch(const CPH7106Latch& rhs)
{
  m_pConfiguration = 0;
  if(rhs.m_pConfiguration) {
    m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));
  }
}


/*------------------------------------------------------------------------------
 * Implementing the CReadoutHardware Interface.
 */

/**
 *   onAttach
 *
 * This method is called when the module is attached to its configuration.
 * We define all of the configuration options.  We define the following
 * options
 *
 * - -slot - Slot the module is in.
 * - -mask - Mask register valu7e.
 * - -threshold - threshold register value [0,1024).
 */
void
CPH7106Latch::onAttach(CReadoutModule& configuration)
{
  m_pConfiguration = &configuration;
  configuration.addParameter("-slot", CConfigurableObject::isInteger, &slotLimits, "0");
  configuration.addParameter("-mask", CConfigurableObject::isInteger, &maskLimits, "0xffff");
  configuration.addParameter("-threshold", CConfigurableObject::isInteger, &thresholdLimits, "0");


}
/**
 *   Initialize:
 *  
 *  Called as a run is starting to initialize the device.
 *  - Try to put the module into remote mode 
 *  - Get really pissed off if we can't get the module into remote mode.
 *  - Initialize the mask and threshold registers.
 *
 * @param controller - CCUSB controller reference.
 */
void
CPH7106Latch::Initialize(CCCUSB& controller)
{
  int slot = getIntegerParameter("-slot");
  if (!slot) {
    throw "A PH7106 discriminator/latch has not had its -slot configured";
  }

  // Turn module into remote and complain if failed:

  checkedControl(controller, slot, 0, 26, "Setting remote on",  true);
  uint16_t qx;
  int status = controller.simpleControl(slot, 0, 27, qx);
  if (status != 0) {
    throw "CAMAC operation to check PH7106 is in remote mode failed";
  }
  if ((qx & CCCUSB::Q) == 0) {
    throw "Could not put PH7106 into remote mode. Check the CAMAC/Local switch";
  }

  // At this point we know we can control the module.

  
  checkedWrite16(controller, slot, 0, 16, getIntegerParameter("-mask"), "Mask register write failed", true);
  checkedWrite16(controller, slot, 0, 17, getIntegerParameter("-threshold"), "Threshold write failed", true);

}

/**
 * addReadoutList 
 *   Add operations to the readout list for the device.  In this case we want to read the
 *   'internal data latch' (F0@A1).
 *
 *  @param list the list (stack) that's being built.
 */
void
CPH7106Latch::addReadoutList(CCCUSBReadoutList& list)
{
  int slot= getIntegerParameter("-slot");
  list.addRead16(slot, 1, 0);
  
}
    
/**
 * clone
 *    Produce a clone of this and return it.
 *
 * @return CPH7106Latch*
 */
CReadoutHardware* 
CPH7106Latch::clone() const
{
  return new CPH7106Latch(*this);
}
