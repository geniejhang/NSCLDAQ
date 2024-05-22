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

/** @file:  udpservertests.cpp
 *  @brief: Tests for the CUDPserver class.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CUDPServer.h"
#include "UDP.h"
#include <system_error>
#include <string>


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static const char* pHost="127.0.0.1";    // lOCALHOST.
static short port = 32000;


class udpservertest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(udpservertest);
    CPPUNIT_TEST(construct_1);
    CPPUNIT_TEST_SUITE_END();
    
private:
    UDP::CUDPServer* m_pServer;
public:
    void setUp() {
        m_pServer = new UDP::CUDPServer(port);
    }
    void tearDown() {
        delete m_pServer;
    }
protected:
    void construct_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(udpservertest);

void udpservertest::construct_1()
{
    // Send data to the server:
    
    uint8_t data[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    in_addr_t destination = inet_addr(pHost);
    uint8_t rcvdata[8192];
    in_addr_t host;
    short srcport;
    
    UDP::CUDP client;
    EQ(sizeof(data), size_t(client.send(data, sizeof(data), destination, port)));
    
    int s = m_pServer->receive(rcvdata, sizeof(rcvdata), host, srcport);
    EQ(sizeof(data), size_t(s));
    EQ(destination, host);
    for (int i =0; i < sizeof(data); i++) {
        EQ(data[i], rcvdata[i]);
    }
    
    
}