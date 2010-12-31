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
#include <config.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <Exception.h>

#include <iostream>
#include <string>
#include <CSocket.h>


/*!
 *  This file contains a simple server listener for the tcp/hoister (spectcldaq.server).
 *  Each connection instance is honored by forking and running the program in
 *  a directory two levels above the one in which this program is installed.  Except for the use of
 * the CSocket class, and I/O stream, this could just as easily be a C program rather than C++
 */



/*!
 *  Print program usage to std::cerr:
 */
static void
usage()
{
  std::cerr << "Usage:\n";
  std::cerr << "   serverListener port\n";
  std::cerr << "Where:\n";
  std::cerr << "   port is the service name or port number on which to listen for connections\n";
}
/*!
 *  Return the name of the program to fork/exec when a connection is received.
 * @param myname  - Full path to this program.
 * @return
 * @retval - Full path to the program to run.
 */
std::string getServerInstanceName(std::string myname)
{
  // The damned dirname function accepts non const char* stuff and may even
  // modify the input string so:

  char* pName = new char[myname.size()+1];
  strcpy(pName, myname.c_str());
  
  std::string serverInstance(dirname(pName));
  delete []pName;
  serverInstance += "/../../spectcldaq.server";

  char* pFullName =  realpath(serverInstance.c_str(), NULL);


  return std::string(pFullName);
  
}
/// Stub:
static void
createNewInstance(CSocket* pSocket, CSocket* listener, std::string program)
{
  int status = fork();
  if (status > 0) {
    return;			// Parent
  } else if (status < 0) {
    perror("Fork failed");
    return;
  }
  // Child process... we need to make stdout the socket's fd:
  // and close the listener socket so we don't inherit it:

  close(listener->getSocketFd());

  int sockfd = pSocket->getSocketFd();
  dup2(sockfd, STDOUT_FILENO);	// Now stdout is the socket.
  
  // exec to run the program.

  execl(program.c_str(), program.c_str(), NULL);
  perror("Failed execl");
  exit(EXIT_FAILURE);

  
  
}

/*!
 * Listen for connections on the service port and 
 * spawn off the server instance program if we get one.
 * @param service - service name or port number.
 * @param instance - Path to program to run for client.
 */

static void server(std::string service, std::string instance) 
{
  CSocket listener;
  listener.Bind(service);
  listener.Listen();
  while (1) {
    std::string client;
    CSocket* instanceSocket = listener.Accept(client);
#ifdef SERVER_DEBUG
    std::cerr << "Connection from " << client << std::endl;
#endif
    createNewInstance(instanceSocket, &listener,  instance); 
    // Don't delete as that will shutdown.
    // TODO:
    //   provide mechanism to mark a server instance socket as close on destroy (disconnected0.

  }
}

/*!
 * Main program requires a single parameter, the port on which this program should be listened.
 * The port can be one in /etc/services or a port number.
 */
int main(int argc, char** argv)
{
  //
  // Need the right number of parameters.
  //
  if (argc != 2) {
    usage();
    exit(EXIT_FAILURE);
  }
  // Extract the port, and program name as strings.

  std::string me(argv[0]);
  std::string port(argv[1]);

  // the ifdef below supports foreground debugging.

#ifndef SERVER_DEBUG
  int status = daemon(0,1);			// Run in the background
  if (status == -1) {
    int e = errno;
    std::cerr << "Warning could not background: " << strerror(e) << std::endl;
    std::cerr << "Runing in non daemon mode\n";
  }
#endif

  try {
    std::string serverInstance = getServerInstanceName(me);

    server(port, serverInstance);
  }
  catch(CException& rExcept) {
    std::cerr << "CException : " << rExcept.ReasonText() << " : " 
	      << rExcept.WasDoing() << std::endl;
  }
  catch (std::string msg) {
    std::cerr << "String exception: " << msg << std::endl;
  }
  catch (const char* msg) {
    std::cerr << "const char* exception: " << msg << std::endl;
  }

}


void* gpTCLApplication(0);
