/**
 * @addtogroup configuration libConfiguration.so
 * @brief DDAS Pixie-16 hardware configuration library.
 *
 * Shared library containing classes to manage the internal configuration of a 
 * DDAS system and store information about its hardware. Contains all functions
 * defined in the DAQ::DDAS::HardwareRegistry namespace.
 * @{
 */

/**
 * @file ConfigurationParser.h
 * @brief Define a class to parse the contents of the cfgPixie16.txt file.
 */

#ifndef CONFIGURATIONPARSER_H
#define CONFIGURATIONPARSER_H

#include <iosfwd>
#include <regex>
#include <tuple>

/** @namespace DAQ */
namespace DAQ {
    /** @namespace DAQ::DDAS */
    namespace DDAS {

	class Configuration;
	class FirmwareConfiguration;

/*!
 * \brief A class to parse the contents ofthe cfgPixie16.txt file.
 *
 * This file is pretty basic. It contains information about the slot map, 
 * crate id, and settings file path. It has the following form:
 *
 \verbatim
 CRATE_ID
 NUM_MODULES
 SLOT_MODULE_0   [Per-module-firmware-map [per-module-set-file]]
 SLOT_MODULE_1   [Per-module-firmware-map [per-module-set-file]]
 ...
 SLOT_MODULE_N-1
 PATH_TO_SETTINGS_FILE
 \endverbatim
 * 
 * where CRATE_ID is a non-negative number, NUM_MODULES is a positive number, 
 * SLOT_MODULE_# is a number greater than or equal to 2, and 
 * PATH_TO_SETTINGS_FILE is a legitimate path. In the top section, the parser 
 * will ignore up to 256 characters following the leftmost integer or string 
 * found on each line. Because of this, it is customary to add notes on each 
 * of these lines. There is no convention for adding notes, though many people
 * like to use a #. An example would be (note the varying conventions for
 * demonstration):
 *
 \verbatim
 1    # crate id
 2    number of modules
 2    | slot of first module
 3    - slot of second module
 /path/to/setfile.set ! another comment
 \endverbatim
 *
 * Note the structure shown above reflects changes for issue daqdev/DDAS#106.
 * Each slot specification can have an optional one or two fields: The first 
 * optional field is a per slot firmware map file and the second an optional 
 * per slot .set file (since optional firmwares may require .set files of a 
 * different format).
 *
 * The ConfigurationParser can be used in the following fashion:
 *
 * \code
 * using namespace DAQ::DDAS;
 *
 * Configuration config;
 * ConfigurationParser parser;
 *
 * std::ifstream configFile("cfgPixie16.txt", std::ios::in);
 *
 * parser.parse(configFile, config);
 *
 * \endcode
 *
 */
	class ConfigurationParser
	{
	public:
	    /**
	     * @typedef SlotSpecification
	     * @brief Data returned when parsing a slot.
	     * 
	     * daqdev/DDAS#106. This typedef defines the data that can be 
	     * returned when parsing a slot line. The int is the slot number.
	     * The first string is the optional firmware map (empty string if
	     * not given) and the  last the optional .set file specification 
	     * (empty if not given).
	     */
	    typedef std::tuple<int, std::string, std::string> SlotSpecification;
	private:
	    std::regex m_matchExpr; //!< Expression for pattern matching.

	public:
	    ConfigurationParser();
	    void parse(std::istream& input, Configuration& config);
	    bool parseHardwareTypeTag(
		const std::string& line, int& revision, int& freq,
		int& resolution
		);
	    FirmwareConfiguration extractFirmwareConfiguration(
		std::istream &input
		);
	    double extractClockCalibration(std::istream &input);
	    void updateClockCalibration(int type, double calibration);
	    SlotSpecification parseSlotLine(std::istream& input);    
	};

    } // end DDAS namespace
} // end DAQ namespace

#endif // CONFIGURATIONPARSER_H

/** @} */
