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

static const char* Copyright= "(C) Copyright Michigan State University 2002, All rights reserved";

// Author:
//   Jason Venema
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:venemaja@msu.edu
//
// Copyright
//   NSCL All rights reserved.
//
// See Logger.h for a description of this class.
//

#include <config.h>
#include <CLogger.h>
#include <stdio.h>
#include <pwd.h>
#include <sys/types.h>
#include <CDuplicateNameException.h>
#include <CNoSuchObjectException.h>


#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

#define PORT string("2702")

/*!
  "Default Constructor"  This is the default constructor which constructs a
  CLogger given a list of hosts to which it will form socket connections when
  logging events, and a facility name which will be the name of the facility
  doing the logging.

  \param facility - the name of the facility doing the logging
*/
CLogger::CLogger(string facility) :
  m_sFacility(facility)
{ }

/*!
  "Copy Constructor"  This is the copy constructor. It creates a new object by
  copying the information of the reference object which is its parameter.

  \param aCLogger - the reference object whose attributes will be copied.
*/
CLogger::CLogger(const CLogger& aCLogger) :
  m_HostList(aCLogger.m_HostList),
  m_sFacility(aCLogger.m_sFacility)
{ }

/*!
  "Destructor" Called when an object goes out of scope, or when execution
  of the program is terminated. Destroys the object, and frees up space.
*/
CLogger::~CLogger()
{ }

/*!
  Operation Type:
     Log to file and screen

     Attempts to log a message (facility, severity, message, date) to 
     EventLog.tcl by opening a socket connection to each of the hosts in
     m_HostList. The first connection logs the message which consists of the
     facility, severity, and message to the log file via a call to the tcl
     procedure Logger::Log. This only needs to be written once, since the same
     log file will be used by each displayer. The Logger::Log tcl procedure
     obtains the exact time of the event via a call to date(1). It then returns
     the date string through the socket (which we read) and that string is what
     is displayed on the GUI via a call to Logger::Display_Event. In this way,
     the date on each display will be exactly the same.
     NOTE: If Log() fails for the first host in the list, then the event
     will not be written to the log file and will not be displayed anywhere. 
     If connection to the first host succeeds,  but fails on a subsequent
     host, the event will be logged to file but will not be displayed on
     whichever host connection failed. It will, however, be displayed on
     all hosts (including subsequent hosts) to which connection does not fail.

\exception CErrnoException       - Errno exception occurred
\exception CTCPBadSocketState    - CSocket::m_State was not disconnected
\exception CTCPNoSuchHost        - Host not in DNS or nonexistent
\exception CTCPNoSuchService     - Named service does not translate.
\exception CTCPConnectionFailed  - Connection refused by remote host
\exception CTCPConnectionLost    - Connection terminated by remote host

\param sev     - This is an enumerated value which represent the severity
                 of the event.
\param message - This is the message which the caller wants to log.
*/
bool
CLogger::Log(Severity sev, string message)
{
  if(!m_HostList.size())  // make sure we have some hosts to log to
    return 1;

  bool flag = 1; // this will be the return value

  // Turn the enumeration into a string so it can be logged
  string severity;
  switch(sev) {
  case SUCCESS:
    severity = "Success";
    break;
  case WARNING:
    severity = "Warning";
    break;
  case ERROR:
    severity = "Error";
  }

  // Now put together a "from" string...
  string from("");
  struct passwd* p = getpwuid(geteuid());
  if(p)
    from += (p->pw_name);
  else
    from += "unknown";

  from += "@";
  char hostname[50];
  if(gethostname(hostname, sizeof(hostname)) == 0)
    from += hostname;
  else
    from += "unknown";

  // 1. Prepare a message to write to the socket for EventLog.tcl to log. 
  //    This is the first message that we're writing. Subsequent writes
  //    will be to the display function...
  string entry("Logger::Log {");
  entry += m_sFacility+" "+severity+" "+from+" "+message+"}\n";
  void* pBuf = (void*)entry.c_str();
  char* pRead = new char[29];   // will contain the date and time of the log
                                // upon return from sock.Read()

  // Construct a socket and log to the log file using the name of the first
  // host in the hostlist. Then try the write the command to log. Read the
  // date/time which is returned, and shutdown the connection.
  try {
    CSocket sock;
    sock.Connect(*(m_HostList.begin()), PORT);
    sock.Write(pBuf, entry.size());
    sock.Read((void*)pRead, 28);
    sock.Shutdown();
  }
  catch (CException& e) {
    cerr << "Caught exception while attempting to connect to host " 
	 << *(m_HostList.begin()) << endl;
    cerr << "Reason was: " << e.ReasonText() << endl;
    cerr << e.WasDoing() << endl;
    return 0;
  }

  // 2. Prepare a message to write to the socket for EventLog.tcl to display...
  string display("Logger::Display_Event {");
  display += m_sFacility+" "+severity+" {"+message+"} "
    " {"+(char*)pRead+"} "+from+"\n}";
  void* pDisp = (void*)display.c_str();
  list<string>::iterator It;
  pRead[28] = '\0';  // append null character to the date string

  // For each host in m_HostList, we construct a socket, open a connection to
  // that host, tell it to display out message, and shutdown the connection.
  // Note that each socket can only be used once, which is why a new one has
  // to be constructed each pass through the loop. Catch any exceptions.
  for(It = m_HostList.begin(); It != m_HostList.end(); It++) {
    try {
      CSocket sock;
      sock.Connect((*It), PORT);
      sock.Write(pDisp, display.size());
      sock.Shutdown();
    }
    catch(CException& e) {
      cerr << "Caught exception while attempting to connect to host " 
	   << *(m_HostList.begin()) << " on port " << PORT << endl;
      cerr << "Reason was: " << e.ReasonText() << endl;
      cerr << e.WasDoing() << endl;
      flag = 0;
    }
  }

  // Delete the character array which contained the date string
  delete pRead;
  return flag;
}

