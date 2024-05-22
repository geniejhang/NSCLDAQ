#include "CTcpClient.h"

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

CTcpClient::CTcpClient() {
    sockfd = -1;
}

CTcpClient::~CTcpClient() {
    close(sockfd);
}

void CTcpClient::setAddressAndPort(const std::string& serverIP, int port) {
    if (sockfd != -1) { 
        std::cerr << "socket already created. Cannot change address and port."<< sockfd << std::endl;
        return;
    }
    // Create a TCP socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    if (inet_pton(AF_INET, serverIP.c_str(), &servaddr.sin_addr) <= 0) {
        perror("invalid address / address not supported");
        return;
    }
}

bool CTcpClient::connectToServer() {
    return connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) >= 0;
}

bool CTcpClient::sendCommand(const std::string& command) {
    return send(sockfd, command.c_str(), command.length(), 0) >= 0;
}

std::string CTcpClient::receiveResponse() {
    char buffer[1024];
    int bytesReceived = recv(sockfd, buffer, sizeof(buffer), 0);
    if (bytesReceived < 0) {
        perror("recv failed");
        return "";
    }
    buffer[bytesReceived] = '\0';
    return std::string(buffer);
}