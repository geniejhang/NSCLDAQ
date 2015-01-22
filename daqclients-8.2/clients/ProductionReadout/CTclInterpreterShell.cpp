/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2008

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

static const char* Copyright = "(C) Copyright Michigan State University 2002, All rights reserved";/*

  Implements the CTclInterpreterShell class.

  CTclInterpreterShell embodies an extended Tcl interpreter.  Extensions
  include all commands required to run the readout software.  The extensions
  are embodied in a base class CInterpreterShell and get hooked in in one of 
  two ways:  The default constructor for CTclInterpreter class will create one
  The parameterized constructor accepts one pre-created..

Author:
  Ron Fox
  NSCL
  Michigan State University
  East Lansing, MI 48824-1321
  fox@nscl.msu.edu

Change Log:
  5/9/02 - initial version.
  */

#include <config.h>
#include "CTclInterpreterShell.h"                  
#include <TCLLiveEventLoop.h>
#include "CInterpreterCore.h"
#include <ErrnoException.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif
/*!
   Default constructor

   Constructs an interpreter shell when the core is not initially know or
   alternatively is not yet constructable.  Action is as follows:
   - m_pCore   <- 0
   - m_bMyCore <- false

 */
CTclInterpreterShell::CTclInterpreterShell () :
  m_pCore(0),
  m_bMyCore(false)
{}
/*!
   Parameterized constructor:
   
   Construtor to use if an interpreter core can be instantiated/constructed
   prior to the construction of an intperpreter shell 
   (currently this may not be possible).  Action is as follows:
   - m_pCore    <- rCore
   - m_bMyCore  <- false

   \param rCore - CInterpreterCore& [modified] Refers to the core extension
                  package that will be used by this interpreter.  Modified in
		  the sense that at some later point, member functions of this
		  core extension object will be called that can modify the 
		  object's state.  It is the client's responsibility to manage
		  rCore's allocation lifetime.
*/
CTclInterpreterShell::CTclInterpreterShell (CInterpreterCore& rCore) :
  m_pCore(&rCore),
  m_bMyCore(false)
{}
/*!
    Destructor:

      Get rid of the interpreter core if it has been allocated by us:
*/
CTclInterpreterShell::~CTclInterpreterShell()
{
  if(m_bMyCore) {
    delete m_pCore;
  }
}
/*!
   Copy constructor

   Does a shallow copy into this of a reference object.  Note that 
   copy construction is not reccomended as it complicates storage
   management.  In no case does the 
   m_bMyCore flag get set true in order to prevent deletion of the core used
   by the rhs.

   \param rhs - CTclInterpreterShell& rhs [in] - Template interpreter shell to
               copy into this.

*/
CTclInterpreterShell::CTclInterpreterShell(const CTclInterpreterShell& rhs) :
  m_pCore(rhs.m_pCore),
  m_bMyCore(false)
{}
/*!
   Assignment

   Does a shallow copy into this from rhs.  If m_bMyCore is true, the current
   core is deleted prior to doing the copy.  In no case does the m_bMyCore
   wind up as true.  Note that assignment is not reccomended, as it complicates
   storage management.

   \param rhs - const CTclInterpreterShell [modified] - Template to copy into
                 *this.  Modification of the rhs.core may occur if at a later
		 time non const members of that object are called.

   \return - Reference to *this so that operator chaining is supported. 
*/
CTclInterpreterShell& 
CTclInterpreterShell::operator= (const CTclInterpreterShell& rhs)
{
  if(this != &rhs) {
    if(m_bMyCore) {
      delete m_pCore;
    }
    m_pCore   = rhs.m_pCore;
    m_bMyCore = rhs.m_bMyCore;
  }
  return *this;
}
/*!
   Equality comparison

   Compares *this with rhs to determine equality.  Equality is defined over
   CTclIntpreterShell such that if the core pointers are identical, the objects
   are equal.  m_bMyCore is irrelevent since copy construction and assignment
   produce equivalent objects with m_bMyCore unconditionally false.

   \param rhs - const CTclIntepreterShell& [modified] The object to which *this
               will be compared.

   \return 0 if not equal, 0==0 if equal.
 */
int         
CTclInterpreterShell::operator==(const CTclInterpreterShell& rhs) const
{
  return (m_pCore == rhs.m_pCore);
}
/*!
  Register core extensions.

  If no core has been set, the virtual function ConstructCore is called to 
  create an appropriate interpreter core, and m_bMyCore <- true.  Once it is
  established that there is a valid core, its RegisterExtensions member is
  called to register the core command extensions on our interpreter.

  \exception CErrnoException& in the event ConstructCore() returns a null 
                pointer (it is assumed that the reason for the failure to
		construct a valid exception is due to a condition that
		can be captured by errno).

*/
void 
CTclInterpreterShell::RegisterExtensions () 
{
  if(!m_pCore) {
    m_pCore = ConstructCore();
    if(!m_pCore) {
      throw 
	CErrnoException("CTclInterpreterShell::RegisterExtensions -null core");
    }
    m_bMyCore = true;
    CInterpreterShell::Initialize(*m_pCore); // Spectrodaq base classe also
				             // have an Initialize member.
  }

  m_pCore->RegisterExtensions();
  CInterpreterStartup::RegisterExtensions(); // register framework extensions.
  //
  // Create and start our eventl loop.. the assumption is that by now we have
  // and stdio commander ready to run on it.
  
  CTCLLiveEventLoop* pEventLoop = CTCLLiveEventLoop::getInstance();
  
  pEventLoop->start(&Interp());
}
/*!
    Create an interpreter core

    This virtual, overridable function allows the shell to create an 
    interpreter core extension package if none has been defined by the time
    RegisterExtensions() has been called.  The default implementation creates
    and returns a pointer to a CInterpreterCore object.  If a user wants
    to initialize additional extensions, they may extend CInterpreterCore and
    override this member function to instantiate their substitute interpreter
    core.

    \return A pointer to the newly created CInterpreterCore object, or a null
            pointer if the new built in failed.

*/
CInterpreterCore* 
CTclInterpreterShell::ConstructCore()
{
  return new CInterpreterCore(*(getInterpreter()));
}
