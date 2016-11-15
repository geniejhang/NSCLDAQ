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
	
     @file CJoergerClock.h
     @brief Define driver for the Joerger clock module.
*/

#ifndef CJOERGERCLOCK_H
#define CJOERGERCLOCK_H

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
 *  @class CJoergerClock
 *    Clock module based on the sketchy info I have from Cary? at UTK.edu.
 *    The module has 4 outputs and the clock rate is programmable for each
 *    output, by writing F17@An for each output.  It's not so clear what
 *    the values mean, therefore we'll just expose the raw values.
 *    Configuration parameters:
 *    - -slot - crate slot number.
 *    - -chan0 - Value to program in chan0.
 *    - -chan1 - Value to program in chan1.
 *    - -chan2 - Value to program in chan2.
 *    - -chan3 - Value to program in chan3.
 */
class CJoergerClock : public CReadoutHardware
{
public:
  CJoergerClock();
  CJoergerClock(const CJoergerClock& rhs);
  virtual ~CJoergerClock();
private:
  int operator==(const CJoergerClock& rhs) const;
  int operator!=(const CJoergerClock& rhs) const;

  // The interface CReadoutHardware derived objects must 
  // implement.


public:
  virtual void onAttach(CReadoutModule& configuration);
  virtual void Initialize(CCCUSB& controller);
  virtual void addReadoutList(CCCUSBReadoutList& list);
  virtual CReadoutHardware* clone() const;
};

#endif
