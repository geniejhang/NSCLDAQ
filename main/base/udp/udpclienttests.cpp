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

/** @file:  udpclienttests.cpp
 *  @brief: Test UDP::CUDPClient class.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CUDPServer.h"
#include "CUDPClient.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <string.h>

static const char* localhost = "127.0.0.1";
static short port=32000;

class udpclienttest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(udpclienttest);
    CPPUNIT_TEST(send_1);
    CPPUNIT_TEST(send_rcv);
    CPPUNIT_TEST_SUITE_END();
    
private:
    UDP::CUDPServer* m_pServer;
    UDP::CUDPClient* m_pClient;
public:
    void setUp() {
        
        m_pServer = new UDP::CUDPServer(port);
        m_pClient = new UDP::CUDPClient(inet_addr(localhost), port);
    }
    void tearDown() {
        delete m_pServer;
        delete m_pClient;
    }
protected:
    void send_1();
    void send_rcv();
};

CPPUNIT_TEST_SUITE_REGISTRATION(udpclienttest);

// send ping to server.  It receives it.

void udpclienttest::send_1()
{
    const char* ping = "ping";
    size_t nBytes = strlen(ping) + 1;  // include null.
    
    EQ(nBytes, size_t(m_pClient->send(ping, nBytes)));
    
    in_addr_t fromIP;
    short     fromPort;
    char message[8192];
    EQ(nBytes, size_t(m_pServer->receive(message, sizeof(message), fromIP, fromPort)));
    
    EQ(0, strcmp(ping, message));
    
}
// send ping to server it receives it and sends it back to the client:

void udpclienttest::send_rcv()
{
    const char* ping = "ping";
    size_t nBytes = strlen(ping) + 1;  // include null.
    
    EQ(nBytes, size_t(m_pClient->send(ping, nBytes)));
    
    in_addr_t fromIP;
    short     fromPort;
    char message[8192];
    int rcvBytes = m_pServer->receive(message, sizeof(message), fromIP, fromPort);
    
    // send it back: send_1 verifies we'll get it.
    
    char echomsg[8192];
    in_addr_t serverIP;
    short     serverPort;
    
    m_pServer->send(message, rcvBytes, fromIP, fromPort);
    EQ(nBytes, size_t(m_pClient->receive(echomsg, sizeof(echomsg), serverIP, serverPort)));
    EQ(0, strcmp(ping, echomsg));
    
}