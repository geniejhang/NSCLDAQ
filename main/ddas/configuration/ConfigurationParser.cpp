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

#include <config.h>

#include "Configuration.h"
#include "FirmwareVersionFileParser.h"

#define FILENAME_STR_MAXLEN 256 //!< Number of characters to skip when parsing a line. Maximum allowed length of any comment added by a user.

/**
 * @details
 * Parses the configuration file line by line, extracting slot information, 
 * optional per-module firmware and DSP settings files, and the (default) 
 * per-crate DSP settings file. DSP settings files must have the file extension
 * .set for XIA API 2 and either .set or .json for XIA API 3. Exits on any 
 * error processing the configuration file.
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
    for(int i = 0; i < NumModules; i++) {
	DAQ::DDAS::ConfigurationParser::SlotSpecification slotInfo;
	try {
	    slotInfo = parseSlotLine(input);
	}
	catch (std::runtime_error& e) {
	    std::cerr << e.what() << std::endl;
	}
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

    // Check the file extension. If XIA API 2, it must be .set, if XIA API 3,
    // it can be either .set or .json. Assume whatever comes after the last '.'
    // is the extension.
    size_t pos = DSPParFile.find_last_of('.');
    if (pos != std::string::npos) {
	std::string ext = DSPParFile.substr(pos + 1);
	std::transform(
	    ext.begin(), ext.end(), ext.begin(),
	    [](unsigned char c){ return std::tolower(c); }
	    );
#if XIAAPI_VERSION >= 3
	if (ext != "set" && ext != "json") {
	    std::stringstream errmsg;
	    errmsg << "The DSP settings file " << DSPParFile << " read from"
		   << " cfgPixie16.txt must have the extension 'set' or"
		   << " 'json' but read '" << ext << "'.";
	    throw std::runtime_error(errmsg.str()); 
	}
#else
	if (ext != "set") {
	    std::stringstream errmsg;
	    errmsg << "The DSP settings file " << DSPParFile << " read from"
		   << " cfgPixie16.txt must have the extension 'set' but"
		   << " read '" << ext << "'.";
	    throw std::runtime_error(errmsg.str()); 
	}
#endif
    } else {
	std::stringstream errmsg;
#if XIAAPI_VERSION >= 3
	errmsg << "The DSP settings file " << DSPParFile << " read from"
	       << " cfgPixie16.txt must have the extension 'set' or 'json'.";
#else
	errmsg << "The DSP settings file " << DSPParFile << " read from"
	       << " cfgPixie16.txt must have the extension 'set'.";
#endif
	throw std::runtime_error(errmsg.str());
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
