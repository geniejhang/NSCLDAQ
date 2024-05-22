#ifndef CTCPCLIENT_H
#define CTCPCLIENT_H

#include <string>
#include <arpa/inet.h>

class CTcpClient {
public:
    CTcpClient();
    ~CTcpClient();

    void setAddressAndPort(const std::string& serverIP, int port);

    bool connectToServer();
    bool sendCommand(const std::string& command);
    std::string receiveResponse();

private:
    int sockfd;
    struct sockaddr_in servaddr;
};

#endif