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
#include "CMDGG16Control.h"
#include "WienerMDGG16Registers.h"
#include "CWienerMDGG16.h"
#include "CControlModule.h"
#include "CVMUSB.h"
#include "CVMUSBReadoutList.h"	// for the AM codes.

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace std;

static const char* modeEnum [] = { "or" , 0 };

/*!
   construct the beast.. The shadow registers will all get set to zero
*/
CMDGG16Control::CMDGG16Control() :
  CControlHardware(), m_dev()
{
}

/*!

  Copy construction:
*/
CMDGG16Control::CMDGG16Control(const CMDGG16Control& rhs) :
  CControlHardware(rhs), m_dev(rhs.m_dev)
{
}
/*!
  While destruction could leak I seem to recall problems if I destroy
  the configuration..
*/
CMDGG16Control::~CMDGG16Control()
{
}

/*!
  Assignment is a clone:
*/
CMDGG16Control&
CMDGG16Control::operator=(const CMDGG16Control& rhs)
{
  if(this != &rhs) {
    m_dev = rhs.m_dev;
    CControlHardware::operator=(rhs);
  }
  return *this;
}

/*!
  Same configuration implies equality:
*/
int 
CMDGG16Control::operator==(const CMDGG16Control& rhs) const
{
  return CControlHardware::operator==(rhs);
}
/*
   != is the logical inverse of ==
*/
int
CMDGG16Control::operator!=(const CMDGG16Control& rhs) const
{
  return !(*this == rhs);
}

///////////////////////////////////////////////////////////////////////////

/*!
   Attachment : We need to define our parameters which are:
   -base   - unlimited integer.

 \param configure - Encapsulates the configuration for this module.

*/
void
CMDGG16Control::onAttach(CControlModule& configuration)
{
  m_pConfig = &configuration;
  configuration.addParameter("-base", CConfigurableObject::isInteger, NULL, 
                             string("0"));

  configuration.addEnumParameter("-mode", modeEnum, "or");
  configuration.addIntegerParameter("-or_a",0,255,255);
  configuration.addIntegerParameter("-or_b",0,255,255);
  configuration.addIntegerParameter("-or_c",0,255,255);
  configuration.addIntegerParameter("-or_d",0,255,255);
}
////////////////////////////////////////////////////////////////////////////
/*!
    Initialize the module bringing it to a known state.

   \param CVMUSB& vme
     Controller that hooks us to the VM-USB.
*/
void
CMDGG16Control::Initialize(CVMUSB& vme)
{
   m_dev.setBase(base());

   unique_ptr<CVMUSBReadoutList> pList(vme.createReadoutList());

   std::cout << std::hex;
   std::cout << "Firmware : " << m_dev.readFirmware(vme) << std::endl;
   std::cout << "Global : " << m_dev.readGlobal(vme) << std::endl;
   std::cout << std::dec;

   configureECLOutputs(*pList);   
   configureORMasks(*pList);   

   size_t nRead=0;
   uint32_t buf[8];
   int status = vme.executeList(*pList, buf, sizeof(buf), &nRead);

   if (status < 0) {
     std::stringstream errmsg;
     errmsg << "CMDGG16Control::Initialize() failed while executing list ";
     errmsg << "with status = " << status;
     throw errmsg.str();
   }
}
//
//void CMDGG16Control::readState(CVMUSB& vme) 
//{
//  
//   unique_ptr<CVMUSBReadoutList> pList(vme.createReadoutList());
//   m_dev.addReadFirmware(*pList);
//   m_dev.addReadGlobal(*pList);
//
//}

