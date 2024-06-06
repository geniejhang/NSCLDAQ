#ifndef UDPBROKERBASE_H
#define UDPBROKERBASE_H

#include <CDataSink.h>
// #include <CRingItem.h>
#include <netinet/in.h> // for in_addr_t
// #include <arpa/inet.h>
// #include <iostream>
#include <string>
#include <memory> // for smart pointer
#include <map>
// #include <vector>


class UDPBrokerBase {
public:
    UDPBrokerBase();
    ~UDPBrokerBase ();

    virtual void initialize(short port);
    virtual void addSink(std::string sinkType, int sid);
    virtual void run();
    // const std::map<int, std::unique_ptr<CDataSink>>& getSinks();
    int getSocket();

    // std::map<int, std::unique_ptr<CDataSink>> m_dataSinks;


protected:
    virtual void mainLoop();
    virtual void makeRingItem(in_addr_t from, short port, uint8_t* datagram, size_t nBytes);
    
    //complication if declared private, need getter function returning const which is then not convenient in derived class...
    std::map<int, std::unique_ptr<CDataSink>> m_dataSinks;

private:
    int m_socket;
    short m_port;
    // std::map<int, std::unique_ptr<CDataSink>> m_dataSinks;  

};

#endif
