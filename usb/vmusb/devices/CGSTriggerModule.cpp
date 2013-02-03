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

static const char* readModeEnum[] = {
    "event", "scaler", 0
};
/*-----------------------------------------------------------------------------
 * Board access information (address modifier and register offsets).
 */
static const uint8_t amod = CVMUSBReadoutList::a24PrivData;


// Trigger control register (provisional).

static const uint32_t  TriggerEnableReg = 0x888;
static const uint16_t  TriggerEnableBit = 0x0001;

// Event latched timestamp

static const uint32_t EventTimestampHigh = 0x880;
static const uint32_t EventTimestampMed  = 0x882;
static const uint32_t EventTimestampLow  = 0x884;
static const uint32_t EventTimestampReadSeq[] = {
  EventTimestampLow,  EventTimestampMed, EventTimestampHigh  
};

// Manually latched timestamp (provisional):

static const uint32_t LatchTimestampReg = 0x000;
static const uint32_t LatchTimestampBit = 0x0001;

static const uint32_t LatchedTimestampHigh = 0x890;
static const uint32_t LatchedTimestampMed  = 0x892;
static const uint32_t LatchedTimestampLow  = 0x894;
static const uint32_t LatchedTimestampReadSeq[] = {
    LatchedTimestampLow, LatchedTimestampMed, LatchedTimestampHigh  
};

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
 */
void
CGSTriggerModule::onAttach(CReadoutModule& config)
{
    m_pConfiguration = &config;         // Save for Initalize and addReadoutList
    
    config.addIntegerParameter("-base", 0, baseLimit, 0);
    config.addBooleanParameter("-passtrigger");
    config.addEnumParameter("-readmode", readModeEnum, "event");
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
    bool     passTrigger = m_pConfiguration->getBoolParameter("-passtrigger");
    
    // Get the current value of the trigger control register:
    // and compute the new value:
    
    uint16_t triggerReg;
    int status = controller.vmeRead16(
        baseAddress + TriggerEnableReg, amod, &triggerReg);
    if (status != 0) {
        throw std::string("GSTriggerModule::Initialize - failed to read trigger control register");
    }
    if (passTrigger) {
      triggerReg |= TriggerEnableBit;
    } else {
        triggerReg &= (~TriggerEnableBit);
    }
    status = controller.vmeWrite16(
        baseAddress + TriggerEnableReg, amod, triggerReg);
    if (status != 0) {
        throw std::string("GSTriggerModule::Initialize - failed to write trigger control register");
    }
    
    
}
/**
 * addReadoutList
 *
 * Add commands to the module readout list.  What is added depends
 * on the state of the -readmode configuration parameter.
 * See the header file for more information about this parameter.
 *
 * @param list - Reference to a CVMUSBReadoutList into which we get to add
 *               operations.
 */
void
CGSTriggerModule::addReadoutList(CVMUSBReadoutList& list)
{
    uint32_t    base = m_pConfiguration->getUnsignedParameter("-base");
    std::string mode = m_pConfiguration->cget("-readmode");
    
    const uint32_t* readSequence(0);   // Sequence of three offsets with 48 bit timestamp
    
    if (mode == "scaler") {
        list.addWrite16(base + LatchTimestampReg, amod, LatchTimestampBit);
        readSequence  = LatchedTimestampReadSeq;
        
    } else if (mode == "event") {
        readSequence = EventTimestampReadSeq;
        
    } else {
        std::string msg = "CGSTriggerModule:: addReadoutList -readmode is invalid: ";
        msg            += mode;
        throw std::string(msg);
    }
    // At this time, readSequence points to three offsets to read that
    // give a 48 bit timestamp.
    
    for (int i = 0; i < 3; i++) {
        list.addRead16(base + *readSequence++, amod);
    }
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
