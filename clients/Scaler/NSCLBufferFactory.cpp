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

static const char* Copyright= "(C) Copyright Michigan State University 2002, All rights reserved";// Class: CNSCLBufferFactory
//   Produces NSCLDaqBuffer derived classes given a DaqWordBuffer.
//
// Author:
//    Ron Fox
//    NSCL
//    Michigan State University
//    East Lansing, MI 48824-1321
//
//
//////////////////////////.cpp file/////////////////////////////////////////////////////
#include <config.h>
#include "NSCLBufferFactory.h"    				
#include "NSCLDaqBuffer.h"
static const char* pCopyrightNotice = 
"(C) Copyright 1999 NSCL, All rights reserved NSCLBufferFactory.cpp \n";

map<NSCLBufferType, CNSCLBufferCreator*> CNSCLBufferFactory::m_rCreatorMap;

// Functions for class CNSCLBufferFactory

CNSCLBufferFactory::CNSCLBufferFactory() {}

//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     Create(DaqWordBuffer& rRawBuffer)
//  Operation Type: 
//     
CNSCLDaqBuffer* CNSCLBufferFactory::Create(DAQWordBuffer& rRawBuffer)  
{
  //  Given a DaqWordBuffer produces the appropriate 'typed' buffer.

  CNSCLDaqBuffer StructuredBuffer(rRawBuffer);
  map<NSCLBufferType,CNSCLBufferCreator*>::iterator item;
  item = m_rCreatorMap.find(StructuredBuffer.getBufferType());
  if(item == m_rCreatorMap.end()) return new CNSCLDaqBuffer(StructuredBuffer);
  return (*item).second->Create(StructuredBuffer);
  
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     Register(NSCLBufferType RecognizedType, CNSCLBufferCreator& rCreator)
//  Operation Type: 
//     
void CNSCLBufferFactory::Register(NSCLBufferType RecognizedType, 
				  CNSCLBufferCreator& rCreator)  
{
  // Registers a BufferCreator in the buffer factory map. 
  // Buffer creator's are in a map indexed by buffer type id.
  // When the Create member of the factory is asked to
  // create a buffer, the type is determined and the appropriate
  // creator in the map is invoked.  If there is no match, then 
  // CNSCLDaqBuffer is created
  //
 
  m_rCreatorMap[RecognizedType] = &rCreator;
}

