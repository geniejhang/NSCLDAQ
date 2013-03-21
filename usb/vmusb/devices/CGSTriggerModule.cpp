/******************************************************************************
#
# Via Vetraia, 11 - 55049 - Viareggio ITALY
# +390594388398 - www.caen.it
#
#*****************************************************************************/

/**
 * @file CGSTriggerModule.cpp
 * @brief Implementation of the module class that manages the GammaSphere trigger
 *        module.
 * @author Ron Fox (ron@caentech.com)
 */
#include "CGSTriggerModule.h"
#include <CReadoutModule.h>
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>

/*-----------------------------------------------------------------------------
 * Constraints on configuration parameters:
 */

static const uint32_t baseLimit = 0xfff000;  // not 0xffffff to allow space for the board.

/*-----------------------------------------------------------------------------
 * Board access information (address modifier and register offsets).
 */
static const uint8_t amod = CVMUSBReadoutList::a24PrivData;
static const char* triggerSelections[] = {
  "nim", "ecl", 0
};

/* Register offsets we care about; and bits in that register */

static const uint32_t HwStatus(0x20);
static const uint16_t SERDES_LOCKED(0x40); // serdes locked.
static const uint16_t SERDES_SM_LOCKED(0x8000); // Statemachine locked in.
static const uint16_t SERDES_SM_LOSTLOCK(0x4000); // State machine lost lock.


static const uint32_t PulsedControl(0x40c);
static const uint16_t RESET_RX_MACH(1);
static const uint16_t RESET_LOST_CLOCK(0x4);


static const uint32_t GatingControl(0x702);
static const uint16_t CHICO_TRIG_ENABLE(1);
static const uint16_t ECL_TRIG_SEL(0x8000);

static const uint32_t TimestampHigh(0x70a);
static const uint32_t TimestampMid(0x70c);
static const uint32_t TimestampLow(0x70e);

static const uint32_t CoincidenceDelay(0x710);
static const uint32_t CoincidenceWidth(0x712);

static const uint32_t SdConfig(0x848);
static const uint16_t SdConfigInit(0x8073); // Value to write to lock in serdes clock
static const uint16_t SdRun(0xf3);	    // set clock to serdes and do hard data checking.


/*-----------------------------------------------------------------------------
 ** Implementation of canonical methods:
 */

/**
 * CGSTriggerModule
 *
 * Constructs a trigger moduel object.  The only thing we need to do
 * is initialize our configuration pointer to null.  The configuration object
 * is passed to us in onAttach below.
 */
CGSTriggerModule::CGSTriggerModule() : m_pConfiguration(0)
{}
/**
 * ~CGSTriggerModule
 *
 * Destructor.  The configuration is owned by our client and therefore does not
 * require destruction by us.
 */
CGSTriggerModule::~CGSTriggerModule() {}

/*-----------------------------------------------------------------------------
 ** Implementation of the CReadoutHardware interface.
 */

/**
 * onAttach
 *
 *  Called to attach a configuration object to us.  We must also define
 *  our module configuration options to that objwedt at this point.
 *  For the set of configuration options supported, see the class header file.
 *
 *  @param config - Reference to the configuration object associated with
 *                  this object.
 *
 * @note  The module supports the following configuration options:
 *  * -base   - Configures the moodule base address.  This must match the module
 *              DIP switch settings and is an A24 address (range 0 -0x00ffffff).
 *  * -chicodelay - Length of chico trigger delay.
 *  * -window     - Length of matching window.
 *  * -chicolatch - True chico triggers latch timstamp.
 *  * -triggersel - nim | ecl selects source of trigger
 */
void
CGSTriggerModule::onAttach(CReadoutModule& config)
{
    m_pConfiguration = &config;         // Save for Initalize and addReadoutList
    
    config.addIntegerParameter("-base", 0, baseLimit, 0);
    config.addIntegerParameter("-chicodelay", 0, 0xffff, 0);
    config.addIntegerParameter("-window",  0, 0xffff, 0);
    config.addBooleanParameter("-chicolatch", true);
    config.addEnumParameter("-triggersel", triggerSelections, "nim");
}

