///////////////////////////////////////////////////////////
//  CCAENV977.cpp
//  Implementation of the Class CCAENV977
//  Created on:      07-Jun-2005 04:42:54 PM
//  Original author: Ron Fox
///////////////////////////////////////////////////////////

#include <config.h>
#include "CCAENV977.h"
#include "VmeModule.h"
#include <string>

#ifdef __HAVE_STD_NAMESPACE
using namespace std;
#endif

#define CHECK_ALL
#define _DEBUG
#include <DesignByContract.h>

using namespace DesignByContract;


// The following are word offsets into the device:

static const UInt_t INPUT_SET(0);
static const UInt_t INPUT_MASK(1);
static const UInt_t INPUT_READ(2);
static const UInt_t SINGLEHIT_READ(3);
static const UInt_t MULTIHIT_READ(4);
static const UInt_t OUTPUT_SET(5);
static const UInt_t OUTPUT_MASK(6);
static const UInt_t INTERRUPT_MASK(7);
static const UInt_t CLEAR_OUTPUT(8);
static const UInt_t SINGLEHIT_RDCLEAR(11);
static const UInt_t MULTIHIT_RDCLEAR(12);
static const UInt_t TEST_CONTROL(13);
static const UInt_t IPL(16);
static const UInt_t INTERRUPT_ID(17);
static const UInt_t SERIAL(18);
static const UInt_t FIRMWARE_REV(19);
static const UInt_t CONTROL_REGISTER(20);
static const UInt_t DUMMY_REGISTER(21);
static const UInt_t SOFTWARE_RESET(23);


// Multi bit masks.

static const UShort_t validTestBits =  (CCAENV977::test_Clear   |
					CCAENV977::test_Mask    |
					CCAENV977::test_OrMask  |
					CCAENV977::test_IrqMask |
					CCAENV977::test_Read);

  static const UShort_t validControlBits = (CCAENV977::control_Pattern  |
					    CCAENV977::control_gateMask |
					    CCAENV977::control_OrMask);

/*!
   Construct a CAENV977 I/O register.
   @param base
        Base address of the module in the rotary switches
   @param crate
        Crate in which the module is installed (defaults to 0).
*/
CCAENV977::CCAENV977(ULong_t lBase, UShort_t nCrate) :
  m_Module(*(new CVmeModule(CVmeModule::a32d32, lBase, 0x100, nCrate)))
{

}


/*!
   The destructor must delete the vme module.
*/
CCAENV977::~CCAENV977()
{
  delete &m_Module;
}
/*!
    Copy construction:
*/
CCAENV977::CCAENV977(const CCAENV977& rhs) :
  m_Module(*(new CVmeModule(rhs.m_Module)))
{
}
/*!
  Assignment:
 */
CCAENV977&
CCAENV977::operator=(const CCAENV977& rhs)
{
  if(this != &rhs) {
    m_Module = rhs.m_Module;
  }
  return *this;
}
/*!
   Equality comparison
*/
int
CCAENV977::operator==(const CCAENV977& rhs) const
{
  return m_Module == rhs.m_Module;
}
/*!
   Inequality 
*/
int
CCAENV977::operator!=(const CCAENV977& rhs) const
{
  return !(*this == rhs);
}

/// 'useful' operations.

/**
 * Read the input set register.
 */
UShort_t 
CCAENV977::inputSet()
{
  return m_Module.peekw(INPUT_SET);
}


/**
 * Write the input set register.
 * @param value    New value for the
 * 
 */
void 
CCAENV977::inputSet(UShort_t value)
{
  m_Module.pokew(value, INPUT_SET);
}


/**
 * Read the input mask register.
 */
UShort_t 
CCAENV977::inputMask()
{
  return m_Module.peekw(INPUT_MASK);
}


/**
 * write the input mask register.
 * @param mask    New input mask value.
 * 
 */
void 
CCAENV977::inputMask(UShort_t mask)
{
  m_Module.pokew(mask, INPUT_MASK);
}


