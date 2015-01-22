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

/*
   $Header$

Modification History:
$Log$
Revision 1.2.2.1  2007/07/17 12:43:31  ron-fox
Fix error in handling entity count which could cause random losses
of partial data buffers forced out in scaler events.

Revision 1.2  2005/02/09 14:40:11  ron-fox
Debugged high performance production readout.

Revision 1.1  2005/02/07 19:50:54  ron-fox
Break off branch for HPProduction readout (using transparent copyin).

Revision 4.2  2004/11/22 19:26:11  ron-fox
Port to gcc/g++ 3.x

Revision 4.1  2004/11/08 17:37:30  ron-fox
bring to mainline

Revision 3.2  2003/08/22 18:38:43  ron-fox
Fix errors in buffer formatting (bug 71)

Revision 3.1  2003/03/22 04:03:07  ron-fox
Added SBS/Bit3 device driver.

Revision 1.1.1.1  2003/03/12 04:17:38  ron-fox
Correct initial import this time.

Revision 1.2  2002/11/20 16:14:42  fox
Misc. stuff

 * Revision 1.1  2002/10/09  11:22:29  fox
 * Stamp with copyright/gpl license notice
 *
*/

//////////////////////////CNSCLPhysicsBuffer.h file//////////////////////////////////

#ifndef __CNSCLPHYSICSBUFFER_H  
#define __CNSCLPHYSICSBUFFER_H
                               
#ifndef __CNSCLOUTPUTBUFFER_H
#include "CNSCLOutputBuffer.h"
#endif

#ifdef __HAVE_STD_NAMESPACE
using namespace std;
#endif

#ifndef __SPECTRODAQ_H
#include <spectrodaq.h>
#ifndef __SPECTRODAQ_H
#define __SPECTRODAQ_H
#endif
#endif
                               
/*!
   Encapsulates the formatting of a physics
   buffer.  Physics buffers contain
   - The standard buffer header.
   - A series of entities called events.  Each
     event is a series of words which is lead by
     a self inclusive word count.  
   
   While the NSCL daq system makes no requirements
   on the contents of an event, typically an event consists
   of a series of self described packets each of which contains:
   - Packet size
   - Packet type code
   - Packet contents.
   
 */		
class CNSCLPhysicsBuffer  : public CNSCLOutputBuffer        
{ 
private:
  unsigned short*   m_pBuffer;
  unsigned short*   m_pBufferCursor;
  unsigned short    m_nEntityCount;
  unsigned int      m_nBufferSize;

      
public:
	// Constructors, destructors and other cannonical operations: 

    CNSCLPhysicsBuffer (unsigned nWords = 4096); //!< Default constructor.
    virtual ~ CNSCLPhysicsBuffer ( );            //!< Destructor.
private:
    CNSCLPhysicsBuffer& operator= (const CNSCLPhysicsBuffer& rhs); //!< Assignment
    int         operator==(const CNSCLPhysicsBuffer& rhs) const; //!< Comparison for equality.
    int         operator!=(const CNSCLPhysicsBuffer& rhs) const;
    CNSCLPhysicsBuffer(const CNSCLPhysicsBuffer& rhs); //!< Copy constructor.
public:

	// Selectors for class attributes:
public:

    unsigned short* getEventStartPtr() const {
      return m_pBufferCursor;
    }

	// Mutators:
protected:  

	// Class operations:
public:
     unsigned short* StartEvent ()  ;
     void EndEvent (unsigned short* rPtr)  ;
     void RetractEvent (unsigned short* p)  ;
     virtual void Route();
     int  WordsInBody() const;
     unsigned short getEntityCount() const {
       return m_nEntityCount;
     }
};

#endif
