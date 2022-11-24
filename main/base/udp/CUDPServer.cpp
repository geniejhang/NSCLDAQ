/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  CUDPServer.cpp
 *  @brief: Trivial implementation of the UDP server:
 */
#include "CUDPServer.h"

namespace UDP {
/**
 * constructor
 *  @param port - port (host byte order) through which we receive
 *                data.
 */
CUDPServer::CUDPServer(short port) :
    CUDP()
{
    bind(port);            // This is what makes us a server.
}
/**
 * Destructor
 *    just exists to support constructor chaining.
 */
CUDPServer::~CUDPServer()
{}

/**
 * send
 *    Really just the send from the base class.
 */
int
CUDPServer::send(
            const void* pData, size_t nBytes,
            in_addr_t& ipAddress, short port
)
{
    return CUDP::send(pData, nBytes, ipAddress, port);
}
/**
 * receive
 *     Delegate to protected base class.
 */
int
CUDPServer::receive(
            void* pData, size_t nBytes,
            in_addr_t&  fromIP, short& fromPort
)
{
    return CUDP::receive(pData, nBytes, fromIP, fromPort);
}
}