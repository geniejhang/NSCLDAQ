
/*
This software is Copyright by the Board of Trustees of Michigan
State University (c) Copyright 2015.
You may use this software under the terms of the GNU public license
(GPL). The terms of this license are described at:
http://www.gnu.org/licenses/gpl.txt
Author:
Ron Fox
NSCL
Michigan State University
East Lansing, MI 48824-1321
*/

/*
  This is the implementation file for the MyEventSegment
  class. This class defines funtions that can be used to
  readout any module covered in the CAENcard class. These
  include the V785, V775, and V792
  Tim Hoagland
  11/3/04
  s04.thoagland@wittenberg.edu
*/
#include <string>

#ifdef HAVE_STD_NAMESPACE                             
using namespace std;
#endif

// Set the polling limit for a timeout
static int CAENTIMEOUT = 50;

#include "MyEventSegment.h"                             

// Packet version -should be changed whenever major changes are made
// to the packet structure.
static const char* pPacketVersion = "1.0";                    

//constructor set Packet details
MyEventSegment::MyEventSegment(short slot, unsigned short Id):
  m_myPacket(Id,"My Packet","Sample documented packet",pPacketVersion),
  m_module(slot)                                       
{
}

// Is called right after the module is created. All one time Setup
// should be done now.
void MyEventSegment::initialize()
{
  m_module.reset();                                        
  clear();
}

// Is called after reading data buffer
void MyEventSegment::clear()
{
  // Clear data buffer
  m_module.clearData();                                    
}

//Is called to readout data on m_module
size_t MyEventSegment::read(void* pBuffer, size_t maxsize)
{
  int nBytesRead=0;
  // Tests again that data is ready 
  if(m_module.dataPresent())
  {
    // Opens a new Packet
    uint16_t* pBuf = reinterpret_cast<uint16_t*>(pBuffer);
    pBuf = m_myPacket.Begin(pBuf);                                 
    // Reads data into the Packet 
    nBytesRead = m_module.readEvent(pBuf);                         
    // Closes the open Packet
    pBuf = m_myPacket.End(pBuf+nBytesRead/sizeof(uint16_t));       
  }

  // compute number of bytes added to buffer
  return nBytesRead;                                               
}
