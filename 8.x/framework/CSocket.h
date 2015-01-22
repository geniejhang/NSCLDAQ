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

// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//
// Copyright 
//   NSCL All rights reserved.
//
#ifndef __CSOCKET_H  //Required for current class
#define __CSOCKET_H

//
// Include files:
//

#ifndef __STL_STRING
#include <string>
#define __STL_STRING
#endif

#ifndef __STL_MAP
#include <map>
#define __STL_MAP
#endif

#ifndef __INET_IN_H
#include <netinet/in.h>
#define __INET_IN_H
#endif




/*!
  Encapsulates a generalized TCP/IP SOCK_STREAM
  socket. 
  Note that TCP/IP Sockets can come in two flavors:
  Clients and Servers.  Clients must perform a connect,
  while servers perform a bind, listen and then serveral
  accepts to create 'server instances'.
  The state of a socket is maintained in the m_State
  variable and is from the enumerator:
  CSocket::SocketState
  
  - Disconnected:  The socket is not connected to anything.
  - Bound:         The socket is a server socket which is
                   not connected, but has been bound to
                   a service port.
  - Listening:     The socket is a server port which is
                   listening and can therefore accept
                   connections
  - Connected      The socket is either a client or a 
                   server instance and is connected
                   to it's counterpart.
  
  */
class CSocket
{
public:

  //! Captures the state of the socket.  See general remarks for more info
  typedef enum _SocketState {
    Disconnected,
    Bound,
    Listening,
    Connected
  } SocketState;

private:
  
  // Private Member data:
  int m_Fd;			//!<  Socket  
  CSocket::SocketState m_State;	//!<  State of socket.  
  static STD(map)<CSocket::SocketState, STD(string)> m_StateNames;  //!< State name lookup tbl.
   
  // Public nested data types:



public:
  CSocket ();
  CSocket (    int am_Fd,  CSocket::SocketState am_State  ) ;

  virtual ~CSocket ( );
  
  //! Copy Constructor forbidden.
private:
  CSocket (const CSocket& aCSocket );
  
  //! Assignment Operator Forbidden
  CSocket& operator= (const CSocket& aCSocket);
 
  //! Equality Operator Forbidden
  int operator== (const CSocket& aCSocket) const;
public:

// Selectors:

public:

  //! Get accessor function socket file descriptor.
  int getSocketFd() const
  { return m_Fd;
  }  

  //! Get accessor function for socket state.
  CSocket::SocketState getState() const
  { return m_State;
  }   

// Attribute mutators:

protected:

  //! Set accessor function for Socket file descriptor.
  void setSocketFd(const int am_Fd)
  { m_Fd = am_Fd;
  }  
  //! Set accessor function for current socket state.
  void setState (const CSocket::SocketState am_State)
  { m_State = am_State;
  }   

  // Class operations:

public:

  void Connect (const STD(string)&     host,      const STD(string)&  service);
  void Connect (unsigned long int IpAddress, unsigned short service);
  void Bind (const STD(string)& service)   ;
  void Listen (unsigned int nBacklog=5)   ;
  CSocket*  Accept (STD(string)& client)   ;
  void Shutdown ()   ;
  int Read (void* pBuffer, size_t nBytes)   ;
  int Write (void* pBuffer, size_t nBytes)   ;
  void getPeer (unsigned short& port, STD(string)& peer)   ;
  void OOBInline (bool State=true)   ;
  bool isOOBInline ()   ;
  void setRcvLowWaterMark (size_t nBytes)   ;
  size_t getRcvLowWaterMark ()   ;
  void setSndLowWaterMark (size_t nBytes)   ;
  size_t getSndLowWaterMark ()   ;
  void setRcvTimeout (unsigned int nMs)   ;
  unsigned int getRcvTimeout ()   ;
  void setSndTimeout (unsigned int nMs)   ;
  unsigned int getSndTimeout ()   ;
  void Debug (bool fState=true)   ;
  bool isDebug ()   ;
  void SetNotRoutable (bool fRoutable=true)   ;
  bool isNotRoutable ()   ;
  void setSndBufSize (size_t nBufferSize)   ;
  size_t getSndBufSize ()   ;
  void setRcvBufSize (size_t nBytes)   ;
  size_t getRcvBufSize ()   ;
  void setLinger (bool lOn, int nLingerSeconds)   ;
  void getLinger (bool& isLingering, int& nLingerSeconds)   ;

  static STD(string) StateName(CSocket::SocketState state);


  // Protected utility functions.
protected:
  static void StockStateMap();
  unsigned short Service(const STD(string)& rService);
  STD(string)         AddressToHostString(in_addr peer);
  void           OpenSocket();
};

#endif

