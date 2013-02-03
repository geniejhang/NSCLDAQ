/******************************************************************************
*
* Via Vetraia, 11 - 55049 - Viareggio ITALY
* +390594388398 - www.caen.it
*
******************************************************************************/

/**
 * @file CGSTriggerCommand.cpp
 * @brief Implementation of the gammasphere trigger command.
 * @author Ron Fox (ron@caentechnologies.com)
 */

#include <config.h>
#include <CGSTriggerCommand.h>
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "CConfiguration.h"

#include <CGSTriggerModule.h>
#include <CReadoutModule.h>

#include <stdlib.h>
#include <errno.h>


/*-----------------------------------------------------------------------------
 * Canonical methods:
 *---------------------------------------------------------------------------*/

/**
 * constructor
 *
 * Creates the command and saves the configuration object so that
 * modules can be located and added to the current configuration.
 *
 * @param interp - Reference to the Tcl interpreter on which the command
 *                 will be registered.
 * @param config - The current module configuration.
 * @param name   - The command name (defaults to "dgstrigger")
 */
CGSTriggerCommand::CGSTriggerCommand(
    CTCLInterpreter& interp, CConfiguration& config, std::string name) :
    CTCLObjectProcessor(interp, name, true),
    m_Config(config)
{}

/**
 * destructor
 *
 * Chain method to the base class destructor which unregisters the command
 * from all interpreters on which it was bound/registered.
 */
CGSTriggerCommand::~CGSTriggerCommand()
{}

/*-----------------------------------------------------------------------------
 * Implement the interface of CTCLObjectProcessor
 */

/**
 * @note For command processing we are going to follow the pattern of centralizing
 * error handlnig by throwing a string exception when the command, or its
 * subprocessors detect errors.  The exception string becomes the
 * result and is mapped at operator() to a return value of TCL_ERROR.
 */

/**
 * operator()
 *
 *   Just does basic validation of the minimal parameter list lenght
 *   and dispatches to the appropriate subcommand handler.
 *
 *   @param interp - Interpreter that is executing the command.
 *   @param objv   - Vector of command words that make up the
 *                   full command string.
 *  @return int
 *  @retval TCL_OK - If the command executed successfully.
 *  @retval TCL_ERROR - If the command failed.
 */
int
CGSTriggerCommand::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    try {
        /*
          Validate the argument count and bind the parameters to the interp.
          There must be at least 3 command words (commands subcommand name).
        */
        if(objv.size() < 3) {
            throw std::string("dgstrigger - Too few command line parameters");
        }
        for (int i = 0; i < objv.size(); i++) {
            objv[i].Bind(interp);
        }
        // Get the subcommand and dispatch to the processor or throw if no match:
        
        std::string subcommand = objv[1];
        if (subcommand == "create") {
            return create(interp, objv);
        } else if (subcommand == "config") {
            return config(interp, objv);
        } else if (subcommand == "cget") {
            return config(interp, objv);
        } else {
            std::string msg("dgstrigger - invalid subcommand: ");
            msg += subcommand;
            throw msg;
        }
    }
    catch(std::string msg){
        interp.setResult(msg);
        return TCL_ERROR;
    }
    return TCL_OK;
}
/*--------------------------------------------------------------------------
 *  Utility methods; specifically sub command processors.
 */

/**
 * create
 *
 *    Create a new trigger module object and add it to the configuration.
 *   @param interp - Interpreter that is executing the command.
 *   @param objv   - Vector of command words that make up the
 *                   full command string.
 *                   
 *  @return int TCL_OK - All errors are thrown up with an std::string
 *                       exception.
 */