/**
 * Read the input read register (which reflects the value of the instantaneous
 * inputs to the module).
 * 
 */
UShort_t 
CCAENV977::inputRead()
{
  return m_Module.peekw(INPUT_READ);
}


/**
 * Read the single hit read register.  
 */
UShort_t 
CCAENV977::singleHitRead()
{
  return m_Module.peekw(SINGLEHIT_READ);
}


/**
 * Read the multihit read register.
 */
UShort_t 
CCAENV977::multihitRead()
{
  return m_Module.peekw(MULTIHIT_READ);
}


/**
 * Read the output set register.
 */
UShort_t 
CCAENV977::outputSet()
{
  return m_Module.peekw(OUTPUT_SET);
}


/**
 * Write the output set register.
 * @param pattern    New output pattern.
 * 
 */
void 
CCAENV977::outputSet(UShort_t pattern)
{
  m_Module.pokew(pattern, OUTPUT_SET);
}


/**
 * Read the output mask register.
 */
UShort_t 
CCAENV977::outputMask()
{
  return m_Module.peekw(OUTPUT_MASK);
}


/**
 * Write the output mask register.
 * @param mask    New output mask value.
 * 
 */
void 
CCAENV977::outputMask(UShort_t mask)
{
  m_Module.pokew(mask, OUTPUT_MASK);
}


/**
 * Read the interrupt mask register.
 */
UShort_t 
CCAENV977::interruptMask()
{
  return m_Module.peekw(INTERRUPT_MASK);
}


/**
 * Write the interrupt mask register.
 * @param mask    New interrupt mask value.
 * 
 */
void 
CCAENV977::interruptMask(UShort_t mask)
{
  m_Module.pokew(mask, INTERRUPT_MASK);
}


/**
 * Clears all output flipflops.
 */
void 
CCAENV977::outputClear()
{
  m_Module.peekw(CLEAR_OUTPUT);
}


/**
 * Read the single hit register and clear it.
 */
UShort_t 
CCAENV977::singleHitReadAndClear()
{
  
  return m_Module.peekw(SINGLEHIT_RDCLEAR);
}


/**
 * Read and clear the multi hit read register.
 */
UShort_t 
CCAENV977::multiHitReadAndClear()
{

  return m_Module.peekw(MULTIHIT_RDCLEAR);

}


/**
 * Read the test control register.
 */

UShort_t 
CCAENV977::testControlRegister()
{

  return (m_Module.peekw(TEST_CONTROL) & validTestBits);
}


/**
 * Write the test control register.
 * @param mask    New mask of bits to write to the Test Control register.  If this
 * has invalid bits set, a contract violation is thrown.
 * 
 */
void 
CCAENV977::testControlRegister(UShort_t mask)
{

  REQUIRE(mask == (mask & validTestBits), "Invalid bits in test mask");
  m_Module.pokew(mask, TEST_CONTROL);

}



/**
 * Read the serial number register.
 */
UShort_t 
CCAENV977::serialNumber()
{

  return m_Module.peekw(SERIAL);

}


/**
 * Read the firmware revision level.
 */
UShort_t 
CCAENV977::firmwareLevel()
{

  return m_Module.peekw(FIRMWARE_REV);

}


/**
 * Write the control register.
 * @param mask    new mask of control register bits.  If extraneous bits are set,
 * a contract violation is thrown.
 * 
 */
void 
CCAENV977::controlRegister(UShort_t mask)
{

  REQUIRE(string("Invalid bits in control Register mask"), (mask & validControlBits) == mask);
  m_Module.pokew(mask, CONTROL_REGISTER);

}


/**
 * Read the control register.
 */
UShort_t 
CCAENV977::controlRegister()
{

  return (m_Module.peekw(CONTROL_REGISTER) & validControlBits);
}


/**
 * Resets the module to default conditions.
 */
void 
CCAENV977::Reset()
{

  m_Module.pokew(0, SOFTWARE_RESET);

}


