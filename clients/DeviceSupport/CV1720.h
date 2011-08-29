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

#ifndef __CV1720_H
#define __CV1720_H

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __CRT_UINSTD_H
#include <unistd.h>
#ifndef __CRT_UNISTD_H
#define __CRT_UNISTD_H
#endif
#endif

// Forward class definitions.


class CVmeModule;		/* Encapsulates the VME bus interface. */


/*!
 *  This class provides support for the CAEN V1720 with
 *  Charge integration firmware.  It is provided to allow
 *  Artemis et al. to test that board in conjunction with the
 *  SUN detector.  As such this should not be treated as a finished
 *  product.  The correct way to implement support for this board
 *  is as a base class for the 'brain dead board' and derived classes
 *  for each of the firmware options the board has.
 */
class CV1720
{
  /* Exported data types: */

public:
  typedef enum _GateMode {
    Fixed, Matched
  } GateMode;

  typedef enum _TRGOutput {
    Gate, Discriminator, Coincidence
  } TRGOutput;

  /* private storage: */

private:
  CVmeModule* m_pModule;		//< VME representation of the module.

  /* Canonical functions */

public:
  CV1720(uint32_t baseAddress, 
	 uint8_t  moduleId,
	 uint8_t  crate = 0);
  virtual ~CV1720();
  
  /* forbidden canonicals: */

private:
  CV1720(const CV1720& rhs);
  CV1720& operator=(const CV1720& rhs);
  int operator==(const CV1720& rhs) const;
  int operator!=(const CV1720& rhs) const;

  /* Operations on the module: */

public:
  
  /* Utility operations.    */

   void setChannelMask(uint8_t enables); 
   void setDCOffset(unsigned nChannel, uint16_t offset); 
   void invertInput(bool invert);
   void selfTrigger(bool enable); 
   void setGateMode(GateMode mode); 
   void selectTriggerOutput(TRGOutput selection);

   void setTriggerThreshold(uint16_t threshold,int channel);
   void setOverThresholdTime(uint32_t samples, int channel);


   void setTriggerAveragingPeriod(uint8_t period);
   void setTriggerRiseTime(uint8_t riseTime);

   void setGateTailWidth(uint16_t width); 
   void setGateTailWidth(uint16_t width, int channel); 
   void setGatePreTriggerWidth(uint8_t width); 
   void setGateHoldoffWidth(uint16_t width);

   void setBaselineInhibitThreshold(uint8_t threshold); 
   void setBaselineInhibitWidth(uint16_t width); 
   void setBaselineAveragingPeriod(uint16_t width); 

   void setCoincidenceWidth(uint8_t width);

   void swReset(); 
   void swClear();  
   void loadFirmware(); 

   void discardTriggerOverlap(bool discard); 
   void setPostTriggerWindow(uint32_t windowWidth); 
   void setModuleId(uint8_t id);
   
   void run();
   void stop();

   void setTriggerMode(bool individual);

   void setChannelTriggerMask(uint8_t mask);
   void setTriggerOutEnableMask(uint32_t mask);

  bool  haveData(); 
  ssize_t readEvent(void* pBuffer, size_t maxSize); 

  void setBufferOrg(uint8_t numBufs);
  void setCustomSize(uint32_t nItems);

  // private utilities:
 private:
  inline uint32_t byteOffsetToLongOffset(uint32_t byteOffset) {
    return byteOffset/sizeof(uint32_t);
  }
  uint32_t peekl(uint32_t byteOffset);
  void     pokel(uint32_t value, uint32_t byteOffset);
  void throwIfBadChan( int channel,const char  * methodName );
  void throwIfBadRange(int maxRange , const char * methodName , int value);  
  void waitRunning(uint32_t usec);
};

#endif
