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
# @brief  This class provides status reporting.
# @author <fox@nscl.msu.edu>
*/

#ifndef     CSTATUSREPORTING_H
#define     CSTATUSREPORTING_H

#include <CStatusMessage.h>
#incluce <string>
#include <zmq.hpp>

/**
 * @class CStatusReporting
 *     Encapsulates status message reporting for the readout subsystems.
 *     Normally the startup software will create one of these and save a pointer
 *     to it in the public member CStatusReporting::pInstance.
 *     The instance provides a simplified interface to the status messaging
 *     system tailored to the needs of readout programs.
 */s
class CStatusReporting
{
public:
    static CStatusReporting*               pInstance;
private:
    CStatusDefinitions::ReadoutStatistics* m_pStatistics;
    CStatusDefinitions::LogMessage*        m_pLogger;
    zmq::socket_t*                         m_pSocket;
    
public:
    CStatusReporting(const char* application, const char* aggregator);
    virtual ~CStatusReporting();
    
    // logging methods:

public:
    void log(std::uint32_t severity, const char* msg);
    
    void logBegin(std::uint32_t run, const char* title);
    void logStatistics(std::uint64_t triggers, uint64_t events, uint64_t bytes);
    
    // Utilities:
    
private:
    int aggregatorPort(const char* service);
    zmq::socket_t* connectSocket(int port);
};

#endif