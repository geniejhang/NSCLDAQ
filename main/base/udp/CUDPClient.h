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

/** @file:  CUDPServer.h
 *  @brief: Support class for a UDP server.
 */
#ifndef CUDPCLIENT_H
#define CUDPCLIENT_H
#include "UDP.h"

namespace UDP {
    
    /**
     * @class CUDPClient
     *   Provides a UDP client for a single host/port pair.
     */
    class CUDPClient : protected CUDP {
    private:
        in_addr_t m_targetHost;
        short     m_targetPort;
        
    public:
        CUDPClient(in_addr_t targetHost, short targetPort);
        virtual ~CUDPClient();
        
        int send(const void* pData, size_t nBytes);
        int receive(
            void* pData, size_t nBytes, in_addr_t& fromHost, short& fromPort
        );
    };
}

#endif