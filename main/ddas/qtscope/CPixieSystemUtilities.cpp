/**
 * @file CPixieSystemUtilities.cpp
 * @brief Implementation the Pixie DAQ system utilities class.
 */

#include "CPixieSystemUtilities.h"

#include <sstream>
#include <iostream>

#include <config.h>
#include <config_pixie16api.h>

#include <SystemBooter.h>
#include <CXIAException.h>

using namespace DAQ::DDAS;
namespace HR = DAQ::DDAS::HardwareRegistry;

/**
 * @details
 * Default: boot in online mode and read the settings file specified in 
 * cfgPixie16.txt.
 */
CPixieSystemUtilities::CPixieSystemUtilities() :
    m_bootMode(0), m_booted(false), m_ovrSetFile(false), m_numModules(0),
    m_modEvtLength(0), m_modADCMSPS(0), m_modADCBits(0), m_modRev(0),
    m_modClockCal(0)
{}

/**
 * @details
 * Reads in configuration information from cfgPixie16.txt, loads settings file 
 * information, boots modules and saves configuration info.
 */
int
CPixieSystemUtilities::Boot()
{
    // If the settings file path has been overwritten pre system boot, use
    // the new path. first we grab it, then reset it after initializing
    // the configuration settings below.  
    std::string newSetFile;
    if (m_ovrSetFile) {
	newSetFile = m_config.getSettingsFilePath();
    }
  
    // Create a configuration from the default settings:  
    const char* fwFile =  FIRMWARE_FILE; // From $DDAS_SHARE.
    char* alternateFirmwareFile = getenv("FIRMWARE_FILE");
    if (alternateFirmwareFile) fwFile = alternateFirmwareFile;
    m_config = *(
	Configuration::generate(fwFile, "cfgPixie16.txt", "modevtlen.txt")
	);

    // (Re)set the custom settings file path here if used:  
    if (m_ovrSetFile) {
	m_config.setSettingsFilePath(newSetFile);
    }
  
    // Assume full boot:
    SystemBooter booter;  
    booter.setOfflineMode(m_bootMode); // 1: offline, 0: online
    try {
	booter.boot(m_config, SystemBooter::FullBoot);
    }
    catch (const CXIAException& e) {
	std::cerr << e.ReasonText() << std::endl;    
	return -1;
    }

    // Get number of modules and set event lengths based on modevtlen file:  
    m_numModules = m_config.getNumberOfModules();
    m_modEvtLength = m_config.getModuleEventLengths();
  
    // The hardware map is set up during boot time:  
    std::vector<int> hdwrMap = m_config.getHardwareMap();
    for (size_t i = 0; i < hdwrMap.size(); i++) {
	HR::HardwareSpecification spec = HR::getSpecification(hdwrMap.at(i));
	m_modADCMSPS.push_back(spec.s_adcFrequency);
	m_modADCBits.push_back(spec.s_adcResolution);
	m_modRev.push_back(spec.s_hdwrRevision);
	m_modClockCal.push_back(spec.s_clockCalibration);
    }
  
    m_booted = true;
  
    return 0;
}

/**
 * @details
 * File format depends on what is supported by the version of the XIA API being 
 * used. Version 3+ will save the settings file as a JSON file while in version
 * 2 it is binary.
 */
int
CPixieSystemUtilities::SaveSetFile(char* fileName)
{
    int retval;
    try {
	int retval = Pixie16SaveDSPParametersToFile(fileName);
    
	if (retval < 0) {
	    std::stringstream msg;
	    msg << "CPixieSystemUtilities::SaveSetFile() failed to save"
		<< " DSP parameter file to: " << fileName;
	    throw CXIAException(
		msg.str(), "Pixie16SaveDSPParametersToFile()", retval
		);
	}
    }
    catch (const CXIAException& e) {
	std::cerr << e.ReasonText() << std::endl;
    }
  
    return retval;
}

/**
 * @details
 * Check and see if the system is booted. If so, load the parameters from 
 * the settings file. If not flag that a new settings file path (potentially 
 * different from that in the cfgPixie16.txt) has been set. The flag is 
 * checked at boot to load the new settings file.
 */
int
CPixieSystemUtilities::LoadSetFile(char* fileName)
{
    int retval;
    try {  
	if(m_booted) { // If system is booted just apply the params.    
	    retval = Pixie16LoadDSPParametersFromFile(fileName);
    
	    if (retval < 0) {
		std::stringstream msg;
		msg << "CPixieSystemUtilities::LoadSetFile() failed to"
		    << " load DSP parameter file from: " << fileName;
		throw CXIAException(
		    msg.str(), "Pixie16LoadDSPParametersFromFile()", retval
		    );
	    } else {
		std::cout << "Loading new DSP parameter file from: "
			  << fileName << std::endl;
	    }  
	} else { // Not booted so hold on to the name.
	    m_ovrSetFile = true;
	    m_config.setSettingsFilePath(fileName);
	    std::cout << "New DSP parameter file " << fileName
		      << " will be loaded on system boot" << std::endl;
	}
    }
    catch (const CXIAException& e) {
	std::cerr << e.ReasonText() << std::endl;
	return e.ReasonCode();
    }
  
    return retval;
}

/**
 * @details
 * If the call to Pixie16ExitSystem() fails for any module, return the error 
 * code and set the booted state flag to false. The system is likely in a bad 
 * state.
 */
int
CPixieSystemUtilities::ExitSystem()
{
    int retval;
    try {
	if (m_booted) {
	    for (int i = 0 ; i < m_numModules; i++) {      
		retval = Pixie16ExitSystem(i);      
		if (retval < 0) {
		    std::stringstream msg;
		    msg << "CPixieSystemUtilities::ExitSystem() failed"
			<< " to exit " << "module " << i;
		    throw CXIAException(
			msg.str(), "Pixie16ExitSystem()", retval
			);
		}
	    }    
	}  
	m_booted = false;
    }
    catch (const CXIAException& e) {
	std::cerr << e.ReasonText();
	m_booted = false;
	return e.ReasonCode();
    }
      
    return retval; // All good.
}

/**
 * @todo (ASC 7/14/23): Would be better to return max(unsigned int) or 
 * something besides 0 if an exception is raised. Must also be checked 
 * wherever this return value is used on the Python side.
 */
unsigned short
CPixieSystemUtilities::GetModuleMSPS(int module)
{
    int nmodules = m_modADCMSPS.size() - 1;
    try {
	if (!m_booted) {
	    std::stringstream errmsg;
	    errmsg << "CPixieSystemUtilities::GetModuleMSPS() ";
	    errmsg << "system not booted.";
	    throw std::runtime_error(errmsg.str());
	} else if ((module < 0) || (module > nmodules)) {
	    std::stringstream errmsg;
	    errmsg << "CPixieSystemUtilities::GetModuleMSPS() ";
	    errmsg << "invalid module number ";
	    errmsg << module << " for " << nmodules << " module system.";
	    throw std::range_error(errmsg.str());
	} else {
	    return m_modADCMSPS[module];
	}
    }
    catch (const std::range_error& e) {
	std::cerr << e.what() << std::endl;
	return 0;
    }
}
