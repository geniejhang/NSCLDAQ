/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   CStatusReporting
# @brief  Class specialized to do status reporting for readout programs.
# @author <fox@nscl.msu.edu>
*/

#include "CStatusReporting.h"
#include <stdexcept>
#include <sstream>
#include <CPortManager.h>
#include <Exception.h>
#include <CStatusMessage.h>

// The instance for singleton use:

CStatusReporting* CStatusReporting::pInstance(nullptr);

/**
 * constructor
 *    - Create the ZMQ socket.
 *    - Create the statistics object.
 *    - Create the logger.
 */
CStatusReporting::CStatusReporting(const char* application, const char* aggregator)
{
    // Create the zmq socket and connect it to the aggregator:
    
    int port = aggregatorPort(aggregator);
    m_pSocket = connectSocket(port);
    
    // Make the loggin objects:
    
    m_pStatistics = new CStatusDefinitions::ReadoutStatistics(*m_pSocket, application);
    m_pLogger     = new CStatusDefinitions::LogMessage(*m_pSocket, application);
}
/**
 * destructor
 *    destroy the dynamically allocated resources:
 */
CStatusReporting::~CStatusReporting()
{
    delete m_pStatistics;
    delete m_pLogger;
    delete m_pSocket;
}
/**
 * log
 *    Make a generic log message.
 *  @param severity - message severity
 *  @param message  - Text of the message.
 */
void
CStatusReporting::log(std::uint32_t severity, const char* msg)
{
    m_pLogger->Log(severity, msg);
}
/**
 * logBegin
 *    Log a begin run message.
 *
 *  @param run - run number
 *  @param title - Run Title.
 */
void
CStatusReporting::logBegin(std::uint32_t run, const char* title)
{
    m_pStatistics->beginRun(run, title);
}
/**
 * logStatistics
 *    Log the statistics within the current run:
 *
 *  @param triggers  - number of triggers seen.
 *  @param events    - Number of events emitted.
 *  @param bytes     - Number of bytes of event data emitted.
 */
void
CStatusReporting::logStatistics(
    std::uint64_t triggers, std::uint64_t events, std::uint64_t bytes
)
{
    m_pStatistics->emitStatistics(triggers, events, bytes);
}
/*-----------------------------------------------------------------------------
 *  Private utility methods.
 */


/**
 * aggregatorPort
 *    Determine the port on which the aggregator is running.
 *
 *  @param service -aggregator service name.
 *  @return int    - port number on which the aggregator is listening.
 *  @throw std::runtime_error - Problems finding the aggregator port.
 */
int
CStatusReporting::aggregatorPort(const char* service)
{
    try {
        CPortManager portmgr;
        std::vector<CPortManager::portInfo> ports = portmgr.getPortUsage();
        for (size_t i = 0; i < ports.size(); i++) {
            if (ports[i].s_Application == std::string(service)) {
                return ports[i].s_Port;
            }
        }
        throw std::runtime_error("Status aggregation service is not running");
    }
    catch(CException& e) {
        std::string msg = "Unable to determine status aggregation service port :";
        msg += e.ReasonText();
        throw std::runtime_error(msg);
    }
}
/**
 * connectSocket
 *    Create a zmq push socket and connecte it to the status aggregator.
 *
 *  @param port - port on which the status aggregator is listening.
 *  @return zmq::socket_t*  - the created socket.
 */
ZmqSocket*
CStatusReporting::connectSocket(int port)
{
    std::ostringstream uri;
    uri << "tcp://localhost:" << port;
    ZmqSocket* result = ZmqObjectFactory::createSocket(ZMQ_PUSH);
    (*result)->connect(uri.str().c_str());
    
    return result;
}