/*!
  Update the device from the shadow configuration.

  \param vme  - VME controller object.
  \return string
  \retval "OK"  - Everything worked.
  \retval "ERROR -...."  Describes why things did not work.

*/
string
CMDGG16Control::Update(CVMUSB& vme)
{

  return string("OK");
}
///////////////////////////////////////////////////////////////////////////////////
/*!
   Set a value in the device.


  \param vme        - The VM-USB controller object.
  \param parameter  - Name of the parameter to set.
  \param value      - Value to which the parameter will be set.

   \return string
   \retval "OK"    - The parameter was set (the shadow value will also be set).
   \retval "ERROR -..."  An error was detected, the remaining string describes the error.
*/
string
CMDGG16Control::Set(CVMUSB& vme, string parameter, string value)
{
  // to ensure that we use the most recent base address.
  m_dev.setBase(base());

  // convert string to unsigned long integer (auto detects base)
  uint32_t val = std::stoul(value,0,0);

  unique_ptr<CVMUSBReadoutList> pList( vme.createReadoutList() );

  if (parameter == "or_ab") {
    m_dev.addWriteLogicalORMaskAB(*pList, val);
  } else if (parameter == "or_cd") {
    m_dev.addWriteLogicalORMaskCD(*pList, val);
  } else {
    std::string errmsg("CMDGG16Control::Set() called with invalid parameter ");
    errmsg += "name (";
    errmsg += parameter;
    errmsg += ").";
    throw errmsg;
  }


  size_t nRead=0;
  uint32_t buf[8];
  int status = vme.executeList(*pList, buf, sizeof(buf), &nRead);
  if (status<0) {
    std::stringstream errmsg;
    errmsg << "CMDGG16Control::Set() failure while executing list. ";
    errmsg << "Status returned is " << status;
    throw errmsg.str();
  }

  std::string result;
  size_t nLongs = nRead/sizeof(uint32_t);
  if (nLongs>0) {
    for (size_t i=0; i<nLongs; ++i) {
      result += (std::to_string(buf[i]) + " ");
    } 
  }
  return result;
}
/////////////////////////////////////////////////////////////////////////////////////////
/*!
   Get a value from the device.
 
  \return string
  \retval stringified-integer   - Things went ok.
  \retval "ERROR - ..."         - An error occured described by the remainder of the string.
*/
string
CMDGG16Control::Get(CVMUSB& vme, string parameter)
{
  m_dev.setBase(base());

  unique_ptr<CVMUSBReadoutList> pList(vme.createReadoutList());
  
  string result = "OK";
  if (parameter=="or_ab") {
    m_dev.addReadLogicalORMaskAB(*pList);
  } else if (parameter=="or_cd") {
    m_dev.addReadLogicalORMaskCD(*pList);
  } else {
    std::string errmsg("CMDGG16Control::Get() called with invalid parameter ");
    errmsg += "name (";
    errmsg += parameter;
    errmsg += ").";
    throw errmsg;
  } 

  // execute list
  size_t nRead=0;
  uint32_t buf[8];
  int status = vme.executeList(*pList, buf, sizeof(buf), &nRead);
  if (status<0) {
    std::stringstream errmsg;
    errmsg << "CMDGG16Control::Set() failure while executing list. ";
    errmsg << "Status returned is " << status;
    throw errmsg.str();
  }

  // convert returned data into a list
  size_t nLongs = nRead/sizeof(uint32_t);
  if (nLongs>0) {
    // reset the value
    result = "";
    for (size_t i=0; i<nLongs; ++i) {
      result += std::to_string(buf[i]);
      if (i<(nLongs-1)) {
       result += " ";
      } 
    }
  }
  return result;

}

///////////////////////////////////////////////////////////////////////////////////////
/*!
  At present, cloning is a no-op.
*/
std::unique_ptr<CControlHardware>
CMDGG16Control::clone() const
{
  return std::unique_ptr<CControlHardware>(new CMDGG16Control(*this));
}

//////////////////////////////////////////////////////////////////////////////////
///////////////// private utilities //////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

/*
   Return the base address of the device:

*/
uint32_t 
CMDGG16Control::base()
{
  if (m_pConfig) {
    string strBase = m_pConfig->cget("-base");
    unsigned long base;
    base = strtoul(strBase.c_str(), NULL, 0);
    return static_cast<uint32_t>(base);
  }
  else {
    return static_cast<uint32_t>(0);
  }
}

void CMDGG16Control::configureECLOutputs(CVMUSBReadoutList& list)
{
  if (m_pConfig) {
    // there is currently only 1 option and that is to have all
    // of the logical or outputs provided. 
    using namespace ::WienerMDGG16::ECL_Output;
    uint32_t outputBits = (Logical_OR << ECL9_Offset);
    outputBits |= (Logical_OR << ECL10_Offset);
    outputBits |= (Logical_OR << ECL11_Offset);
    outputBits |= (Logical_OR << ECL12_Offset);
    outputBits |= (Logical_OR << ECL13_Offset);
    outputBits |= (Logical_OR << ECL14_Offset);
    outputBits |= (Logical_OR << ECL15_Offset);
    outputBits |= (Logical_OR << ECL16_Offset);
    m_dev.addWriteECLOutput(list, outputBits);
  }
}

void CMDGG16Control::configureORMasks(CVMUSBReadoutList& list)
{
  if (m_pConfig) {
    uint32_t or_a = m_pConfig->getUnsignedParameter("-or_a"); 
    uint32_t or_b = m_pConfig->getUnsignedParameter("-or_b"); 
    uint32_t or_c = m_pConfig->getUnsignedParameter("-or_c"); 
    uint32_t or_d = m_pConfig->getUnsignedParameter("-or_d"); 

    using namespace ::WienerMDGG16::Logical_OR;

    uint32_t or_ab = (or_b<<B_Offset)|(or_a<<A_Offset);
    uint32_t or_cd = (or_d<<D_Offset)|(or_c<<C_Offset);
    m_dev.addWriteLogicalORMaskAB(list, or_ab); 
    m_dev.addWriteLogicalORMaskCD(list, or_cd);
  }

}
