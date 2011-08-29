/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2011

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Kole Reece, Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


/*!
 * \file CV1720.cpp implementation of the CV1720 class.
 */

// headers required:

#include <config.h>
#include "CV1720.h"
#include <VmeModule.h>
#include "CV1720internal.h"

#include <sstream>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

static uint32_t toobigdata[1024*1024/sizeof(uint32_t)];

/*!
 * Construct a module.  The main point of this is to create the
 * CVMEModule object that encapsulates the vme address window that makes
 * up the register space address window.
 * @param baseAddress - The base address of the modulea s determined by the
 *                      module rotary switches.
 * @param moduleId    - The ID of the module as set in the rotary switches.
 * @param crate       - Number of the VME crate the module is installed in (default to zero).
 */
CV1720::CV1720(uint32_t baseAddress, 
	       uint8_t  moduleId,
	       uint8_t  crate ) :
  
  m_pModule(0)
{
  // Create the module address window

  m_pModule = new CVmeModule(CVmeModule::a32d32,
			     baseAddress,
			     ADDLENGTH, // TODO: This should come from the register definition header
			     crate);

  // Reset the module, load the firmware and program the module id:
  
   swReset();	 		// TODO: Figure out if these methods will 
   swClear();			//       need delays to work correctly.
   loadFirmware();
   setModuleId(moduleId);
						
}
/**
 *  Destructor - Get rid of the vme address window we created.
 */
CV1720::~CV1720()
{
   delete m_pModule;		// Releases all resource asociated with the map.
}


/**
* Allow enable/disable channels of the digitzer.1 Enables the  channel and  0 Disables the channel
* @param enables -used to enable masking for Each channel
*/
 void CV1720::setChannelMask(uint8_t enables)
{
   throwIfBadRange(CHANENABLEMAX,"setChannelMask",enables);
   pokel(enables,CHAN_MASK);
} 
/*
 * Allow direct write access to the front panel trigger enable mask
 * @param mask
 * TODO: Leave to Kole to clean this up
 */
void
CV1720::setTriggerOutEnableMask(uint32_t mask)
{
  pokel(mask, TRIGOUT_MASK);
}

/** 
* Allows a DC offset to the be added to the input signal.When the channel  status bit 2 is set to 0  Dc offset is updated.
* @param nChannel -The channel to which the DC offset will be applied 
* @param offset  -The value of the required DC offset
*/
void CV1720::setDCOffset(unsigned nChannel, uint16_t offset)
{       
   throwIfBadChan( nChannel,"DCOffset"); 
   pokel(offset,(DC_OFFSET_BASE+(nChannel*INC_CHANNEL)));   
}



/**
* Write to config Register in bitsetmode invert the input 
* @param  invert- used to determine if to invert the Input  
*/
void CV1720::invertInput(bool invert)
{
   if (invert) {
       pokel(INVERT_SIG,CONFIG_SET); 
   }
   
   else {
       pokel(INVERT_SIG,CONFIG_CLEAR); 
   }
}

/**
* Write to config register in bitset mods .Enable Self Trigger 
* @param enable - Allow self trigger mods
*/
void CV1720::selfTrigger(bool enable)
{
   if (enable) {
       pokel(SELF_TRIGGER,CONFIG_SET); 
   }
   
   else {
       pokel(SELF_TRIGGER,CONFIG_CLEAR);
   }  
}


/**
* Used to Set desired gate mode either Fixed or Matched 
* @param mode -Set the gate mode 
*/
 void  CV1720::setGateMode(GateMode mode)
{ 
   if (mode==Fixed) {
       pokel(MODE,CONFIG_CLEAR); 
   }
   
   else if (mode==Matched) {
	  pokel(MODE,CONFIG_SET);                                       
   }
}
  
  
/**
* Selects the appropriate output signal Gate, Discriminator and Coincidence
* @param selection- output selection
*/
void CV1720::selectTriggerOutput(TRGOutput selection)
{  
   if (selection==Gate) {
	   pokel(GATE,CONFIG_CLEAR); 
   }
   
   else if (selection==Discriminator) {
   
        int oldconfig=peekl(CONFIG); //value currently int the register  
		int newconfig=(oldconfig & ~DIOMASK); // clear bits
		newconfig=(newconfig | DISCRI);
	    pokel(newconfig,CONFIG);
   }
   
   else if (selection==Coincidence) {
        int oldconfig=peekl(CONFIG); //value currently int the register  
		int newconfig=(oldconfig & ~DIOMASK); // clear bits
		newconfig=(newconfig | COINCIDENCE);  // set the new bits
	    pokel(newconfig,CONFIG);  
   }
}