/**
 * Initialize
 *
 * Initialize the hardware in accordance with the configuration. The only thing
 * we need to do is to set or unset the trigger control register in accordance
 * with the value of the -passtrigger configuration parameter.
 *
 * @param controller - Reference to a CVMUSB object that represents the
 *                     VM-USB controller.
 */
void
CGSTriggerModule::Initialize(CVMUSB& controller)
{
    uint32_t baseAddress = m_pConfiguration->getUnsignedParameter("-base");
    uint16_t chicoDelay  = m_pConfiguration->getUnsignedParameter("-chicodelay");
    uint16_t window      = m_pConfiguration->getUnsignedParameter("-window");
    bool     chicoLatch  = m_pConfiguration->getBoolParameter("-chicolatch");
    std::string triggerSel = m_pConfiguration->cget("-triggersel");

    // Set up the state machine  if this fails throw a std::string
    // to prevent the run from starting:

    std::cerr << "Turning on MyRIAD SERDES chip\n";
    controller.vmeWrite16(baseAddress + SdConfig, amod, SdConfigInit);

    std::cerr << "Verifying SERDES lock:\n";
    uint16_t status;
    controller.vmeRead16(baseAddress + HwStatus, amod, &status);
    if ((status & SERDES_LOCKED) == 0) {
      throw std::string("SERDES did not lock check cable, try again!!!");
    }
    controller.vmeWrite16(baseAddress + SdConfig, amod , SdRun);

    std::cerr << "Got lock\nInitializing SERDES State Machine:\n";
    controller.vmeWrite16(baseAddress + PulsedControl, amod, RESET_RX_MACH);
    controller.vmeWrite16(baseAddress + PulsedControl, amod, RESET_LOST_CLOCK);
    controller.vmeRead16(baseAddress + HwStatus, amod, &status);
    if ((status & SERDES_LOCKED) == 0) {
      throw std::string("SERDES lost lock initializint state machine!!");
    }
    if ((status & SERDES_SM_LOCKED) == 0) {
      throw std::string("SERDES StateMachine not locked in after init!");
    }
    if ((status & SERDES_SM_LOSTLOCK) != 0) {
      throw std::string("SERDES Statemachine lost lock!!");
    }
    // All is copacetic with the serdes and statemachine.
    // Now we can program the device to match our configuration:

    // Delay and matching window:

    controller.vmeWrite16(baseAddress + CoincidenceDelay, amod, chicoDelay);
    controller.vmeWrite16(baseAddress + CoincidenceWidth, amod, window);

    // trigger enable and latch source:
    
    uint16_t triggerReg = 0;
    if (chicoLatch) triggerReg |= CHICO_TRIG_ENABLE;
    if (triggerSel == "ecl") triggerReg |= ECL_TRIG_SEL;

    controller.vmeWrite16(baseAddress + GatingControl, amod, triggerReg);
}
/**
 * addReadoutList
 *
 * Add commands to the module readout list.
 */
void
CGSTriggerModule::addReadoutList(CVMUSBReadoutList& list)
{
    uint32_t    base = m_pConfiguration->getUnsignedParameter("-base");
    
    // Read the 48 bits of the timestamp in little endian order:

    list.addRead16(base + TimestampLow, amod);
    list.addRead16(base + TimestampMid, amod);
    list.addRead16(base + TimestampHigh, amod);

}
/**
 * clone
 *
 *  This is a virtual copy constructor.
 *  The result is a dynamically allocated object that is identical to
 *  *this at this instant in time.
 *
 *  @return CReadoutHardware*  - Pointer to the newly created module.
 */
CReadoutHardware*
CGSTriggerModule::clone() const
{
    CGSTriggerModule* pResult = new CGSTriggerModule;
    pResult->m_pConfiguration = m_pConfiguration;   // Shallow copy.
    
    return reinterpret_cast<CReadoutHardware*>(pResult);
}
