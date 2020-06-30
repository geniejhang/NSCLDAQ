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

#ifndef CDriverTemplate_H
#define CDriverTemplate_h

#include "CControlHardware.h"
#include <string>
#include <stdint.h>

#include <memory>

#include <CControlModule.h>
class CCCUSB;


/*!
  Sample device driver template.  This template shows how to build a
device driver for a slow controls devicde.

*/

class CDriverTemplate : public CControlHardware
{

public:
  // canonicals:

  CDriverTemplate();
  CDriverTemplate(const CDriverTemplate& rhs);
  virtual ~CDriverTemplate();

  CDriverTemplate& operator=(const CDriverTemplate& rhs);
  int operator==(const CDriverTemplate& rhs) const;
  int operator!=(const CDriverTemplate& rhs) const;


  // virtual overrides:

public:
  virtual void onAttach(CControlModule& configuration);  //!< Create config.
  virtual void Initialize(CCCUSB& vme);	                 //!< init module after configuration is done.
  virtual std::string Update(CCCUSB& vme);               //!< Update module.
  virtual std::string Set(CCCUSB& vme, 
			  std::string parameter, 
			  std::string value);            //!< Set parameter value
  virtual std::string Get(CCCUSB& vme, 
			  std::string parameter);        //!< Get parameter value.
  virtual CControlHardware* clone() const;	     //!< Virtual

  // utilities:
private:
  uint32_t getSlot();
};




#endif