/**
* Generates a local trigger when the signal exceeds the treshold for a given channel
* @param threshold -the value of the desired threshold 
* @param channel - the channel to apply the threshold setting to       
*/
void CV1720::setTriggerThreshold(uint16_t threshold,int channel)
{
   throwIfBadChan(channel,"SetTriggerThreshold");
   pokel(threshold,(CHAN_THRESHOLD+(channel*INC_CHANNEL)));  		   
}

/** 
 * Set the time the signal has to be over threshold for it to make a trigger
 * in sample.
 * @param samples - number of samples that must be above threshold.
 * @param channel - channel number.
 */
void
CV1720::setOverThresholdTime(uint32_t samples, int channel)
{
  throwIfBadChan(channel,"SetOverThresholdTime");
  pokel(samples, (CHAN_OVERUNDER + (channel* INC_CHANNEL)));
}

/**
* Generates a local trigger when the signal exceeds the treshold for a given channel
* @param threshold -the value of the desired threshold 
* @param channel - the channel to apply the threshold setting to
*/

void  CV1720::setTriggerAveragingPeriod(uint8_t period)
{
   throwIfBadRange(TRIGAVGMAX,"setTriggerAveraging",period);
   uint32_t dpp1Reg=peekl(DPP_PAR1_CH0); // get the old value in the register 
   uint32_t newperiod= (uint32_t) period;
   dpp1Reg= dpp1Reg &(~TRIGAVGMASK);
   dpp1Reg=  dpp1Reg | (newperiod << TRIGAVGSHIFT); 
   pokel(dpp1Reg,DPP_PAR1);
}

/**
* Generates a local trigger when the signal exceeds the treshold for a given channel
* @param threshold -the value of the desired threshold 
* @param channel - the channel to apply the threshold setting to
*/

void  CV1720::setTriggerRiseTime(uint8_t riseTime)
{
   throwIfBadRange(RISETIMEMAX,"setTriggerRiseTime",riseTime);
   uint32_t dpp1Reg=peekl(DPP_PAR1_CH0);
   uint32_t updateRiseTime= (uint32_t) riseTime;
   dpp1Reg= dpp1Reg & (~RISETIMEMASK);
   dpp1Reg=  dpp1Reg | (updateRiseTime);
   pokel(dpp1Reg,DPP_PAR1);
}

/**
* Set the gate tail gate widh for every channel
* @param width - desired width 
*/
 void CV1720::setGateTailWidth(uint16_t width)
{
   throwIfBadRange(GATETAILWIDTHMAX,"setGateTailWidth",width);
   uint32_t dpp2Reg=peekl(DPP_PAR2_CH0); // get the old value in the register 
   uint32_t tailwidth= (uint32_t) width;
   dpp2Reg= dpp2Reg & (~GATETAILMASK); //clear out the old bits
   dpp2Reg=  dpp2Reg | (tailwidth << GATETAILSHIFT); 
   pokel(dpp2Reg,DPP_PAR2);
}
 
/**
*  Set the gate Pre Trigger widtd for all Channels
*  @param width 
**/
 void CV1720::setGatePreTriggerWidth(uint8_t width)
{
   throwIfBadRange(GATEPRETRIGMAX,"setGatePreTriggerWidth",width);
   uint32_t dpp2Reg=peekl(DPP_PAR2_CH0); // get the old value in the register 
   uint32_t prewidth= (uint32_t) width;
   dpp2Reg= dpp2Reg & (~PRETRIGMASK); //clear out the old bits
   dpp2Reg=  dpp2Reg | (prewidth << PRETRIGSHIFT); 
   pokel(dpp2Reg,DPP_PAR2);
}
 
/**
* Set the gate HoldOff witdh for all channels
* @param width -desired width 
*/
 void CV1720::setGateHoldoffWidth(uint16_t width)
{
   throwIfBadRange(HOLDOFFMAX,"setGateHoldoffWidth",width);
   uint32_t dpp2Reg=peekl(DPP_PAR2_CH0); // get the old value in the register 
   uint32_t gateholdoffwidth= (uint32_t) width;
   dpp2Reg= dpp2Reg & (~GATEHOLDOFFMASK); //clear out the old bits
   dpp2Reg=  dpp2Reg | (gateholdoffwidth <<GATEHOLDSHIFT); 
   pokel(dpp2Reg,DPP_PAR2);
}
 
 

