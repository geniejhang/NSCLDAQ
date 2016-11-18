/*========================================================================================
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
	
     @file CJoergerClockCommand.cpp
     @brief Implement the command that creates/configures CJoergerClock instances.
*/

#include <config.h>
#include "CJoergerClockCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <CConfiguration.h>
#include <CReadoutModule.h>
#include <CJoergerClock.h>
#include <Exception.h>
#include <stdexcept>


/*-------------------------------------------------------------------------------
 *  Canonical methods
 */


/**
 * constructor
 *  @param interp - references the interpreter on which the command will be registered.
 *  @param config - Global module configuration object reference (definitions are stored here).
 *  @param command - The command name string (base command of the ensemble).
 */

CJoergerClockCommand::CJoergerClockCommand(CTCLInterpreter& interp, CConfiguration& config, const char* command) :
  CTCLObjectProcessor(interp, command, true),
  m_Config(config)
{}
/**
 * destructor
 */
CJoergerClockCommand::~CJoergerClockCommand() {}

/**
 * operator()
 *    Called when the base command of the ensemble is invoked.  We're going to:
 *   - bind all the command word objects to an interpreter.
 *   - Setup error handling to be done via exception catching.
 *   - Dispatch to the appropriate subcommand processor
 *
 * @param interp  - interpreter that is running the command.
 * @param objv    - words that make up the command.
 * @return int    - TCL_OK - success,TCL_ERROR - failure.
 */
int
CJoergerClockCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  bindAll(interp, objv);
  try {
    requireAtLeast(objv, 2, "Command ensemble requires at least a subcommand");
    std::string subcommand = objv[1];
    if (subcommand == "create") {
      create(interp, objv);
    } else if (subcommand == "config") {
      config(interp, objv);
    } else if (subcommand == "cget") {
      cget(interp, objv);
    } else {
      throw std::invalid_argument("Invalid subcommand");
    }
  }
  catch (std::exception& e) {
    interp.setResult(e.what());
    return TCL_ERROR;
  }
  catch (CException& e) {
    interp.setResult(e.ReasonText());
    return TCL_ERROR;
  }
  catch (std::string msg) {
    interp.setResult(msg);
    return TCL_ERROR;
  }
  catch (const char* msg) {
    interp.setResult(msg);
      return TCL_ERROR;
  }
  catch (...) {
    interp.setResult("Unanticipated C++ exception type caught");
    return TCL_ERROR;
  }
  return TCL_OK;
}
/**
 * create
 *   Process the create subcommand -create  a new module.
 *   We need an odd number of command words and at least 3:
 *   command, subcommand, new module name.
 *   Any additional parameters are considered to be module option/value pairs.
 *
 * @param interp - interpreter running the command.
 * @param objv   - Command words.
 */
void
CJoergerClockCommand::create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  requireAtLeast(objv, 3, "create subcommand requires at least a module name");
  if ((objv.size() & 1) == 0) {
    throw std::logic_error("The number of parameters for the create sub command must be odd");
  }

  std::string name = objv[2];

  // It is an error for the module name to be a duplicate:

  CReadoutModule* pModule = m_Config.findAdc(name);
  if (pModule) {
    throw std::invalid_argument("create subcommand - duplicate name");
  }

  // Create the new module and attach its configuration:

  CJoergerClock* pDevice = new CJoergerClock();
  pModule                = new CReadoutModule(name, *pDevice);
  try {
    configure(interp, pModule, objv);
    m_Config.addAdc(pModule);
    interp.setResult(name);
  }
  catch (...) {
    delete pModule;
    delete pDevice;
    throw;
  }
    
}
/**
 *  config
 *    Configure a module.
 *    We need an odd number of command words.. and at least 5 (one configuration option/value
 *    pair after the module name.
 *    - Find the module
 *    - COnfigure it.
 *  
 * @param interp - the interpreter executing this command.
 * @param objv   - The command words.
 */
void
CJoergerClockCommand::config(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  requireAtLeast(objv, 5, "config requires at least one option/value pair after the module name");
  if ((objv.size() & 1) == 0) {
    throw std::invalid_argument("config requires an odd number of command line words");
  }

  std::string name = objv[2];
  CReadoutModule* pModule = m_Config.findAdc(name);
  if (! pModule) {
    throw std::invalid_argument("config -module name not found");
  }

  configure(interp, pModule, objv);
 
}
/**
 * cget
 *    Return the option value pairs of the current configuration as a list of
 *    pairs.  Each pair is an option followed by its value.
 *
 * @param interp  - interpreter running the command
 * @param objv    - The words that make up the command.
 */
void
CJoergerClockCommand::cget(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  requireExactly(objv, 3, "cget takes no additional command line parameters");

 // Get the module's configuration:

  std::string name = objv[2];
  CReadoutModule* pModule = m_Config.findAdc(name);
  if(!pModule) {
    throw std::invalid_argument("cget - module name not found");
  }

  CConfigurableObject::ConfigurationArray config = pModule->cget();

  // Turn the configuration into a list that can be set as the result:

  CTCLObject result;
  result.Bind(interp);

  for (int i = 0; i < config.size(); i++) {
    CTCLObject key;
    key.Bind(interp);
    key = config[i].first;

    CTCLObject value;
    value.Bind(interp);
    value= config[i].second;

    CTCLObject item;
    item.Bind(interp);
    item += key;
    item += value;

    result += item;
  }
  interp.setResult(result);

}
  
 
/**
 * configure
 *   Shared configuration code (between create and config).
 *
 * @param interp -  interpreter running the command.
 * @param pModule - Module being configured.
 * @param config - Command words.
 * @param firstPair - Where configuration starts (defaults to 3).
 */
void
CJoergerClockCommand::configure(
    CTCLInterpreter& interp, CReadoutModule* pModule, std::vector<CTCLObject>& objv, int firstPair
)
{
  std::string baseMessage = "joergerclock - invalid configuration/value pair: ";
  std::string key;
  std::string value;

  try {
    for (int i = firstPair; i < objv.size(); i+=2) {
      key = std::string(objv[i]);
      value = std::string(objv[i+1]);
      pModule->configure(key, value);
    }
  }
  catch (CException& e) {
    throw (configMessage(baseMessage, key, value, e.ReasonText()));

  }
  catch (std::string msg) {
    throw (configMessage(baseMessage, key, value, msg));
  }
  catch (...) {
    throw configMessage(baseMessage, key, value, "Unexpected C++ exception caught");
  }

}
/**
 * configMessage
 *   Produce a confighuration error message.
 *   Produce a configuration message from its elements.
 *
 * @param base - The base error message.
 * @param key  - The keyword that resulted in the error.
 * @param value - The value that resulted in the error.
 * @param errorMessage - The specific error message from the config subsystem.
 */
std::string
CJoergerClockCommand::configMessage(
     std::string base, std::string key, std::string value, std::string errorMessage
)
{
  std::string result = base;
  result += key;
  result += " ";
  result += value;
  result += " ";
  result += errorMessage;

  return result;
}
