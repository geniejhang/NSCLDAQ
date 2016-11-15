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
	
     @file CPH7106Command.cpp
     @brief Implement the command class that creates CPH7106Latch driver instances.
            discriminator/latch.
*/
#include <config.h>
#include "CPH7106Command.h"


#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <CConfiguration.h>
#include <CReadoutModule.h>
#include <CPH7106Latch.h>
#include <Exception.h>
#include <stdexcept>


/*------------------------------------------------------------------------------
 * Canonical methods:
 */

/**
 *  constructor
 *    Construct the command, register it with the interpreter and
 *    retain the device configuration so that we can manipulate/query
 *    it.
 *
 *  @param interp  - the interpreter on which the command will be registered.
 *  @param config  -  The configuration of modules and stacks the config file creates.
 *  @param command - Command names tring.
 */
CPH7106Command::CPH7106Command(CTCLInterpreter& interp, CConfiguration& config, std::string command) :
  CTCLObjectProcessor(interp, command, true),
  m_Config(config)
{}

/**
 * destructor
 *   Unregisters the class from the interpreter on which it is bound.
 */
CPH7106Command::~CPH7106Command() {}

/**
 * operator()
 *    This is called to execute the command in an interpreter.
 *
 *  @param interp - Interpreter that is executing the command.
 *  @param objv   - Command words that make up the entire command.
 *  @return int   - TCL_OK On success, TCL_ERROR on failure.
 *
 * @note The command is an ensemble and this method only does dispatching to the 
 *       proper subcommand method.
 */
int
CPH7106Command::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  bindAll(interp, objv);

  // Exception processing is used to centralize/simplify error handling.
  // methods invoked by us (and operator() for that matter, signal failure
  // via a variety of exception types.

  try {
    requireAtLeast(objv, 2, "The ph7106 command requires at lest a subcommand");

    std::string subcommand = objv[1];
    if (subcommand == "create") {
      create(interp, objv);
    } else if (subcommand == "config") {
      config(interp, objv);
    } else if (subcommand == "cget") {
      cget(interp, objv);
    } else {
      throw std::string("ph7106 invalid subcommand must be create, config, or cget");
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
    interp.setResult("Unexpected C++ exception type");
    return TCL_ERROR;
  }

  return TCL_OK;
}
/**
 * create
 *   Responsible for creation and initial configuration of 
 *   a module object.
 *
 * @param interp - interpreter that is executing the command.
 * @param objv   - the command words.
 */
void
CPH7106Command::create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  requireAtLeast(objv, 3, "ph7106 create command requires at least a module name");

  // There must be an odd number of words as any extra must be config
  // param/value pairs:

  if ((objv.size() & 1) == 0) {
    throw std::string("ph106 create command must have an odd number of command words");
  }
  // Get the requested name and be sure it does not exist yet:

  std::string name = objv[2];
  CReadoutModule* pModule = m_Config.findAdc(name);
  if (pModule) {
    throw std::string("ph7106 create - duplicate module name");
  }
  
  // Create the new module and register it into the configuration:

  CPH7106Latch* pDevice = new CPH7106Latch;
  pModule = new CReadoutModule(name, *pDevice);  // Does onAttach as well.

  try {
    configure(interp, pModule, objv); // Throws appropriate error message.

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
 * config
 *   Process the module configuration.
 *
 * @param interp - interpreter on which the command is executing.
 * @param objv   - COmmand words being executed.
 */
void
CPH7106Command::config(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  // validate parameter counts.  At least 5 parameters and an odd number:

  requireAtLeast(
     objv, 5, "ph7106 config requires at least a name and one configuration name/value pair"
  );
  if((objv.size() & 1) == 0) {
    throw std::string("ph7106 config - must have an odd number of parameters");
  }

  // Get the module from the configuration or complain if the name is invalid:

  std::string name = objv[2];
  CReadoutModule* pModule = m_Config.findAdc(name);
  if (!pModule) {
    throw std::string("ph7106 config - module name not found");
  }
  configure(interp, pModule, objv);
  interp.setResult(name);
  
}
/**
 * cget
 *   Sets the result to a list of two element configuration option name/value pairs.
 *
 * @param interp - interpreter running the command.
 * @param objv   - vector of command words.
 */
void
CPH7106Command::cget(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  requireExactly(objv, 3, "ph7106 cget requires only the module name follow the subcommand");
  
  // Get the module's configuration:

  std::string name = objv[2];
  CReadoutModule* pModule = m_Config.findAdc(name);
  if(!pModule) {
    throw std::string("cph7106 cget modulename not found");
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
 *  configure
 *   Actually configure a module given some key/value pairs.
 * 
 * @param  interp - interpreter running the command that is calling us.
 * @param  pModule - The module being configured.
 * @param  config  - Command words.
 * @param  firstPair - Index into config of the first configuration key/value pair.
 */
void
CPH7106Command::configure(
    CTCLInterpreter& interp, CReadoutModule* pModule, std::vector<CTCLObject>& objv, int firstPair
)
{
  std::string baseMessage = "ph7106 - invalid configuration/value pair: ";
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
 *   Produce a configuration message from its elements.
 *
 * @param base - The base error message.
 * @param key  - The keyword that resulted in the error.
 * @param value - The value that resulted in the error.
 * @param errorMessage - The specific error message from the config subsystem.
 */
std::string
CPH7106Command::configMessage(
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
