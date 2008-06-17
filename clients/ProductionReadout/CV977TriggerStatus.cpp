/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include <config.h>
#include "CV977TriggerStatus.h"
#include <CTrigger.h>
#include <CStatusModule.h>
#include <CCAENV977.h>
#include "CExperiment.h"

/////////////////////////////////////////////////////////////////////////////////////
// 
// The trigger and busy classes are defined only here and live in the namespace
// CV977Private.
//
namespace CV977Private {

  // The trigger class:

  class Trigger : public ::CTrigger
  {
  private:
    CCAENV977*  m_pHardware;
  public:
    Trigger(CCAENV977* pHardware);

    virtual bool operator()();
    
  };

  // the busy class.

  class Busy : public ::CStatusModule 
  {
  private:
    CCAENV977*  m_pHardware;
  public:
    Busy(CCAENV977* pHardware);

    virtual void GoBusy();
    virtual void GoClear();
    virtual void ModuleClear();
    
  };

  //////////////////////////////////////////////////////////////////////////
  // Implementing thhe CV977Private::Trigger class:

  /*
   *  The constructor saves the module pointer, and
   *  - initializes the module to Single hit pattern, input register mode requiring gates.
   *  - sets the input mask -> 0
   *  - sets the otuput mask -> 0xfffe.
   *  - Clears the output and input registers.
   */

  Trigger::Trigger(CCAENV977* pHardware) :
    m_pHardware(pHardware)
  {
    m_pHardware->Reset();
    m_pHardware->controlRegister(0);
    m_pHardware->inputMask(0);
    m_pHardware->outputMask(0xfffe);
    m_pHardware->outputClear();
  }
  /*
   *  The test for trigger is just looking for the bottom bit in the 
   *  single-hit read register.  We don't want to clear this bit as it's used as
   *  the busy output too.
   */
  bool
  Trigger::operator()() 
  {
    return ((m_pHardware->singleHitRead() & 1) != 0);
  }

  ////////////////////////////////////////////////////////////////////////////
  // Implementing the CV977Private::Busy module.
  //
  /*
   * Construction just saves the module pointer.  The guy that makes us
   * made the trigger too and it does the initialization.
   */
  Busy::Busy(CCAENV977* pHardware) :
    m_pHardware(pHardware) {}

  /*
   *  GoBusy is done by setting the input set register trigger bit.
   *  the trigger will not be processed because this entry is called when the
   *  software is entering a state when it will not process triggers.
   *  The module is set up so that this will propagate to the busy output.
   */
  void
  Busy::GoBusy() 
  {
    m_pHardware->inputSet(1);
  }
  /*
   * Go clear just clears the output register.  That clears all the terms that make up
   * the output flip flop.
   */
  void
  Busy::GoClear()
  {
    m_pHardware->outputClear();
  }
  /*
   * ModuleClear just toggles the 2's bit of the OutputSet register.
   * This should leave the other bits alone.
   */
  void 
  Busy::ModuleClear()
  {
    m_pHardware->outputSet(2);
    m_pHardware->outputSet(0);
  }
  
};



/////////////////////////////////////////////////////////////////////////////////////
//
//  CV977TriggerStatus ... establishing the trigger.
/*!
  Create a a CCAENV977 module, use it to create a status and
  trigger module that cooperate to run the trigger as documented in the class
  description.
  \param rExperiment - Reference to the experiment object.
  \param baseAddress - Base address of the CAEN V977 module in the vme space.
  \param crate       - VME crate holding the module.
*/
void
CV977TriggerStatus::Register(CExperiment& rExperiment,
			     uint32_t     baseAddress,
			     unsigned int crate)
{
  // Create the module.

  CCAENV977* pHardware = new CCAENV977(crate, baseAddress);
  rExperiment.EstablishTrigger(new CV977Private::Trigger(pHardware));
  rExperiment.EstablishBusy(new CV977Private::Busy(pHardware));

}
