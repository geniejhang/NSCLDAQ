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

/** @file:  CUDPClient.cpp
 *  @brief: Implement the UDP::CUDPClient class.
 */
#include "CUDPClient.h"

namespace UDP {
    /**
     * constructor
     *    @param  targetHost - IP address of the target host.
     *    @param  targetPort - Port in target system to receive the data
     *
     *  @note - since targetHost most simply comes from network routines
     *          that already  provide it in network byte order it's expected
     *          to be in network byte order.  The targetPort, however is
     *          expected in host byte order.
     */
    CUDPClient::CUDPClient(in_addr_t targetHost, short targetPort) :
        m_targetHost(targetHost),
        m_targetPort(targetPort)
    {}
    /**
     * destructor
     */
    CUDPClient::~CUDPClient()
    {}
    /**
     * send
     *    Send data to the target.
     * @param pData - data to send.
     * @param nBytes - amount of data to send.
     * @return int - value from CUDP::send
     */
    int
    CUDPClient::send(const void* pData, size_t nBytes)
    {
        return CUDP::send(pData, nBytes, m_targetHost, m_targetPort);
    }
    /**
     * receive
     *    Just a jacket that re-exposes CUDP::receive:
     *  @param pData - pointer to data buffer.
     *  @param nBytes - Size of data buffer.
     *  @param fromIp- filled in with source IP address.
     *  @param fromPort - Filled in with source port.
     */
    int
    CUDPClient::receive(
        void* pData, size_t nBytes,
        in_addr_t&  fromIP, short& fromPort
    )
    {
        return CUDP::receive(pData, nBytes, fromIP, fromPort);
    }
}