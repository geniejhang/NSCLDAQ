/******************************************************************************
#
# Via Vetraia, 11 - 55049 - Viareggio ITALY
# +390594388398 - www.caen.it
#
#*****************************************************************************/

/**
 * @file CGSTriggerModule.h
 * @brief Header for the module class that manages the GammaSphere trigger
 *        module.
 * @author Ron Fox (ron@caentech.com)
 */

#ifndef _CGSTRIGGERMODULE_H
#define _CGSTRIGGERMODULE_H

// Include and forward definition boilerplate for ReadoutHardware classes:
/**
* @note TODO: why not encapsulate all of this into a single header named
*       CReadoutHardwareDefs.h
*/

#ifndef __CREADOUTHARDWARE_H
#include "CReadoutHardware.h"
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif


// Forward class definitions:

class CReadoutModule;
class CVMUSB;
class CVMUSBReadoutList;

/**
 * @class CGSTriggerModule
 *
 * This class manages the gammaSphere trigger module.  It is intended to be
 * used by the ChicoII interface with Digital GammaSphere.
 * If you want time-stamped scaler data you'll need to make two of these modules,
 * identically configured but with one having -readmode set to event that's put
 * in the event stack and the other having -readmode scaler which is put in the
 * scaler stack.
 *
 * The module supports the following configuration options:
 *
 *  * -base   - Configures the moodule base address.  This must match the module
 *              DIP switch settings and is an A24 address (range 0 -0x00ffffff).
 *  * -chicodelay - Length of chico trigger delay.
 *  * -window     - Length of matching window.
 *  * -chicolatch - True chico triggers latch timstamp.
 *  * -triggersel - nim | ecl selects source of trigger
 */

class CGSTriggerModule : public CReadoutHardware {
    // Private data:
private:
  CReadoutModule*     m_pConfiguration;      //!< Pointer to configuration object.
    
    // Canonical operations
public:
    CGSTriggerModule();
    virtual ~CGSTriggerModule();
    
    // Forbidden canonicals (unimplemented) but defined to flat acccidental
    // usage:
    
private:
    CGSTriggerModule(const CGSTriggerModule&);
    CGSTriggerModule& operator=(const CGSTriggerModule&);
    int operator==(const CGSTriggerModule&) const;
    int operator!=(const CGSTriggerModule&) const;
    
    // CReadoutHardware interface (implemented to distinguish this from the
    // base class):
    
public:
    virtual void onAttach(CReadoutModule& configuration);
    virtual void Initialize(CVMUSB& controller);
    virtual void addReadoutList(CVMUSBReadoutList& list);
    virtual CReadoutHardware* clone() const;
    
    
};

#endif

                                                                              
