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

/** @file:  UDP.cpp
 *  @brief:  Implement UDP::CUDP class base class.
 */
#include "UDP.h"
#include <unistd.h>
#include <sys/types.h>
#include <system_error>
#include <errno.h>
#include <iostream>
#include <string.h>
// #include <csignal>

namespace UDP {

/**
 *     constructor
 *  Creates a new socket that's set up as a datagram socket.
 *  stores that in m_socket;
 */
CUDP::CUDP() :
    m_socket(-1)
{
    m_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_socket < 0) {
        throw std::system_error(errno, std::generic_category());
    }

    // struct sigaction sigact;
    // sigact.sa_sigaction = signalHandler;
    // sigemptyset(&sigact.sa_mask);
    // sigaction(SIGINT, &sigact, NULL);
    // sigaction(SIGTERM, &sigact, NULL);
}
/**
 * destructor
 *    Just close the socket if it's valid:
 */
CUDP::~CUDP()
{
    if (m_socket >= 0) {
        if (close(m_socket) < 0) {
            // We can't actually throw from a destructor!!
            
            int e = errno;
            std::cerr << "CUDP destructor failed to close the socket: "
                << strerror(errno) << std::endl;
        }
    }
}
/**
 * send
 *    Send a datagram to a UDP receiver that's been bound to a port.
 *
 * @param pData     - Data to send.
 * @param nBytes    - Number of bytes to send (must fit in a datagram).
 * @param ipAddress - IP address of host to send it to from e.g. inet_aton
 *                    so it's in network byte order.
 * @param port      - Port that the server is bound to.
 * @return int      - Return value from sendto.
 * 
 */
int
CUDP::send(const void* pData, size_t nBytes, in_addr_t& ipAddress, short port)
{
    // Fill in the to address information:
    
    sockaddr_in to;
    memset(&to, 0, sizeof(to));
    to.sin_family = AF_INET;
    to.sin_port   = htons(port);
    memcpy(&(to.sin_addr.s_addr), &ipAddress, sizeof(in_addr));
    
    // now the send:
    
    return sendto(
        m_socket, pData, nBytes,
        MSG_CONFIRM,
        reinterpret_cast<const sockaddr*>(&to), sizeof(to)
    );
}
/**
 * recieve
 *    Receives a message on a UDP data port.
 *
 *  @param[out] pData - Buffer in which to receive the data
 *  @param[in]  nBytes - Size of the buffer in bytes.
 *  @param[out] fromIP - Node this message came from (network byte order).
 *  @param[out] fromPort -the port this message came from (host byte order)
 *  @return result from recvfrom
 *  @note fromIP and fromPort are only meaningfulif the result is >= 0.
 */
int
CUDP::receive(
    void* pData, size_t nBytes, in_addr_t& fromIP, short& fromPort
)
{
    sockaddr_in from;
    memset(&from, 0, sizeof(from));
    socklen_t fromLen = sizeof(from);
    
    int result = recvfrom(
        m_socket, pData, nBytes, 0,
        reinterpret_cast<sockaddr*>(&from), &fromLen
    );
    if (result >= 0) {
        fromPort = ntohs(from.sin_port);
        memcpy(&fromIP, &(from.sin_addr.s_addr), sizeof(in_addr));
    }
    return result;
}
/**
 * bind
 *    This is used by servers to specify the port through which
 *    they will receive messages and, in our case, that any IP address
 *    can send them data.
 *  @param port - the port (in host byte order) on which to receive the
 *                data
 *  @throws std::system_error encapsulating errno on bind failure.
 */
void CUDP::bind(short port)
{
    struct sockaddr_in sAddr;
    memset(&sAddr, 0, sizeof(sAddr));
    sAddr.sin_family = AF_INET;
    sAddr.sin_port = htons(port);
    sAddr.sin_addr.s_addr = INADDR_ANY;
    
    int stat =  ::bind(
        m_socket, reinterpret_cast<sockaddr*>(&sAddr), sizeof(sAddr)
    );

    if (stat < 0) {
        throw std::system_error(errno, std::generic_category());
    }
}

// void CUDP::closeServer() {
//     std::cout<<"Simon - CUDP in close()"<<std::endl;
//     if (m_socket != -1) {
//         std::cout<<"Simon - CUDP in close() m_socket != -1"<<std::endl;
//         int stat = ::close(m_socket);
//         if (stat < 0) {
//             std::cerr << "Error closing socket: " << strerror(errno) << std::endl;
//         }
//         m_socket = -1;  // Invalidate the socket descriptor
//     }
// }


}                     // UDP Namespace