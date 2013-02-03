/******************************************************************************
*
* Via Vetraia, 11 - 55049 - Viareggio ITALY
* +390594388398 - www.caen.it
*
******************************************************************************/
#ifndef __CGSTRIGGERCOMMAND_H
#define __CGSTRIGGERCOMMAND_H
/**
 * @file  CGSTriggerCommand.h
 * @brief Command class to instantiate, configure an query Gammasphere trigger modules.
 * @author Ron Fox (ron@caentechnologies.com)
 */

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

/**
 * @class CGSTriggerCommand
 *
 * This class is a module wrapper command.  It implements the three
 * subcommands required for creating, configuring and querying
 * CGSTriggerModule objects.  Those objects are responsible for
 * interfacing with and manipulating John T. Anderson's trigger module
 * for Gammasphere as embedded in the Chico 2 detector system.
 *
 * For configuration options and other details, see the header for
 *  CGSTriggerModule.
 */
class CGSTriggerCommand : public CTCLObjectProcessor
{
    // Object internal data:
    
private:
    CConfiguration& m_Config;
    
    // Canonicals, both supported and un.
    
public:
    CGSTriggerCommand(
        CTCLInterpreter& interp, CConfiguration&  config,
	std::string      commandName = std::string("dgstrigger")
    );
    virtual ~CGSTriggerCommand();
private:
    CGSTriggerCommand(const CGSTriggerCommand&);
    CGSTriggerCommand& operator=(const CGSTriggerCommand&);
    int operator==(const CGSTriggerCommand&) const;
    int operator!=(const CGSTriggerCommand&) const;
    
    /*
      override of the CTCLObjectProcessor interface, specifically
      the command processing method:
    */
public:
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    /*
        Private utility methods.. it's convenient to break out the
        code associated with each subcommand into a separate method:
    */
private:
    int create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    int config(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    int cget   (CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

};



#endif
