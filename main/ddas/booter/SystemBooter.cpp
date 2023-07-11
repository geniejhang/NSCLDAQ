/**
 * @file SystemBooter.cpp
 * @brief Implementation of the system booter class for DDAS.
 */

#include "SystemBooter.h"

#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>

#include <config.h>
#include <config_pixie16api.h>
#include <Configuration.h>

using namespace std;

/*!
 * \brief Constructor.
 *
 * Enables verbose output by default.
 */
DAQ::DDAS::SystemBooter::SystemBooter() :
    m_verbose(true),
    m_offlineMode(0)
{}

/*!
 * \brief Boot the entire system.
 *
 * Provided a configuration, all modules will be booted. The configuration 
 * contains the firmware files for each hardware type, the slot map, and the 
 * number of modules. During the course of booting, the hardware will be 
 * queried as well to determine the the serial number, revision, ADC 
 * frequency, and resolution. The revision, adc frequency, and resolution 
 * will all be parsed and the information will be stored in the configuration
 * as a HardwareRegistry::HardwareType.
 *
 * \param config  A configuration describing the system.
 * \param type    Style of boot.
 *
 * \throws std::runtime_error If Pixie16InitSystem() call returns an error.
 * \throws std::runtime_error If populateHardwareMap() throws.
 * \throws std::runtime_error If bootModuleByIndex() throws.
 */
void DAQ::DDAS::SystemBooter::boot(Configuration &config, BootType type)
{
    cout << "------------------------\n";
    cout << "Initializing PXI access... \n";
    cout.flush();

    int NumModules = config.getNumberOfModules();
    int retval = Pixie16InitSystem(
	NumModules, config.getSlotMap().data(), m_offlineMode
	);
    if(retval < 0) {
	std::stringstream errmsg;
	errmsg << "SystemBooter::boot() Failure. Pixie16InitSystem returned = "
	       << retval << ".";
	throw std::runtime_error(errmsg.str());
    } else {
	cout << "System initialized successfully. " << endl;
    }

    // Give the system some time to settle after initialization.
    usleep(1000);

    populateHardwareMap(config);

    for (int index=0; index<NumModules; ++index) {
	bootModuleByIndex(index, config, type);
    }

    if (m_verbose) {
	cout << "All modules ok " << endl;
    }

}


/*!
 * \brief Read and store hardware info from each of the modules in the system.
 *
 * To retrieve information about all of the modules in the system, 
 * Pixie16ReadModuleInfo is called for each module index. The resulting 
 * revision number, ADC bits, and ADC frequency is printed (if verbose output
 * enabled) and the hardware mapping is stored in the configuration that was
 * passed in.
 * 
 * \param config  The system configuration.
 * 
 * \throws std::runtime_error  If Pixie16ReadModuleInfo returns error code.
 */
void DAQ::DDAS::SystemBooter::populateHardwareMap(Configuration &config)
{
    unsigned short ModRev;
    unsigned int   ModSerNum;
    unsigned short ModADCBits;
    unsigned short ModADCMSPS;
    unsigned short nchannels;

    int NumModules = config.getNumberOfModules();
    std::vector<int> hdwrMapping(NumModules);

    for(unsigned short k=0; k<NumModules; k++) {
	int retval = Pixie16ReadModuleInfo(
	    k, &ModRev, &ModSerNum, &ModADCBits, &ModADCMSPS
	    );
	if (retval < 0)
	{
	    std::stringstream errmsg;
	    errmsg << "SystemBooter::boot() Reading hardware variant ";
	    errmsg << "information (i.e. Pixie16ReadModuleInfo()) failed ";
	    errmsg << "for module " << k << " with retval = " << retval;
	    throw std::runtime_error(errmsg.str());
	} else {
	    if (m_verbose) {
		logModuleInfo(k, ModRev, ModSerNum, ModADCBits, ModADCMSPS);
	    }
	    auto type = HardwareRegistry::computeHardwareType(
		ModRev, ModADCMSPS, ModADCBits
		);
	    hdwrMapping[k] = type;
	}
    }

    // Store the hardware map in the configuration so other components of the
    // program can understand more about the hardware being used.
    config.setHardwareMap(hdwrMapping);
}