/**
* Freeze basline calculation
* @param width -desired width  in double sample periods
*/ 
void CV1720::setBaselineInhibitThreshold(uint8_t threshold)
{
   throwIfBadRange(BASELINETHRESHMAX,"setBaselineInhibitThreshold",threshold);
   uint32_t dpp1Reg=peekl(DPP_PAR1_CH0);
   uint32_t updatethreshold= (uint32_t) threshold;
   dpp1Reg= dpp1Reg & (~BASELINETHRESHMASK);
   dpp1Reg=  dpp1Reg | (updatethreshold << BASELINETHRESHSHIFT);
   pokel(dpp1Reg,DPP_PAR1);
}

 /**
 * Set the Baseline Inhbit width 
 * @param - threshold to hold basline
 */
 
 void CV1720::setBaselineInhibitWidth(uint16_t threshold)
{  
   throwIfBadRange(BASELINEWIDTHMAX,"setBaselineInhibitWidth",threshold);
   uint32_t dpp3Reg=peekl(DPP_PAR3_CH0);
   uint32_t inhibitwidth = (uint32_t) threshold;
   dpp3Reg= dpp3Reg & (~BASELINEWIDTHMASK);
   dpp3Reg=  dpp3Reg | (inhibitwidth << BASELINEWIDTHSHIFT);
   pokel(dpp3Reg,DPP_PAR3); 
}
 
  
/**
 * Sets the number of samples to average to calculate the baseline
 * @param width - number of samples
 */ 

void  CV1720::setBaselineAveragingPeriod(uint16_t width)
{
   throwIfBadRange(AVGMAX,"setBaselineAvergingPeriod",width);
   uint32_t dpp3Reg=peekl(DPP_PAR3_CH0);
   uint32_t avg= (uint32_t) width;
   dpp3Reg= dpp3Reg & (~BASELINEPERMASK);
   dpp3Reg=  dpp3Reg | (avg << BASELINEPERSHIFT);
   pokel(dpp3Reg,DPP_PAR3); 
}
 
/**
* Sets the Width of the discriminator channel
* @param width -desired width  
*/ 
void CV1720::setCoincidenceWidth(uint8_t width)
{
   throwIfBadRange(COINCIDENCEMAX,"setCoincidence",width);  
   uint32_t dpp1Reg=peekl(DPP_PAR1_CH0);
   uint32_t updatewidth= (uint32_t) width;
   dpp1Reg= dpp1Reg & (~COINCIDENCEMASK);
   dpp1Reg=  dpp1Reg | (updatewidth <<COINCIDENCESHIFT);
   pokel(dpp1Reg,DPP_PAR1);
	
}

 
 
/**
* Writes a value to the SW_RESET Register causing a reset of the board. All registers are cleared
*/
void CV1720::swReset()
{
   pokel(0,SW_RESET); //Write any value to register to Reset

   // Wait for the module to be ready:

   waitRunning(READY_WAITUS);
   sleep(2);			// TODO: Tune this delay later

}


/**
* Writes a value to the swClear register. Clears All memory
*/
void CV1720::swClear()
{
   pokel(0,SW_CLEAR);  //write any value to register to clear 
}




/**
* A Write  access to this register causes a software reset and a reload of conguration ROM parameters
*
*/
void CV1720::loadFirmware()
{
   pokel(0,RELOAD);
   sleep(2);			// TODO: Tune this delay.
   waitRunning(READY_WAITUS);
   sleep(2);			// TODO: Tune this delay.
}


/**
* When two acquistion windows overlap the second trigger can be accpeted or rejected
* @param discard -true overlapping disable false overlapping enabled
*
*/
void CV1720::discardTriggerOverlap(bool discard)
{
   if (discard) {
      pokel(OVERLAP,CONFIG_CLEAR);
   }
   
   else {
      pokel(OVERLAP,CONFIG_SET);
   }
}

 
 
/**
* Set the number of post trigger sammples
* @param windowWidth -desired windowWidth
*/ 
void CV1720::setPostTriggerWindow(uint32_t windowWidth)
{
   pokel(windowWidth,POST_TRG);
}
   
 
/**
* Allows to write the correct GEO address  
* @param moduleId -the GEO address of a module  
*/
void CV1720::setModuleId(uint8_t moduleId)
{ 
   pokel(moduleId,BOARD_ID);
}


/*!
* Read a single event from the module to the buffer provided by the user.
* @param pBuffer  - Pointer to user managed storage for the event.
* @param maxSize  - Maximum number of bytes of storage available in the buffer.
* @return ssize_t
* @retval  -1     - Actual event won't fit in the user's buffer

* @retval   0     - There are no events to read (e.g. haveData will return false)
* @retval  >0     - Number of uint32_t's actually read into the user's buffer.
*
* @note The caller is responsible for putting the data they get into an NSCL daq buffer.
*       (e.g. a DAQWordBuffer).
*/
ssize_t
CV1720::readEvent(void* pBuffer, size_t maxSize)
{
  // If there isn't an event don't need to  do anything more:

  if (!haveData()) {
    return 0;
  }
  
  //   the EVENT_SIZE register tells us how big the next unread event is:

  uint32_t eventLongs = peekl(EVENT_SIZE);
  if (eventLongs != 0) {
  }
  if (eventLongs*sizeof(uint32_t) <= maxSize) {
  
    // Valid size:

    uint32_t* d = reinterpret_cast<uint32_t*>(pBuffer);
    for(int i =0; i < eventLongs; i++) {
      *d++ = peekl(EVENT_READOUT_BUFFER);
    }
  }
  else {
    for (int i =0; i< eventLongs; i++) {
      toobigdata[i] = peekl(EVENT_READOUT_BUFFER);
    }
    return -1;			// Invalid size.
  }
  return eventLongs;
}
/**
 * Sets the channel trigger enable mask...
 * Note that sw tiggers and ext triggers are always
 * enabled.
 * @param mask - mask of channels to enable
 */
