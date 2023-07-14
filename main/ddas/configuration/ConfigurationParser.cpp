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
 * Regular expression matching 
 * @verbatim "(^\[Rev([xXa-fA-F0-9]+)-(\d+)Bit-(\d+)MSPS\]$)" \endverbatim 
 * to extract the firmware, bit depth, and module MSPS.
 */
DAQ::DDAS::ConfigurationParser::ConfigurationParser()
    : m_matchExpr(R"(^\[Rev([xXa-fA-F0-9]+)-(\d+)Bit-(\d+)MSPS\]$)")
{}

/**
 * @details
 * Check for file extension ending in .set is not fully implemented. 
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

    std::string ComFPGAConfigFile_RevBCD;
    std::string SPFPGAConfigFile_RevBCD;
    std::string DSPCodeFile_RevBCD;
    std::string DSPVarFile_RevBCD;

    std::string ComFPGAConfigFile_RevF_250MHz_14Bit;
    std::string SPFPGAConfigFile_RevF_250MHz_14Bit;
    std::string DSPCodeFile_RevF_250MHz_14Bit;
    std::string DSPVarFile_RevF_250MHz_14Bit;

    std::string ComFPGAConfigFile_RevF_500MHz_12Bit;
    std::string SPFPGAConfigFile_RevF_500MHz_12Bit;
    std::string DSPCodeFile_RevF_500MHz_12Bit;
    std::string DSPVarFile_RevF_500MHz_12Bit;

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
    input.getline(temp,FILENAME_STR_MAXLEN);

    /**
     * @todo (ASC 7/12/23): Checking ".set" with std::string::find_last_of as 
     * a method to see if the file has a .set extension will almost always 
     * succeed, as it returns the last character matching _any_ of ".set." 
     * Check the substring defining the extension (assume its after last "." ?)
     * or remove the check entirely if we dont want to enforce a naming 
     * convention. API 3 supports JSON settings files so .json would be an 
     * obvious choice to support in that case as well. 
     */
    
    // Check to make sure that this line contains a set file
    // (.set extension) since the format has changed from previous
    // versions of the code.
    if( DSPParFile.find_last_of(".set") == std::string::npos) {
	std::string errmsg("The file ");
	errmsg += DSPParFile;
	errmsg += " read in from configuration file ";
	errmsg += "does not appear to be a *.set file ";
	errmsg += "required by DDAS";
	throw std::runtime_error(errmsg);
    }

    /** 
     * @todo (ASC 7/7/23): The ability to read the [XXXMSPS] tags is
     * deprecated and should be removed.
     */
    // The [100MSPS], [250MSPS], and [500MSPS] tags are still
    // supported but should not be used.
    while (getline(input, line)) {
	if (line == "[100MSPS]") {
	    FirmwareConfiguration fwConfig
		= extractFirmwareConfiguration(input);
	    double calibration = extractClockCalibration(input);

	    config.setFirmwareConfiguration(
		HardwareRegistry::RevB_100MHz_12Bit, fwConfig
		);
	    updateClockCalibration(
		HardwareRegistry::RevB_100MHz_12Bit, calibration
		);

	    config.setFirmwareConfiguration(
		HardwareRegistry::RevC_100MHz_12Bit, fwConfig
		);
	    updateClockCalibration(
		HardwareRegistry::RevC_100MHz_12Bit, calibration
		);

	    config.setFirmwareConfiguration(
		HardwareRegistry::RevD_100MHz_12Bit, fwConfig
		);
	    updateClockCalibration(
		HardwareRegistry::RevD_100MHz_12Bit, calibration
		);

	} else if (line == "[250MSPS]"){
	    FirmwareConfiguration fwConfig
		= extractFirmwareConfiguration(input);
	    double calibration = extractClockCalibration(input);

	    config.setFirmwareConfiguration(
		HardwareRegistry::RevF_250MHz_14Bit, fwConfig
		);
	    updateClockCalibration(
		HardwareRegistry::RevF_250MHz_14Bit, calibration
		);

	} else if (line == "[500MSPS]"){
	    FirmwareConfiguration fwConfig
		= extractFirmwareConfiguration(input);
	    double calibration = extractClockCalibration(input);

	    config.setFirmwareConfiguration(
		HardwareRegistry::RevF_500MHz_12Bit, fwConfig
		);
	    updateClockCalibration(
		HardwareRegistry::RevF_500MHz_12Bit, calibration
		);

	} else if (std::regex_match(line , m_matchExpr) ) {
	    int revision, adcFreq, adcRes;
	    if (
		parseHardwareTypeTag(line, revision, adcFreq, adcRes)
		) {

		FirmwareConfiguration fwConfig
		    = extractFirmwareConfiguration(input);
		double calibration = extractClockCalibration(input);
		int type = HardwareRegistry::createHardwareType(
		    revision, adcFreq, adcRes, calibration
		    );
		config.setFirmwareConfiguration(type, fwConfig);
	    } else {
		std::string msg(
		    "ConfigurationParser::parse() Failed to parse "
		    );
		msg += " the hardware tag '" + line + "'";
		throw std::runtime_error(msg);
	    }
	}
    }

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
 * Parses the values of X, Y, and Z from a tag of the form [RevX-YBit-ZMSPS].
 */
