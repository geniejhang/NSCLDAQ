/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include "CEventOrderClient.h"
#include <ErrnoException.h>
#include <CSocket.h>
#include <CTCPConnectionFailed.h>

#include <CPortManager.h>
#include <os.h>
#include <errno.h>
#include <stdio.h>

#include <string.h>

static const std::string EventBuilderService("ORDERER"); // Advertised service name.

/**
 * Construct the object.
 * @param host - The host on which the event builder is listening for client
 *               connections.
 * @param port - The port on which the event builder is listening for client connections.
 */
CEventOrderClient::CEventOrderClient(std::string host, uint16_t port) :
  m_host(host),
  m_port(port),
  m_pConnection(0)
{}

/**
 * Destroy the object. Any existing connection to the event builder is dropped.
 * properly. See the event builder protocol documentations for a description of 
 * what that means.
 */
CEventOrderClient::~CEventOrderClient()
{
  delete m_pConnection;
}

/**
 * Locate the event builder on the specified host and return
 * the port on which its server is listening for connections.
 * 
 * @param host - the host in which to perform the inquiry.
 * 
 * @return uint16_t
 * @retval - the port on which the event builder is listening for our username.
 *
 */
uint16_t
CEventOrderClient::Lookup(std::string host)
{
  CPortManager manager(host);
  std::vector<CPortManager::portInfo> services = manager.getPortUsage();
  std::string me  = Os::whoami();

  // Look for the first match for my username and the correct service.

  for (int i =0; i < services.size(); i++) {
    if (services[i].s_Application == EventBuilderService &&
	services[i].s_User        == me) {
      return services[i].s_Port;
    }
  }
  // Not running.
  // Use errno = ENOENT

  errno = ENOENT;
  throw CErrnoException("Looking up event builder service");

}
/**
 * Connect to a server.
 *
 * See eventorderer(5daq) for protocol information.
 *
 * @param description - the description used in the CONNECT message
 *                      to describe the client to the server.
 *
 */
void
CEventOrderClient::Connect(std::string description)
{
  char portNumber[32];
  sprintf(portNumber, "%u", m_port);
  m_pConnection = new CSocket();
  try {
    m_pConnection->Connect(m_host, std::string(portNumber));

    char* pConnectMessage = message("CONNECT", strlen("CONNECT"), 
				    description.c_str(), description.size());
    size_t length = strlen("CONNECT") + description.size() + 2*sizeof(uint32_t);
    m_pConnection->Write(pConnectMessage, length);

    delete [] pConnectMessage;

    std::string reply = getReplyString();
    if (reply != "OK") {
      errno = ECONNREFUSED;
      throw CErrnoException("ERROR reply from server");
    }
			     
  }
  catch (CTCPConnectionFailed& e) {
    // Convert to ECONNREFUSED errno
    
    errno = ECONNREFUSED;
    throw CErrnoException("Failed connection to server");
  }

}

/*-------------------------------------------------------------------------------------*/
// Private methods 

/**
 * Return a message consisting of a request header and a body.
 * The message is dynamically allocated and must be freed by the caller as
 * delete []message.
 *
 * @param request - Pointer to the request part of the message.
 * @param requestSize - number of bytes in the request part of the message.
 * @param body    - Pointer to the bytes of data in the body.
 * @param bodySize - number of bytes in the body.
 *
 * @return char*
 * @retval pointer to the formatted message.
 */
char*
CEventOrderClient::message(const void*  request, size_t requestSize, const void* body , size_t bodySize)
{
  // figure out the size of the message:

  uint32_t rsize = requestSize;
  uint32_t bsize = bodySize;
  size_t totalSize = rsize + bsize + 2*sizeof(uint32_t);

  char* pMessage = new char[totalSize];
  if (!pMessage) {
    throw CErrnoException("Allocating buffer");
  }
  // There must always be a request:
  char* p = pMessage;
  memcpy(p, &rsize, sizeof(uint32_t));
  p += sizeof(uint32_t);
  memcpy(p, request, rsize);
  p += rsize;

  // If bsize == 0 or body == NULL, don't put them in the message (request only msg).

  if (body && bsize) {
    memcpy(p, &bsize, sizeof(uint32_t));
    p += sizeof(uint32_t);
    memcpy(p, body, bsize);
  }

  return pMessage;
	
  
}
/**
 * Get a reply string from the server.
 * Reply strings are fully textual lines.  This just means
 * reading a character at a time until the newline.
 *
 * @return std::string.
 */
std::string
CEventOrderClient::getReplyString()
{
  std::string reply;
  while(1) {
    char c;
    m_pConnection->Read(&c, sizeof(c));
    if (c == '\n') return reply;
    reply += c;
  }
}
