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
#include <CAlarmLogger.h>
#include <stdio.h>
#include <pwd.h>
#include <sys/types.h>
#include <CDuplicateNameException.h>
#include <CNoSuchObjectException.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


/*!
  "Default Constructor"  This is the default constructor which constructs a
  CAlarmLogger given a list of hosts to which it will form socket connections when
  logging events, and a facility name which will be the name of the facility
  doing the logging.

  \param facility - the name of the facility doing the logging
  \param expid    - the name of the running experiment.
  \param host  default: "localhost"     - the host on which the logger is running.
  \param port  default: "daqalarm"    - port on which the alarm logger is listening 
      for connections.
*/
CAlarmLogger::CAlarmLogger(string facility, 
			   string expid,
			   string host, 
			   string port) :
  m_sFacility(facility),
  m_sHost(host),
  m_sExpId(expid)
{
  if(port == "daqalarm") {
    struct servent* pServ = getservbyname("daqalarm", "tcp");
    if(pServ) {
      Int_t nPort = pServ->s_port;
      char port[4];
      sprintf(port, "%d", nPort);
      m_sPort = string(port);
    }
    else {
      m_sPort = "2703";
    }
  }
}

/*!
  "Copy Constructor"  This is the copy constructor. It creates a new object by
  copying the information of the reference object which is its parameter.

  \param aCAlarmLogger - the reference object whose attributes will be copied.
*/
CAlarmLogger::CAlarmLogger(const CAlarmLogger& aCAlarmLogger) :
  m_sFacility(aCAlarmLogger.m_sFacility),
  m_sHost(aCAlarmLogger.m_sHost),
  m_sPort(aCAlarmLogger.m_sPort),
  m_sExpId(aCAlarmLogger.m_sExpId)
{ }

/*!
  "Destructor" Called when an object goes out of scope, or when execution
  of the program is terminated. Destroys the object, and frees up space.
*/
CAlarmLogger::~CAlarmLogger()
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

\param message - This is the message which the caller wants to log.
*/
void
CAlarmLogger::Log(string message)
{
  // Put together a "from" string...
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
  char szBuf[1000];
  time_t tim = time(NULL);
  char time_string[28];
  if(tim != (time_t)-1) {
    struct tm* t = localtime(&tim);
    sprintf(time_string, "%04d-%02d-%02d %02d:%02d:%02d",
	    (t->tm_year+1900), (t->tm_mon+1), (t->tm_mday),
	    (t->tm_hour), (t->tm_min), (t->tm_sec));
    if(t->tm_isdst)
      strcat(time_string, " DST");
    else
      strcat(time_string, " EST");
  }
  else {
    sprintf(time_string, "unavailable");
  }

  sprintf(szBuf, "%s 0 %s ~ %s ~ %s ~ %s", m_sExpId.c_str(), 
	  m_sFacility.c_str(), message.c_str(), time_string, from.c_str());
  void* pBuf = (void*)szBuf;

  // Construct a socket and log to the log file using the name of the first
  // host in the hostlist. Then try the write the command to log. Read the
  // date/time which is returned, and shutdown the connection.
  try {
    CSocket sock;
    sock.Connect(m_sHost, m_sPort);
    sock.Write(pBuf, strlen(szBuf));
    sock.Shutdown();
  }
  catch (CException& e) {
    cerr << "Caught exception while attempting to connect to host " 
	 << m_sHost << endl;
    cerr << "Reason was: " << e.ReasonText() << endl;
    cerr << e.WasDoing() << endl;
  }
}
