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
# @file   CPublishRingStatistics.h
# @brief  Define class t publish ring statistics to a ZMQ socket.
# @author <fox@nscl.msu.edu>
*/

#ifndef CPUBLISHRINGSTATISTICS_H
#define CPUBLISHRINGSTATISTICS_H
#include <zmq.hpp>
#include <CRingBuffer.h>
#include <vector>
#include <string>
#include <map>


class CTCLInterpreter;
class CTCLObject;

/**
 * @class CPublishRingStatistics
 *
 *     This class obtains the current set of ring buffer statistics
 *     and publishes them to the specified zmq socket.  Note that publish,
 *     in this case has a meaning defined by the underlying zmq socket type,
 *     not necessarily a PUB.
 */

class CPublishRingStatistics
{
private:
    typedef struct _Usage {
        CRingBuffer::Usage                     s_usage;
        std::string                            s_ringName;
        std::vector<std::string>               s_producerCommand;
        std::vector<std::vector<std::string> > s_consumerCommands;
        std::vector<bool>                      s_logged;
    } Usage, *pUsage;
 
private:
    zmq::socket_t*   m_pSocket;                  // Publication socket.
    std::string      m_appName;
    std::map<std::string, Usage>  m_history;
    
public:
    CPublishRingStatistics(zmq::socket_t& socket, std::string appName);
    virtual ~CPublishRingStatistics();
    
public:
    void operator()();                         // Does the actual publications.
    
    //utilities:
    
private:
    std::vector<Usage> usageTextToVector(std::string& usage);
    Usage              itemToUsage(CTCLInterpreter& interp, CTCLObject& obj);
    void publish(std::vector<Usage>& usage);
    bool logLargeBacklog(const Usage& ringUsage, size_t index);
    bool logBacklogOk(const Usage& ringUsage, size_t index);
    static std::string makeBacklogMessage(
        std::string body, std::vector<std::string> command,
        size_t ringSize, size_t backlog
    );

    static std::pair<bool, size_t> getHistoryIndex(
        const Usage& ringUsage, const Usage& history, size_t index
    );
    void updateRingHistory(const Usage& ringUsage);
    
    bool lastLoggedValue(const Usage& ringUsage, size_t index);
};


#endif