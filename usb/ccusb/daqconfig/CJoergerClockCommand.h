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
	
     @file CJoergerClockCommand.h
     @brief Define the command that creates/configures CJoergerClock instances.
*/

#ifndef CJOERGERCLOCKCOMMAND_H
#define CJOERGERCLOCKCOMMAND_H



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
 * @class CJoergerClockCommand.h
 *   Defines the class that implements the joergerclock command.   This is the usual
 *   ensemble that provides create, config, and cget subcommands.
 *
 */
class CJoergerClockCommand : public CTCLObjectProcessor
{
private:
  CConfiguration& m_Config;             // Global configuration for all DAQ devices created.

public:
  CJoergerClockCommand(CTCLInterpreter& interp, CConfiguration& config, const char* command="joergerclock");
  virtual ~CJoergerClockCommand();

public:
  int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

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