void
CV1720::setChannelTriggerMask(uint8_t mask)
{
  uint32_t registerValue = mask;
  registerValue |= TSRC_SWTRIG | TSRC_EXTRIG;
  pokel(registerValue, TRIGGER_SOURCE);
}


/*----------------------------------------------------------------------*/
/* Private utilities:
 */

/** 
 * m_pModule peek but with a byte offset:
 * @param offset  - Byte offset into the window represented by m_pModule
 * @return uint32_t 
 * @retval The value at that location.
 */
uint32_t
CV1720::peekl(uint32_t byteOffset)
{
  return m_pModule->peekl(byteOffsetToLongOffset(byteOffset));
}
/**
 * m_pModule poke but with byte offset.
 * @param uint32_t value - the value to poke.
 * @param uint32_t byteOffest - the byte offset at which to poke.
 *
 */
void
CV1720::pokel(uint32_t value, uint32_t byteOffset)
{
  m_pModule->pokel(value, byteOffsetToLongOffset(byteOffset));
}


/**
 * throw an Exception if channel is not within allowed Range 
 * @param channel - the channel number 
 * @param methodName the function the exception occured in 
 */
void CV1720::throwIfBadChan(int channel, const char * methodName )
{

 if (channel>=0 && channel>CHANNEL_COUNT) {
   std::ostringstream error;
   error<<"An exception occured in "<< methodName<<"Channel "<<channel<<" is out of range.Channel values 1-7 are valid";   
   string warning(error.str());
   throw warning;
 }

}


/**
 * throw an Exception if channel is not within allowed Range 
 * @param maxRange - the maximum allowed value for the function
 * @param methodName the function the exception occured in
 * @param value  - the actual value passed to the function
 */

void CV1720::throwIfBadRange(int maxRange , const char * methodName , int value)
{
   if (value>maxRange) {
       std::ostringstream error;
      error<<"An exception occured in "<< methodName<<"the value you entered was too large Please see dcoumenation for Maximum allowed Size  "; 
      string warning(error.str());
      throw warning; 
   }

}
bool CV1720::haveData()
{
  uint32_t events = peekl(EVENTSTORED);
   if(events > 0) {
      return true;
     }
   else {
      return false;
   }   
  

}
/**
 * Start/enable data taking.
 */
void
CV1720::run()
{
  pokel(peekl(ACQUISITION_CONTROL)  | ACQ_START,
	ACQUISITION_CONTROL);
}
/*
 * Disable data taking:
 */
void
CV1720::stop()
{
  uint32_t acqReg = peekl(ACQUISITION_CONTROL);
  acqReg  = acqReg & (~ACQ_START);
  pokel(acqReg, ACQUISITION_CONTROL);
}

/**
 * Private function to wait until the run bit is on
 * for at most some number of usec..note that the
 * granularity of 100usec
 * @param usec  -number of microseconds to wait for running
 */
void 
CV1720::waitRunning(uint32_t usec)
{

   unsigned loopPasses  = usec / 100;

   while (loopPasses) {
     usleep(100);
     if (peekl(ACQUISITION_STATUS) & ACQSTAT_ACQREADY) {
       return;
     }
     loopPasses--;
   }
   throw std::string("Warning board not ready after swReset");
}

/**
 * set the trigger channel mode:
 * @param individual - TRUE  individual trigger, false,
 *                     common trigger all channels.
 *                     DPP mode requires individual triggers.
 */
void
CV1720::setTriggerMode(bool individual)
{
  if(individual) {
    pokel(INDIVIDUAL_TRIG, CONFIG_SET);
  }
  else {
    pokel(INDIVIDUAL_TRIG, CONFIG_CLEAR);
  }
}

/**
 * Set the number of buffers of events to maintain
 * @param numBufs - log2 of thenumber of buffers
 *                 Note that you cannot supply more than
 *                7 for this.
 */
void
CV1720::setBufferOrg(uint8_t numBufs)
{
  throwIfBadRange(MAXBUFS, "setBufferOrg", numBufs);
  pokel(numBufs, BUFFER_ORG);

}
/**
 * Set the custom size...it's not clear what this means
 * in DPP non scope mode
 * 
 * @param nItems - value to put in the custom size register.
 */
void
CV1720::setCustomSize(uint32_t nItems)
{
  pokel(nItems, CUSTOM_SIZE);
}