bool
DAQ::DDAS::ConfigurationParser::parseHardwareTypeTag(
    const std::string& line, int &revision, int &freq, int &resolution
    )
{
    bool result = false;
    std::smatch color_match;
    std::regex_search(line, color_match, m_matchExpr);

    if (color_match.size() == 4) {
	std::string revStr(color_match[1].first, color_match[1].second);
	revision = std::stoi(revStr, 0, 0); // auto detect base
	resolution = std::stoi(
	    std::string(color_match[2].first, color_match[2].second)
	    );
	freq = std::stoi(
	    std::string(color_match[3].first, color_match[3].second)
	    );
	result = true;
    }
    return result;
}

/**
 * @details
 * The current implementation does not support reading firmware paths with
 * whitespace in them.
 */
DAQ::DDAS::FirmwareConfiguration
DAQ::DDAS::ConfigurationParser::extractFirmwareConfiguration(
    std::istream &input
    )
{
    FirmwareConfiguration fwConfig;
    
    // Load in files to overide defaults

    // Load syspixie
    input >> fwConfig.s_ComFPGAConfigFile;
    if (!input.good())
	throw std::runtime_error(
	    "Configuration file contains incomplete hardware specification!"
	    );

    // Load fippipixe
    input >> fwConfig.s_SPFPGAConfigFile;
    if (!input.good())
	throw std::runtime_error(
	    "Configuration file contains incomplete hardware specification!"
	    );

    // Load ldr file
    input >> fwConfig.s_DSPCodeFile;
    if (!input.good())
	throw std::runtime_error(
	    "Configuration file contains incomplete hardware specification!"
	    );

    // Load var file
    input >> fwConfig.s_DSPVarFile;
    if (!input.good())
	throw std::runtime_error(
	    "Configuration file contains incomplete hardware specification!"
	    );

    return fwConfig;
}

// Returns the clock calibration in ns/clock tick.
double
DAQ::DDAS::ConfigurationParser::extractClockCalibration(std::istream& input)
{
    double calibration;
    input >> calibration;
    if (!input.good()) {
	std::string errmsg = "ConfigurationParser attempted to parse an "
	    "incomplete hardware specification!";
	throw std::runtime_error(errmsg);
    }
    
    return calibration;
}

/**
 * @details
 * Retrieve hardware specification from the type enum and set its clock
 * calibration to the new value specified by the calibration param. The 
 * type may be Unknown or not mapped, in which case trying to update its
 * clock calibration is an error.
 *
 * @todo (ASC 7/14/23): Catch and handle exceptions thrown by 
 * HardwareRegistry::getSpecification(). 
 */
void
DAQ::DDAS::ConfigurationParser::updateClockCalibration(
    int type, double calibration
    )
{
    HardwareRegistry::HardwareSpecification& hdwrSpec
	= HardwareRegistry::getSpecification(type);
    hdwrSpec.s_clockCalibration = calibration;
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
