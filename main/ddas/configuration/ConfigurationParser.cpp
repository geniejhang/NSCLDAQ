/**
 * @file ConfigurationParser.cpp
 * @brief Define a class to parse the cfgPixie16.txt file.
 */

#include "ConfigurationParser.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <unistd.h>

#include "Configuration.h"
#include "FirmwareVersionFileParser.h"

#define FILENAME_STR_MAXLEN 256 //!< Number of characters to skip when parsing a line. Maximum allowed length of any comment added by a user.

/**
 * @details
 * Supported hardware tags have the format RevX-YBit-ZMSPS and are
 * matched by regular expression to check that the format is good.
 */
void
DAQ::DDAS::ConfigurationParser::parse(
    std::istream &input, DAQ::DDAS::Configuration &config
    )
{
    int CrateNum;
    std::vector<unsigned short> PXISlotMap;
    int NumModules;

    char temp[FILENAME_STR_MAXLEN];
    std::string line;
    std::string DSPParFile;

    // std::string ComFPGAConfigFile_RevBCD;
    // std::string SPFPGAConfigFile_RevBCD;
    // std::string DSPCodeFile_RevBCD;
    // std::string DSPVarFile_RevBCD;

    // std::string ComFPGAConfigFile_RevF_250MHz_14Bit;
    // std::string SPFPGAConfigFile_RevF_250MHz_14Bit;
    // std::string DSPCodeFile_RevF_250MHz_14Bit;
    // std::string DSPVarFile_RevF_250MHz_14Bit;

    // std::string ComFPGAConfigFile_RevF_500MHz_12Bit;
    // std::string SPFPGAConfigFile_RevF_500MHz_12Bit;
    // std::string DSPCodeFile_RevF_500MHz_12Bit;
    // std::string DSPVarFile_RevF_500MHz_12Bit;

    // Maps capture nosuch better than arrays...
    std::map<int, FirmwareMap> perModuleFirmware;
    std::map<int, std::string> perModuleSetfiles;

    // Extract the first value and skip the next FILENAME_STR_MAXLEN
    // characters from this line.
    input >> CrateNum;
    input.getline(temp, FILENAME_STR_MAXLEN);
    input >> NumModules;
    input.getline(temp, FILENAME_STR_MAXLEN);
    PXISlotMap.resize(NumModules);
    for(int i = 0; i < NumModules; i++){
	auto slotInfo = parseSlotLine(input);
	PXISlotMap[i] = std::get<0>(slotInfo);
        
	std::string perModuleMap = std::get<1>(slotInfo);
	if (!perModuleMap.empty()) {
	    std::ifstream fwMapStream(perModuleMap);
	    FirmwareMap aMap;
	    FirmwareVersionFileParser fwparser;
	    fwparser.parse(fwMapStream, aMap);
	    perModuleFirmware[i] = aMap;
	    std::string perModuleSetfile = std::get<2>(slotInfo);
	    if (!perModuleSetfile.empty()) {
		perModuleSetfiles[i] = perModuleSetfile;
	    }
	}
        
    }
    input >> DSPParFile;
    input.getline(temp, FILENAME_STR_MAXLEN);

    /**
     * @note (ASC 9/5/23): Deprecated and removed:
     * - Broken check for ".set" extension. The configuration parser does not
     *   care what (if any) file extension the DSPParFile has. 
     * - Reading firmware configuration using hardware tags from lines past the
     *   path to the settings file. Firmware for each module is deduced from
     *   the module type itself on boot and any custom firmware should be
     *   loaded on a per-module basis when calling parse().
     */

    // while (getline(input, line)) {
    // 	int revision, adcFreq, adcRes;
    // 	if (parseHardwareTypeTag(line, revision, adcFreq, adcRes)) {
    // 	    FirmwareConfiguration fwConfig
    // 		= extractFirmwareConfiguration(input);
    // 	    double calibration = extractClockCalibration(input);
    // 	    int type = HardwareRegistry::createHardwareType(
    // 		revision, adcFreq, adcRes, calibration
    // 		);
    // 	    config.setFirmwareConfiguration(type, fwConfig);
    // 	} else {
    // 	    std::string msg("ConfigurationParser::parse() Failed to parse ");
    // 	    msg += " the hardware tag '" + line + "'";
    // 	    throw std::runtime_error(msg);
    // 	}	
    // }

    config.setCrateId(CrateNum);
    config.setNumberOfModules(NumModules);
    config.setSlotMap(PXISlotMap);
    config.setSettingsFilePath(DSPParFile);
    
    // Set the per module firmware maps:    
    for (auto const& p : perModuleFirmware) {
	config.setModuleFirmwareMap(p.first, p.second);
    }
	    
    // Set the per module DSP Parameter maps:    
    for (auto const& p : perModuleSetfiles) {
	config.setModuleSettingsFilePath(p.first, p.second);
    }
}

/**
 * @details 
 * Slot lines consist of a mandatory slot number, and optional substitute
 * firmware mapping file and an optional .set file for that module. Care must
 * be taken since any populated field (other than the slot number) might 
 * actually be a comment. Requirements:
 *  - Filenames cannot have spaces in their paths.
 *  - Files must be readable by the user.
 *  - #'s must be spaced from the last file e.g.:
 *      1 firmwaremap#  this is an error but,
 *      2 firmwaremap  # This is ok,
 *      3 firmwaremap setfile.set # as is this.
 */
DAQ::DDAS::ConfigurationParser::SlotSpecification
DAQ::DDAS::ConfigurationParser::parseSlotLine(std::istream& input)
{    
    std::string line;
    std::getline(input, line);
    
    if (!input.good()) { // Maybe eof?
	std::string errmsg = "Unable to read a line from the input file "
	    "when parsing a slot line";
	throw std::runtime_error(errmsg);
    }
    std::stringstream lineStream(line);
    
    int slot;
    std::string firmwareMap;
    std::string setFile;
    
    // Do the slot separately so that we can indicate we can't parse:
    
    if (!(lineStream >> slot)) {    
	std::string errmsg("Unable to parse a slot number from: ");
	errmsg += line;       
	throw std::runtime_error(errmsg);
    }
    
    // Now the files:
    
    lineStream >> firmwareMap >> setFile;
    
    // Handle leading #'s which imply a comment.    
    if (firmwareMap[0] == '#') { // Both are comments.
	firmwareMap.clear();
	setFile.clear();
    } else if (setFile[0] == '#') { // Setfile is a comment
	setFile.clear();               
    }
    
    // Check readability of any files:
    
    if (firmwareMap != "") {
	if (access(firmwareMap.c_str(), R_OK)) {
	    std::string msg("Unable to read firmware mapping file ");
	    msg += firmwareMap;
	    msg += " from ";
	    msg += line;            
	    throw std::runtime_error(msg);
	}
	if (setFile != "") {
	    if (access(setFile.c_str(), R_OK)) {
		std::string msg("Unable to read DSP Parameter file ");
		msg += setFile;
		msg += " from ";
		msg += line;                 
		throw std::runtime_error(msg); 
	    }
	}
    }
	    
    // Ok everything is good so:
    
    return std::make_tuple(slot, firmwareMap, setFile);
}
