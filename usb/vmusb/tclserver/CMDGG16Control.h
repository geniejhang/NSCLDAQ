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

#ifndef __CMDGG16Control_H
#define __CMDGG16Control_H


#ifndef __CCONTROLHARDWARE_H
#include "CControlHardware.h"
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif


#include <CControlModule.h>
#include <CWienerMDGG16.h>
class CVMUSB;


/*!

  Configuration parameters include:

  -base         - Base address of the module.

*/

struct CMDGG16ControlState 
{
  uint32_t or_a;
  uint32_t or_b;
  uint32_t or_c;
  uint32_t or_d;
};

class ConfigFileReader {
  public:
    CMDGG16ControlState parse(std::string file);
};

class CMDGG16Control : public CControlHardware
{
  private:
    CWienerMDGG16 m_dev;

public:
  // canonicals:

  CMDGG16Control();
  CMDGG16Control(const CMDGG16Control& rhs);
  virtual ~CMDGG16Control();

  CMDGG16Control& operator=(const CMDGG16Control& rhs);
  int operator==(const CMDGG16Control& rhs) const;
  int operator!=(const CMDGG16Control& rhs) const;


  // virtual overrides:

public:
  virtual void onAttach(CControlModule& configuration);  //!< Create config.
  virtual void Initialize(CVMUSB& vme);	                 //!< init module after configuration is done.
  virtual std::string Update(CVMUSB& vme);               //!< Update module.
  virtual std::string Set(CVMUSB& vme, 
			                     std::string parameter, 
			                     std::string value);            //!< Set parameter value
  virtual std::string Get(CVMUSB& vme, 
			                    std::string parameter);        //!< Get parameter value.
  virtual std::unique_ptr<CControlHardware> clone() const;	     //!< Virtual

  // utilities:
  
private:

  // Confiuration parameter getters:
  uint32_t base();
  void configureECLOutputs(CVMUSBReadoutList& list);
  void configureORMasks(CVMUSBReadoutList& list);
  void configureFromConfigFile(CVMUSBReadoutList& ctlr);


};




#endif