int
CGSTriggerCommand::create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    /*
      We already know we have at least 3 params. We need an odd number
      of params to be able to do post creation configuration.
      While this is also checke in config, checking here ensures we don't even crate the
      module if we have the wrong number of parameters.
    */
    if ((objv.size() & 1) == 0) {
        throw std::string("dgstrigger create - invalid number of parameters");
    }
    // Pull out the module name and look it up.  It's an errror to duplicate:
    
    std::string name = objv[2];
    CReadoutModule* pModule =m_Config.findAdc(name);
    if (pModule) {
        std::string msg("dgstrigger create - Module named ");
        msg += name;
        msg += " already exists";
        throw msg;
    }
    // Create the module and its cofiguration and  register it into the config:
    
    CReadoutHardware* pHardware = new CGSTriggerModule();
    pModule = new CReadoutModule(name, *pHardware);
    
    /*
      If there are additional parameters, invoke config to do the configuration.
      Note that if one of the configurations attempted is illegal, we'll still
      have the module partially configured though the command will error.
      No help for that with the current API.
    */
    if (objv.size() > 3) {
        config(interp, objv);
    }
    // Set the result to the module name and return ok.

    interp.setResult(name);
    return TCL_OK;
    
}
/**
 *  config
 *    
 * Processes the config subcommand.  This locates the configuration of an existing
 * driver instance and modifies it in accordance with the command line parameters.
 *
 * The command line parameters in addition to the subcommand are:
 * -  The name of the driver instance to configure.
 * -  Configuration options and their values 
 *
 * Example:
 *\verbatim
 *   dgstrigger config trigger -base 0x120000
 *\endverbatim
 *
 *  Configures the base address of the module named 'trigger'.  Note that as many configuration
 *  option/value pairs as needed can be supplied on the same line.  In the event multiple
 *  instances of the same option appear, the syntactically later one rules.
 *
 *   @param interp - Interpreter that is executing the command.
 *   @param objv   - Vector of command words that make up the
 *                   full command string.
 *  @return int
 *  @retval TCL_OK - If the command executed successfully.
 *  @retval TCL_ERROR - If the command failed.
 */
int
CGSTriggerCommand::config(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  /*
     We require an odd number of parameters and at least 5 of them (no point in an
     empty configuration.
  */
  if (objv.size() < 5) {
    throw std::string("dgstrigger config - insufficent number of parameters");
  }
  if ((objv.size() & 1) == 0) {
    throw std::string("dgstrigger config - invalid number of parameters");
  }
  //  Locate the module in the configuration:

  std::string     moduleName  = objv[2];
  CReadoutModule* pModule     = m_Config.findAdc(moduleName);
  if (!pModule) {
    std::string error("dgstrigger config - no such module: ");
    error += moduleName;
		      
    throw std::string(error);
  }
  // At this point additional errors result in configuration up until the failing config/option pair.

  for (int i = 3; i < objv.size(); i += 2) {
    std::string option = objv[i];
    std::string value  = objv[i+1];
    pModule->configure(option, value);
  }
  // Set the command result to the module name and the status of the command to ok.
  interp.setResult(moduleName);
  return TCL_OK;
}
/**
 * cget
 *
 * Processes the cget subcommand.   This looks up the module by name and 
 * sets the command result to a list of pairs.  Each pair contains, in order,
 * the name of a configuration option and its value.  No order is implied in this list.
 * it is perfectly legal for the list to change order from invocation to invocation (though
 * not likely)... What I'm trying to say is don't do [lindex $result 2] and expect this to stably
 * be what you expect it to be.
 *
 *   @param interp - Interpreter that is executing the command.
 *   @param objv   - Vector of command words that make up the
 *                   full command string.
 *  @return int
 *  @retval TCL_OK - If the command executed successfully.
 *  @retval TCL_ERROR - If the command failed.
 */
int
CGSTriggerCommand::cget(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  // We only want the subcommand and the module name:

  if (objv.size() != 3) {
    throw std::string("dgstrigger cget - Must be exactly three command line words!");
  }
  // Look up the module:

  std::string     moduleName = objv[2];
  CReadoutModule* pModule    = m_Config.findAdc(moduleName);
  if (!pModule) {
    std::string error("dgstrigger cget - No such module: ");
    error += moduleName;
    throw error;
  }
  // Get the configuration and turn it into a TclList:

  CConfigurableObject::ConfigurationArray config = pModule->cget();
  CTCLObject result;
  result.Bind(interp);

  for (int i = 0; i < config.size(); i++) {
    CTCLObject item;
    item.Bind(interp);
    item += config[i].first;	// Config option name
    item += config[i].second;	// Config option value.

    result += item;		// Add the pair to the list.
  }
  // Set the result to 'result' and return ok:

  interp.setResult(result);
  return TCL_OK;
  
}