/*!
  Operation Type:
     Selector

  Purpose:
     Returns an iterator to the first host in m_HostList.
*/
HostListIterator
CLogger::begin()
{
  return m_HostList.begin();
}

/*!
  Operator Type:
     Selector

  Purpose:
     Returns an iterator which points to just past the last host
     in m_HostList.
*/
HostListIterator
CLogger::end()
{
  return m_HostList.end();
}

/*!
  Operation Type:
     Selector

  Purpose:
     Returns the number of hosts that are currently being logged to.

\return The number of hosts in m_HostList
*/
int
CLogger::size()
{
  return m_HostList.size();
}

/*!
  Operation Type:
     Mutator

  Purpose:
     Adds a host to the list of hosts that we will attempt to form
     a connection with and log a message to.

\param newHost  The name of the new host to be 
                              added to m_HostList

\exception CDuplicateNameException  Thrown if the host already 
                                    exists in the host list
*/
void
CLogger::AddHost(const string& newHost)
{
  HostListIterator It;
  for(It = m_HostList.begin(); It != m_HostList.end(); It++) {
    if((*It) == newHost) {
      CDuplicateNameException dne
	("CLogger::AddHost - Adding host to list of logging hosts", newHost);
	throw dne;
    }
  }
  m_HostList.push_back(newHost);
}

/*!
     Removes a host from the list of hosts that we will attempt to form
     a connection with and log a message to.

\param  oldHost  The name of the old host to be 
                              removed from m_HostList
\exception CNoSuchObjectException  Thrown if the name supplied in the parameter
                                   is not in m_HostList.
*/
void
CLogger::RemoveHost(const string& oldHost)
{
  HostListIterator It;
  for(It = m_HostList.begin(); It != m_HostList.end(); It++) {
    if((*It) == oldHost) {
      m_HostList.remove(*It);
      break;
    }
  }
  if(It == m_HostList.end()) {
    CNoSuchObjectException nsoe
      ("CLogger::RemoveHost - Host is not in hostlist", oldHost);
    throw nsoe;
  }
}

/*!
  Operation Type:
     Mutator

  Purpose:
     Removes a host from the list of hosts that we will attempt to form
     a connection with and log a message to.

\param It  An iterator which points to the host in
                            m_HostList that is to be removed.
\exception CNoSuchObjectException  Thrown if the name supplied in the paramter
                                   is not in m_HostList.
*/
void
CLogger::RemoveHost(HostListIterator It)
{
  HostListIterator i;
  for(i = m_HostList.begin(); i != m_HostList.end(); i++) {
    if((*i) == (*It)) {
      m_HostList.remove(*i);
    }
  }
  if(i == m_HostList.end()) {
    CNoSuchObjectException nsoe
      ("CLogger::RemoveHost - Host is not in hostlist", (*It));
    throw nsoe;
  }
}