/*!
 * \brief Print out some basic information regarding the module
 *
 * \todo ASC 7/7/23: Lots of arguments to this function. Can we pack info 
 * into a struct and pass it around that way instead to clean up these 
 * signatures/calls?
 *
 * \param modIndex    Index of the module in the system.
 * \param ModRev      Hardware revision.
 * \param ModSerNum   Serial number.
 * \param ModADCBits  ADC resolution (number of bits).
 * \param ModADCMSPS  ADC frequency (MSPS).
 */
void DAQ::DDAS::SystemBooter::logModuleInfo(
    int modIndex, unsigned short ModRev, unsigned short ModSerNum,
    unsigned short ModADCBits, unsigned short ModADCMSPS
    )
{
    std::cout << "Found Pixie-16 module #" << modIndex;
    std::cout << ", Rev = " << ModRev;
    std::cout << ", S/N = " << ModSerNum << ", Bits = " << ModADCBits;
    std::cout << ", MSPS = " << ModADCMSPS;
    std::cout << endl;
}


/*!
 * \brief Boot a single module
 *
 * The system is booted into a usable state. The mechanics of booting involve
 * either loading firmware and settings or just settings, depending on the type
 * parameter that was passed as a second argument to the method. If the user
 * chooses to boot with a firmware load, the firmware files stored in the
 * configuration associated with the hardware will be used. The settings
 * file that will be used in any boot type, will be the path stored in the
 * configuration.

 * \param modIndex  Index of the module in the system.
 * \param m_config  The system configuration.
 * \param type      Boot style (load firmware or settings only).
 *
 * \throws std::runtime_error If hardware type is unknown.
 * \throws std::runtime_error If Pixie16BootModule returns an error code.
 *
 * \todo Check that the firmware file paths are less than 256 characters in 
 * length.
 */
