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
	
     @file CPH7106Latch.h
     @brief Define the event readout support class for the PH7106 
            discriminator/latch.
*/

#ifndef CPH7106LATCH_H
#define CPH7106LATCH_H

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

class CReadoutModule;
class CCCUSB;
class CCCUSBReadoutList;


/**
 * @class CPH7106Latch
 *
 *   Provides readout support for the PH 7106 discriminator/latch.  
 *   Note that while there is slow control support for this module
 *   use of the slow control support simultaneously with readout support
 *   on the same hardware module is not supported.  This is because
 *   this module will read/refresh its configuration at the start of a
 *   run regardless of what the slow control GUI might have done.
 *
 *   OPTIONS:
 *
 *    - -slot    - Slot in the crate the module is located in.
 *    - -mask    - Bits to set in the mask regiseter.  Bits that are one enable a channel.
 *    - -threshold - Threshold value (0 - 1023).
 *
 *  @note: to operate the module must have its local/CAMAC switch in CAMAC and the module
 *         will be put in remote mode.  If the switch is in LOCAL mode the system won't
 *         allow the run to start and an error message will be emitted.
 */
class CPH7106Latch : public CReadoutHardware
{
private:

  // Canonicals:

public:
  CPH7106Latch();
  CPH7106Latch(const CPH7106Latch& rhs);
  virtual ~CPH7106Latch();

private:
  int operator==(const CPH7106Latch& rhs) const;
  int operator!=(const CPH7106Latch& rhs) const;

  // The interface CReadoutHardware derived objects must 
  // implement.


public:
  virtual void onAttach(CReadoutModule& configuration);
  virtual void Initialize(CCCUSB& controller);
  virtual void addReadoutList(CCCUSBReadoutList& list);
  virtual CReadoutHardware* clone() const;
};

#endif
