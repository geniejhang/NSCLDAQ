/**
 * @file Configuration.cpp
 * @brief Implementation of the system storage configuration.
 */

#include "Configuration.h"

#include <iostream>
#include <fstream>

#include "FirmwareVersionFileParser.h"
#include "ConfigurationParser.h"
#include "ModEvtFileParser.h"

namespace DAQ {
    namespace DDAS {

/*!
 * \brief Set the crate id for the module.
 *
 * Note that this is not currently used for anything.
 *
 * \param id  The id to assign.
 */
	void
	Configuration::setCrateId(int id)
	{
	    m_crateId = id;
	}

/*!
 * \brief Return the crate id value.
 *
 * \return int  The crate id.
 */
	int
	Configuration::getCrateId() const
	{
	    return m_crateId;
	}

/*!
 * \brief Set the number of modules in the crate.
 *
 * This resizes the vectors storing the slot map, module event lengths, and
 * hardware map to be consistent. The caller should call setNumberOfModules
 * prior to calling setSlotMap() or setModuleEventLengths().
 *
 * \param size  Number of modules.
 */
	void
	Configuration::setNumberOfModules(size_t size)
	{
	    m_slotMap.resize(size);
	    m_modEvtLengths.resize(size);
	    m_hardwareMap.resize(size);
	}

/*!
 * \brief Return the number of modules in the crate.
 *
 * \return size_t  The number of modules.
 */
	size_t
	Configuration::getNumberOfModules() const
	{
	    return m_slotMap.size();
	}

/*!
 * \brief Assign a new slot map.
 *
 * It is important for the caller to first call setNumberOfModules()
 * before calling this to avoid an exception being thrown. To avoid
 * weird configurations, this ensures that the length of the slot map
 * is the same as the module event length vector at all times. If
 * the user has not set the number of modules previously, this cannot
 * be gauranteed and the method will almost always throw.
 *
 * \code
 *  Configuration config;
 *  config.setNumberOfModules(2);
 *  config.setSlotMap({2, 3});
 * \endcode
 *
 * \param map  The slots that are occupied.
 *
 * \throws std::runtime_error  When length of argument is different than 
 *     length of stored modevtlen vector.
 */
	void
	Configuration::setSlotMap(const std::vector<unsigned short> &map)
	{
	    if (map.size() != m_modEvtLengths.size()) {
		std::string errmsg;
		errmsg += "Configuration::setSlotMap(): ";
		errmsg += "Inconsistent data for module evt lengths and ";
		errmsg += "slot mapping. Set number of modules first using ";
		errmsg += "Configuration::setNumberOfModules().";
		throw std::runtime_error(errmsg);
	    }

	    m_slotMap = map;
	}

/*!
 * \brief Return the vector containing the filled slots.
 *
 * \return std::vector<unsigned short>  The vector containing the slots that 
 *   are filled.
 */
	std::vector<unsigned short>
	Configuration::getSlotMap() const
	{
	    return m_slotMap;
	}

/*!
 * \brief Set the path to the .set file.
 *
 * \param path  The path to the settings file.
 */
	void
	Configuration::setSettingsFilePath(const std::string &path)
	{
	    m_settingsFilePath = path;
	}

/*!
 * \brief Return the path to the .set file.
 * 
 * \return std::string  The settings file path.
 */
	std::string Configuration::getSettingsFilePath() const
	{
	    return m_settingsFilePath;
	}

/*!
 * \brief Set the firmware configuration for a hardware type
 * 
 * Any previous FirmwareConfiguration stored will be replaced by the new
 * configuration. If there is no previous configuration for the hardware type
 * it will be added.
 *
 * \param specifier  The hardware type.
 * \param config     The new configuration.
 */
	void
	Configuration::setFirmwareConfiguration(
	    int specifier, const FirmwareConfiguration &config
	    )
	{
	    m_fwMap[specifier] = config;
	}


/*!
 * \brief Retrieve the current firmware specifier for a particular 
 * hardware type.
 *
 * \param hdwrType  The hardware specifier associated with the firmware 
 *   configuration.
 *
 * \return FirmwareConfiguration&  The firmware configuration associated with 
 *   the hdwrType.
 *
 * \throws std::runtime_error  If no firmware configuration exists for the 
 *   provided hdwrType.
 */
	FirmwareConfiguration& Configuration::getFirmwareConfiguration(
	    int hdwrType
	    )
	{
	    auto pSpec = m_fwMap.find(hdwrType);
	    if (pSpec == m_fwMap.end()) {
		std::string errmsg = "Unable to locate firmware ";
		errmsg += "configuration for firmware specifier";
		throw std::runtime_error(errmsg);
	    }

	    return pSpec->second;
	}

/*!
 * \brief Set the lengths of events for each module
 *
 * It is necessary that the caller has previously invoked setNumberOfModules()
 * before calling this. The logic of this method aims to keep the slot map
 * and module event length vectors the same length. Without invoking
 * setNumberOfModules() this is most likely not going to be the case.
 *
 * \param lengths  The module event lengths.
 *
 * \throws std::runtime_error if size of lengths does not match size of 
 *   stored slot map.
 */
	void
	Configuration::setModuleEventLengths(
	    const std::vector<int> &lengths
	    )
	{
	    if (lengths.size() != m_slotMap.size()) {
		std::string errmsg;
		errmsg += "Configuration::setModuleEventLengths() ";
		errmsg += "Inconsistent data for module evt lengths and ";
		errmsg += "slot mapping. Set number of modules first using ";
		errmsg += "Configuration::setNumberOfModules().";
		throw std::runtime_error(errmsg);
	    }

	    m_modEvtLengths = lengths;
	}

/*!
 * \brief Return a copy of the module event length vector.
 *
 * \return std::vector<int>  Copy of module event lengths vector.
 */
	std::vector<int>
	Configuration::getModuleEventLengths() const
	{
	    return m_modEvtLengths;
	}

/*!
 * \brief Set the hardware map for each module.
 *
 * It is necessary that the caller has previously invoked setNumberOfModules()
 * before calling this. The logic of this method aims to keep the slot map
 * and module event length vectors the same length. Without invoking
 * setNumberOfModules() this is most likely not going to be the case.
 *
 * \param map  The hardware map.
 *
 * \throws std::runtime_error if size of lengths does not match size of 
 *   stored slot map.
 */
	void
	Configuration::setHardwareMap(const std::vector<int> &map)
	{
	    if (map.size() != m_slotMap.size()) {
		std::string errmsg;
		errmsg += "Configuration::setModuleEventLengths() ";
		errmsg += "Inconsistent data for hardware mapping and ";
		errmsg += "slot mapping. Set number of modules first using ";
		errmsg += "Configuration::setNumberOfModules().";
		throw std::runtime_error(errmsg);
	    }

	    m_hardwareMap = map;
	}

/*!
 * \brief Return a copy of the hardware map vector.
 *
 * \return std::vector<int>  Copy of hardware map vector.
 */
	std::vector<int>
	Configuration::getHardwareMap() const
	{
	    return m_hardwareMap;
	}
	

/*!
 * \brief Print brief line of information for cfgPixie16.txt
 *
 * Prints out a message similar to:
 * "Crate number 1: 2 modules, in slots:2 3 DSPParFile: /path/to/file.set"
 *
 * \param stream  The ostream to write to.
 */
	void
	Configuration::print(std::ostream &stream)
	{
	    stream << "Crate number " << m_crateId;
	    stream << ": " << m_slotMap.size() << " modules, in slots:";

	    for(auto& slot : m_slotMap){
		stream << slot << " ";
	    }
	    stream << "DSPParFile: " << m_settingsFilePath;


	}


/**
 * \brief Generate a configuration from a firmware version file and 
 * cfgPixie16.txt. 
 *
 * std::move() ensures correct ownership of the returned pointer, 
 * though we _may_ be able to take advantage of some copy elision here.
 * 
 * \param fwVsnPath     Path to the firmware version file.
 * \param cfgPixiePath  Path to cfgPixie16.txt.
 *
 * \return std::unique_ptr<Configuration>  Pointer to Configuration.
 */
	std::unique_ptr<Configuration>
	Configuration::generate(
	    const std::string &fwVsnPath, const std::string &cfgPixiePath
	    )
	{
	    std::unique_ptr<Configuration> pConfig(new Configuration);

	    FirmwareVersionFileParser fwFileParser;
	    ConfigurationParser       configParser;

	    std::ifstream input(fwVsnPath.c_str(), std::ios::in);

	    if(input.fail()) {
		std::string errmsg("Configuration::generate() ");
		errmsg += "Failed to open the firmware version file: ";
		errmsg += fwVsnPath;
		throw std::runtime_error(errmsg);
	    }

	    fwFileParser.parse(input, pConfig->m_fwMap);

	    input.close();
	    input.clear();

	    input.open(cfgPixiePath.c_str(), std::ios::in);

	    if(input.fail()){
		std::string errmsg("Configuration::generate() ");
		errmsg += "Failed to open the system configuration file : ";
		errmsg += cfgPixiePath;
		throw std::runtime_error(errmsg);
	    }

	    configParser.parse(input, *pConfig);

	    return std::move(pConfig);

	}

/**
 * \brief Generate a configuration from a firmware version file and 
 * cfgPixie16.txt. 
 *
 * std::move() ensures correct ownership of the returned pointer, 
 * though we _may_ be able to take advantage of some copy elision here.
 * 
 * \param fwVsnPath      Path to the firmware version file.
 * \param cfgPixiePath   Path to cfgPixie16.txt.
 * \param modEvtLenPath  Path to the modevtlen.txt file.
 *
 * \return std::unique_ptr<Configuration>  Pointer to Configuration.
 */
	std::unique_ptr<Configuration>
	Configuration::generate(
	    const std::string &fwVsnPath, const std::string &cfgPixiePath,
	    const std::string &modEvtLenPath
	    )
	{
	    ModEvtFileParser modEvtParser;

	    std::unique_ptr<Configuration> pConfig = generate(fwVsnPath, cfgPixiePath);

	    int moduleCount = pConfig->getNumberOfModules();

	    // read a configration file to tell Pixie16 how big an event is in
	    // a particular module.  Within one module all channels MUST be set to
	    // the same event length

	    std::ifstream modevt;
	    modevt.open(modEvtLenPath.c_str(), std::ios::in);

	    if(!modevt.is_open()){
		std::string errmsg("Configuration::generate() ");
		errmsg += "Failed to open the module event length configuration file : ";
		errmsg += modEvtLenPath;
		throw std::runtime_error(errmsg);
	    }

	    modEvtParser.parse(modevt, *pConfig);

	    return std::move(pConfig);
	}

/*----------------------------------------------------------------------
 * daqdev/DDAS#106 - additions for per module setfiles and firmware maps.
 */

/**
 * \brief Sets a per module DSP Settings (.set) file.
 *
 * \param modNum  Module number.
 * \param path    Filename path. This should have been checked for 
 *   readability by the caller.
 */
	void
	Configuration::setModuleSettingsFilePath(
	    int modNum, const std::string& path
	    )
	{
	    m_moduleSetFileMap[modNum] = path;
	}

/**
 * \brief Returns a settings file associated with a specific module.
 *
 * If there's a per module set file it's returned otherwise return the 
 * default settings file.
 *
 * \param modnum  Module number.
 *
 * \return std::string  The full path to the settings file.
 */
	std::string
	Configuration::getSettingsFilePath(int modnum)
	{
	    if (m_moduleSetFileMap.count(modnum) > 0) {
		return m_moduleSetFileMap[modnum];
	    } else {
		return m_settingsFilePath;
	    }
	}

/**
 *  \brief Sets a firmware map specific to a module.
 *
 *  \param module   Module index.
 *  \param mapping  Firmware mapping for that module.
 *
 *  \note  An existing map is ovewritten.
 */
	void
	Configuration::setModuleFirmwareMap(
	    int module, const FirmwareMap& mapping
	    )
	{
	    m_moduleFirmwareMaps[module] = mapping;
	}

/**
 * \brief Get the module firmware configuration information.
 *
 * If the module has a firmware map, returns the firmware configuration to 
 * load into the module. If not the default firmware configuration is returned.
 *
 * \param hwType  The hardware type detected in the module.
 * \param modnum  Module number.
 *
 * \return FirmwareConfiguration& - the firmware configuraiton
 * 
 * \note  It is an error to have a firmware configuration map file but
 *   not to have a configuration for the hardware type.
 */
	FirmwareConfiguration&
	Configuration::getModuleFirmwareConfiguration(
	    int hwType, int modnum
	    )
	{
	    if (m_moduleFirmwareMaps.count(modnum) > 0) {   // There's a map.
		FirmwareMap& mapping = m_moduleFirmwareMaps[modnum];
		if (mapping.count(hwType) > 0) {
		    return mapping[hwType];
		} else {
		    std::string errmsg = "Unable to locate firmware configuration ";
		    errmsg += "for firmware specifier in per module map";
		    throw std::runtime_error(errmsg);
		}
	    } else {
		return getFirmwareConfiguration(hwType);
	    }
	}

    } // namespace DDAS
} // namespace DAQ