void
DAQ::DDAS::SystemBooter::bootModuleByIndex(
    int modIndex, Configuration& m_config, BootType type
    )
{

    const size_t FILENAME_STR_MAXLEN = 256;
    char Pixie16_Com_FPGA_File[FILENAME_STR_MAXLEN];
    char Pixie16_SP_FPGA_File[FILENAME_STR_MAXLEN];
    char Pixie16_DSP_Code_File[FILENAME_STR_MAXLEN];
    char Pixie16_DSP_Var_File[FILENAME_STR_MAXLEN];
    char Pixie16_Trig_FPGA_File[FILENAME_STR_MAXLEN];
    char DSPParFile[FILENAME_STR_MAXLEN];

    // Select firmware and dsp files based on hardware variant
    std::vector<int> hdwrMap = m_config.getHardwareMap();
    if (hdwrMap[modIndex] == HardwareRegistry::Unknown) {
	std::stringstream errmsg;
	errmsg <<"Cannot boot module "<< modIndex
	       << ". Hardware type not recognized" << std::endl;
	throw std::runtime_error(errmsg.str());
    }

    // Because the Pixie16BootModule takes char* strings, we have to copy our
    // beautiful std::strings into the character arrays. Note that there is
    // no check at the moment to ensure that the firmware file paths are no
    // more than 256 characters. daqdev/DDAS#106 - modified to get the per
    // module firmware configuration which will default to the global config
    // if not specified.
    
    FirmwareConfiguration fwConfig = m_config.getModuleFirmwareConfiguration(
	hdwrMap[modIndex], modIndex
	);
    strcpy(Pixie16_Com_FPGA_File, fwConfig.s_ComFPGAConfigFile.c_str());
    strcpy(Pixie16_SP_FPGA_File,  fwConfig.s_SPFPGAConfigFile.c_str());
    strcpy(Pixie16_DSP_Code_File, fwConfig.s_DSPCodeFile.c_str());
    strcpy(Pixie16_DSP_Var_File,  fwConfig.s_DSPVarFile.c_str());

    // daqdev/DDAS#106 - modified as above to get a per module setfile:
    
    strcpy(DSPParFile, m_config.getSettingsFilePath(modIndex).c_str());

    if (m_verbose) {
	if (type == FullBoot) {
	    cout << "\nBooting Pixie-16 module #" << modIndex << endl;
	    cout << "\tComFPGAConfigFile:  " << Pixie16_Com_FPGA_File << endl;
	    cout << "\tSPFPGAConfigFile:   " << Pixie16_SP_FPGA_File << endl;
	    cout << "\tDSPCodeFile:        " << Pixie16_DSP_Code_File << endl;
	    cout << "\tDSPVarFile:         " << Pixie16_DSP_Var_File << endl;
	    cout << "\tDSPParFile:         " << DSPParFile << endl;
	    cout << "------------------------------------------------------";
	    cout << "\n\n";
	} else {
	    cout << "\nEstablishing communication parameters with module #"
		 << modIndex << std::endl;
	    std::cout << "\tSkipping firmware load." << std::endl;
	}
    }

    // Arguments are:
    // 0) Name of communications FPGA config. file
    // 1) Name of signal processing FPGA config. file
    // 2) Placeholder name of trigger FPGA configuration file
    // 3) Name of executable code file for digital signal processor (DSP)
    // 4) Name of DSP parameter file
    // 5) Name of DSP variable names file
    // 6) Pixie module number
    // 7) Fast boot pattern bitmask
    int retval = Pixie16BootModule(
	Pixie16_Com_FPGA_File, Pixie16_SP_FPGA_File, Pixie16_Trig_FPGA_File,
	Pixie16_DSP_Code_File, DSPParFile, Pixie16_DSP_Var_File,
	modIndex, computeBootMask(type)
	);
    
    if(retval != 0) {
	std::stringstream errmsg;
	errmsg << "Failed for module " << modIndex << " error code "
	       << retval << " !";
	throw std::runtime_error(errmsg.str());
    }
}

/*!
 * \brief Enable or disable verbose output
 *
 * By default, the output verbosity setting is enabled. If it is disabled,
 * there will be no output printed to the terminal.
 *
 * \param enable  Enables output messages if true.
 */
void DAQ::DDAS::SystemBooter::setVerbose(bool enable)
{
    m_verbose = enable;
}

/*!
 * \brief Return the state of verbosity.
 *
 * \return bool  The state.
 */
bool DAQ::DDAS::SystemBooter::isVerbose() const
{
    return m_verbose;
}

/*!
 * \brief Enable or disable online boot
 *
 * By default, the boot mode is set to online mode (0). If it is set
 * to offline mode (1), calls to the API functions can still be tested
 * with no modules present.
 *
 * \warning Offline boot mode is only supported in XIA API 2!
 *
 * \param mode  Boot mode.
 */
void DAQ::DDAS::SystemBooter::setOfflineMode(unsigned short mode)
{
    m_offlineMode = mode;
}

/*!
 * \brief Return the boot mode of the system.
 *
 * \return bool  The boot mode.
 */
unsigned short DAQ::DDAS::SystemBooter::getOfflineMode() const
{
    return m_offlineMode;
} 

/*!
 * \brief Convert BootType enumeration to usable boot mask.
 *
 * The bootmask is ultimately used in the Pixie16BootModule function.
 *
 * \param type  Either BootType::FullBoot or BootType::SettingsOnly.
 *
 * \return unsigned int  0x7f for FullBoot, 0x70 for SettingsOnly.
 */
unsigned int DAQ::DDAS::SystemBooter::computeBootMask(BootType type)
{
    if (type == FullBoot) {
	return 0x7f;
    } else {
	return 0x70;
    }
}
