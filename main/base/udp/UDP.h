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

/** @file:  UDP.h
 *  @brief: Provides common code used by both a UDP server and client.
 */
#ifndef UDP_H
#define UDP_H
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// #include <csignal>


namespace UDP {
    /**
     * @class CUDP
     *   Provides encapsulation of a UDP socket with common
     *   stuff needed by clients _and_ servers.
     */
    class CUDP {
    private:
        int m_socket;                      // Encapsulated socket.
        // static void signalHandler(int signal, siginfo_t* info, void* context) { 
        //     CUDP::instance().closeServer();
        // }
    public:
        CUDP();                           // Creates the UDP socket.
        virtual ~CUDP();                  // closes the UDP socket.

        static CUDP& instance() {
            static CUDP instance; 
            return instance;
        }
        
        int send(
            const void* pData, size_t nBytes,
            in_addr_t& ipAddress, short port
        );
        int receive(
            void* pData, size_t nBytes,
            in_addr_t&  fromIP, short& fromPort
        );
        
        void bind(short port);
        // void closeServer();
    };
}

#endif