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
#ifndef CUDPSERVER_H
#define CUDPSERVER_H
#include "UDP.h"

namespace UDP {
    
    /**
     * @class CUDPSever
     *    Inherits from CUDP but only exposes the send and receive
     *    method since we do the Bind at consturction time.
     */
    class CUDPServer : protected CUDP {
    public:
        CUDPServer(short port);
        virtual ~CUDPServer();
        
        // delegations to parent class that we re-expose here:
        
        int send(
            const void* pData, size_t nBytes,
            in_addr_t& ipAddress, short port
        );
        int receive(
            void* pData, size_t nBytes,
            in_addr_t&  fromIP, short& fromPort
        );
        
    };
}

#endif