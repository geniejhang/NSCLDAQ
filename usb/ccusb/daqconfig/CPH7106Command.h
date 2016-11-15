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
	
     @file CPH7106Command.h
     @brief Define the command class that creates CPH7106Latch driver instances.
            discriminator/latch.
*/

#ifndef CPH7106COMMAND_H
#define CPH7106COMMAND_H

#ifndef __TCLOBJECTPROCESSOR_H
#include <TCLObjectProcessor.h>
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif



class CTCLInterpreter;
class CTCLObject;
class CConfiguration;
class CReadoutModule;

/**
 * @class CPH7106Command
 *    This class is responsible for creating/configuring and describing
 *    CPH7106Latch instances from configuration file commands.  The device
 *    supports the following forms:
 *
 * \verbatin
 *    ph7106 create name option-value-pairs
 *    ph7106 config name option-value-pairs
 *    ph7106 cget name
 *
 *\endverbatim.
 */

class CPH7106Command : public CTCLObjectProcessor
{
private:
  CConfiguration& m_Config;             // Global configuration containing all DAQ devices.
public:
  CPH7106Command(CTCLInterpreter& interp, CConfiguration& config, 
		 std::string commandName="ph7016");
  virtual ~CPH7106Command();

private:
  CPH7106Command(const CPH7106Command& rhs);
  CPH7106Command& operator=(const CPH7106Command& rhs);
  int operator==(const CPH7106Command& rhs) const;
  int operator!=(const CPH7106Command& rhs) const;

public:
  virtual int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
  CConfiguration* getConfiguration() {return &m_Config; }

private:
  void create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
  void config(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
  void cget(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

  void    configure(CTCLInterpreter&         interp,
		   CReadoutModule*          pModule,
		   std::vector<CTCLObject>& config,
		   int                      firstPair = 3);
  std::string configMessage(std::string base,
			    std::string key,
			    std::string value,
			    std::string errorMessage);


};

#endif
