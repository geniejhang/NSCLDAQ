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
#include <CPH7106Latch.h>
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

CJoergerClockCommand::CJoergerClockCommand(CTCLInterpreter& interp, CConfiguration& config, std::string command) :
  CTCLObjectProcessor(interp, command, true),
  m_pConfig(&config)
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
 * @return int    - TCL_OK - success, TCL_ERROR - failure.
 */
int
CJoergerClockCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  bindAll(interp, objv);
  try {
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
