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
# @file   CPublishRingStatistics.cpp
# @brief  Implement class to publish ring buffer statistics.
# @author <fox@nscl.msu.edu>
*/

#include "CPublishRingStatistics.h"
#include "CStatusMessage.h"
#include <os.h>
#include <CRingMaster.h>
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <stdexcept>
#include <sstream>
#include <set>

/// These backlogs are different to prevent rapid flow of messages.

static const double BACKLOG_LOG_THRESHOLD(0.9);
static const double BACKLOG_OK_THRESHOLD(0.8);        

/**
 * constructor:
 *    Just salt away the socket for use when we publish.
 *
 *  @param socket - reference to the zmq::socket_t on which we'll send messages.
 *  @param appName - Application name sent in messages.
 */
CPublishRingStatistics::CPublishRingStatistics(
    zmq::socket_t& socket, std::string appName
) :
    m_pSocket(&socket),
    m_appName(appName)
{}

/**
 * destructor
 *   At this point our client is responsible for the disposition of the socket
 *   so this is a no-op:
 */
CPublishRingStatistics::~CPublishRingStatistics()
{}

/**
 * operator()
 *    Actually publish the data:
 *    - Obtain the ring buffer usage.
 *    - If necessary, create a CStatusDefinitions::RingStatistics object
 *    - If necessary, push messages for all the rings through that object.
 *  @note 'If necessary' above means that the number of rings in existence
 *        is non-zero.
 */
void
CPublishRingStatistics::operator()()
{
    CRingMaster rmaster;                        // Only want for the localhost.
    std::string usage = rmaster.requestUsage();
    std::vector<Usage> usageVector                   = usageTextToVector(usage);
    publish(usageVector);
    
}

/*---------------------------------------------------------------------------
 *  Private utilities:
 */

/**
 * usageTextToVector
 *    Convert the usage text from the ring master into a vector
 *    Usage structs.  ring master text is a Tcl list of lists
 *    where each sublist has:
 *    -  Name of the ringbuffer
 *    -  List containing statistics, which is as follows:
 *       * Buffer size.
 *       * Bytes avavilable
 *       * Number of consumers allowed
 *       * Producer PID (-1 if none).
 *       * max get space
 *       * min get space.
 *       * List of consumer pids, backlogs
 *       * List of statistics containing
 *         # producer status (puts and bytes).
 *         # For each consumer a triple of the pid, get count and bytes.
 *
 *   @param usageString - Usage string from the ringmaster.
 *   @return std::vector<Usage> - struct that contains the ring buffer statistics.
 */
std::vector<CPublishRingStatistics::Usage>
CPublishRingStatistics::usageTextToVector(std::string& usage)
{
    std::vector<Usage> result;
    
    // Convert the usage list into a Tcl object:
    
    CTCLInterpreter interp;
    CTCLObject      usageList;
    usageList.Bind(interp);
    usageList = usage;
    
    for (int i = 0; i < usageList.llength(); i++) {
        
        // Got an item.
        
        CTCLObject item;
        item.Bind(interp);
        item = usageList.lindex(i);
        
        Usage oneUsage = itemToUsage(interp, item);
        result.push_back(oneUsage);
    }
    
    return result;
}
/**
 * itemToUsage
 *    Take a single ring buffer item and turn it into a Usage struct.
 *
 *  @param interp - Tcl Interpreter to use to parse items.
 *  @param obj    - Single list item object.  See above
 *                  for contents.
 *  @return CPublishRingStatistics::Usage struct.
 */
CPublishRingStatistics::Usage
CPublishRingStatistics::itemToUsage(CTCLInterpreter& interp, CTCLObject& obj)
{
    Usage result;
    
    CTCLObject oRingName  = obj.lindex(0);
    CTCLObject oRingStats = obj.lindex(1);
    oRingName.Bind(interp);
    oRingStats.Bind(interp);
    
    result.s_ringName = std::string(oRingName);
    
    result.s_usage.s_bufferSpace = int(oRingStats.lindex(0));
    result.s_usage.s_putSpace    = int(oRingStats.lindex(1));
    result.s_usage.s_maxConsumers= int(oRingStats.lindex(2));
    result.s_usage.s_producer    = int(oRingStats.lindex(3));
    result.s_usage.s_maxGetSpace = int(oRingStats.lindex(4));
    result.s_usage.s_minGetSpace = int(oRingStats.lindex(5));
    
    CTCLObject oConsumers = oRingStats.lindex(6);
    oConsumers.Bind(interp);
    
    // Consumer backlogs:
    
    for (int i = 0; i < oConsumers.llength(); i++) {
        CTCLObject item = oConsumers.lindex(i);
        item.Bind(interp);
        CTCLObject oPid     = item.lindex(0);
        CTCLObject oBacklog = item.lindex(1);
        oPid.Bind(interp);
        oBacklog.Bind(interp);
        
        std::pair<pid_t, size_t> consumer;
        consumer.first = int(oPid);
        consumer.second = int(oBacklog);
        result.s_usage.s_consumers.push_back(consumer);
    }
    // Producer statistics:
    
    CTCLObject oPstats = oRingStats.lindex(7);     // Producer statistics.
    oPstats.Bind(interp);
    CTCLObject ops;
    ops.Bind(interp);
    CTCLObject bytes;
    bytes.Bind(interp);
    
    ops = oPstats.lindex(0);
    bytes = oPstats.lindex(1);
    result.s_usage.s_producerStats.s_pid = result.s_usage.s_producer;
    result.s_usage.s_producerStats.s_transfers = double(ops);
    result.s_usage.s_producerStats.s_bytes     = double(bytes);
    if (result.s_usage.s_producer != -1) {
        result.s_producerCommand =
            Os::getProcessCommand(result.s_usage.s_producer);
    }
    
    CTCLObject oCstats;                    // Consumer statistics
    oCstats.Bind(interp);
    oCstats = oRingStats.lindex(8);
    for (int i = 0; i < oCstats.llength(); i++) {
        CTCLObject item;
        item.Bind(interp);
        item = oCstats.lindex(i);
        
        CTCLObject oPid;
        oPid.Bind(interp);
        oPid = item.lindex(0);
        ops  = item.lindex(1);
        bytes = item.lindex(2);
        
        CRingBuffer::clientStatistics client;
        client.s_pid = int(oPid);
        client.s_transfers= double(ops);
        client.s_bytes = double(bytes);
        result.s_usage.s_consumerStats.push_back(client);
        if (client.s_pid != -1) {
            result.s_consumerCommands.push_back(
                Os::getProcessCommand(client.s_pid)
            );
            result.s_logged.push_back(false);          // Assume not logged.
        }
        
    }
    
    return result;
}
/**
 * publish
 *    Perform the actual publication.
 *    - Constructs the RingStatistics object,
 *    - Iterates over the ring information and sends message clumps for each ring.
 *
 * @param usage - The list of usages:
 */
void
CPublishRingStatistics::publish(std::vector<Usage>& usage)
{
    // We can emit two types of messages:
    // We unconditionally will emit ring usage messages.
    // If a large backlog is detected for a consumer we'll emit that as well.
    // To do the latter, we need some history for the consumers.
    
    CStatusDefinitions::RingStatistics publisher(*m_pSocket, m_appName);
    CStatusDefinitions::LogMessage     logger(*m_pSocket, m_appName);  
    std::set<std::string>              ringNames;
    for (int i = 0; i < usage.size(); i++) {
        Usage& item(usage[i]);
        
        publisher.startMessage(item.s_ringName);
        ringNames.insert(item.s_ringName);
        
        // Add producer information if there's a producer:
        
        if (item.s_usage.s_producer != -1) {
            publisher.addProducer(
                item.s_producerCommand,
                item.s_usage.s_producerStats.s_transfers,
                item.s_usage.s_producerStats.s_bytes,
                item.s_usage.s_producer
            );
        }
        // add any and all consumers:
        
        for (int c = 0; c < item.s_consumerCommands.size(); c++) {
            CRingBuffer::clientStatistics&
                stats(item.s_usage.s_consumerStats[c]);
            publisher.addConsumer(
                item.s_consumerCommands[c], stats.s_transfers, stats.s_bytes,
                item.s_usage.s_consumers[c].second,             // Backlog
                stats.s_pid
            );
            // Note the message parts don't get sent until the endMessage
            // method.  Therefore we can intersperse a log message as needed:
            
            item.s_logged[c] = lastLoggedValue(item, c);
            
            if(logLargeBacklog(item, c)) {
                logger.Log(
                    CStatusDefinitions::SeverityLevels::WARNING,
                    makeBacklogMessage(
                        item.s_ringName + " Ring free space is low",
                        item.s_consumerCommands[c],
                        item.s_usage.s_bufferSpace, item.s_usage.s_consumers[c].second
                    )
                );
                item.s_logged[c] = true;
            }
            if (logBacklogOk(item, c)) {
                logger.Log(
                    CStatusDefinitions::SeverityLevels::INFO,
                    makeBacklogMessage(
                            item.s_ringName + " Ring free space is ok again",
                            item.s_consumerCommands[c],
                            item.s_usage.s_bufferSpace, item.s_usage.s_consumers[c].second
                    )
                );
                item.s_logged[c] = false;         // Switch back to unlogged stat.
            }
        }
        publisher.endMessage();                   // Send the message.

        updateRingHistory(item);
    }
    // Trim the ring names that don't exist anymore in the history:
    // Careful...you can't just kill off nodes in this loop since erase
    // invalidates the iterator:
    
    std::set<std::string> killTheseNames;
    for (auto p = m_history.begin(); p != m_history.end(); p++) {
        if (ringNames.count(p->first) == 0) {
            killTheseNames.insert(p->first);
        }
    }
    // killTheseNames is the set of names to remove from the map:
    
    for (auto p = killTheseNames.begin(); p != killTheseNames.end(); p++) {
        auto mp = m_history.find(*p);
        m_history.erase(mp);                // We know it's in the map.
    }
    
}
/**
 * logLargeBackLog
 *    Returns true if the daemon should log a backlog large message for a consumer
 *    of a ring.  This happens if the backlog is above BACKLOG_LOG_THRESHOLD
 *    and one of two conditions is also true:
 *    -  There is no history entry for the consumer.
 *    -  There is a history entry for the consumer and we've not yet logged
 *       this. condition.
 *
 *  @param   ringUsage   - Current usage of the ringbuffer.
 *  @param   index       - Index of consumer in the current usage.
 *  @return  bool        - True if logging is needed.
 
 */
bool
CPublishRingStatistics::logLargeBacklog(const Usage& ringUsage, size_t index)
{
    // don't do anything if the backlog is lower than the threshold:
    
    double ringSize = ringUsage.s_usage.s_bufferSpace;
    double backlog  = ringUsage.s_usage.s_consumers[index].second;
    if (backlog/ringSize > BACKLOG_LOG_THRESHOLD) {
        std::map<std::string, Usage>::iterator pH = m_history.find(ringUsage.s_ringName);
        if (pH == m_history.end()) {
            // ring has no prior history so:
            
            return true;
        }
        Usage& history(pH->second);
        std::pair<bool, size_t> clientHistoryInfo =
            getHistoryIndex(ringUsage, history, index);
        if(!clientHistoryInfo.first) {
            // no prior history entry for consumer.
            
            return true;
        }
        return !history.s_logged[clientHistoryInfo.second];   // Log if not logged yet.
        
      
    } else  {
        return false;
    }
    // Should not get here:
    
    throw std::logic_error("CPublishRingStatistics::logLargeBacklog - fell through if");
}
/**
 * logBacklogOk
 *   Determines if it's time to log a message that a backlog is ok.  This happens
 *   when the backlog is lower than BACKLOG_OK_THRESHOLD and there's a history
 *   entry indicating that we already logged a backlog high.
 *
 *   @param ringUsage - Ring usage for the ring buffer.
 *   @param index     - Client index in the ring usage parameter. Note that this
 *                      need not be the same as the client index (if any)
 *                      in the history map.
 *   @return bool - true if a backlog ok log message should be emitted.
 */
bool
CPublishRingStatistics::logBacklogOk(const Usage& ringUsage, size_t index)
{
    double ringSize = ringUsage.s_usage.s_bufferSpace;
    double backlog  = ringUsage.s_usage.s_consumers[index].second;
    if (backlog/ringSize < BACKLOG_OK_THRESHOLD) {
        std::map<std::string, Usage>::iterator pH = m_history.find(ringUsage.s_ringName);
        if (pH == m_history.end()) {
            return false;                      // Never logged.
        }
        Usage& history(pH->second);
        std::pair<bool, size_t> clientHistoryInfo =
        getHistoryIndex(ringUsage, history, index);
        if(!clientHistoryInfo.first) {
            return false;                    // Never logged - no client history.
        }
        return history.s_logged[clientHistoryInfo.second];  // True if logged.
        
    } else {
        return false;                          // Backlog too high.
    }
}
/**
 *  makeBacklogMessage
 *    Create a backlog message from:
 *
 * @param body      - Message body
 * @param command   - vector of the command words.
 * @param ringSize  - Bytes in the ring buffer in bytes.
 * @param backlog   - Backlog in bytes.
 * @return std::string - the built message.
 */
std::string
CPublishRingStatistics::makeBacklogMessage(
    std::string body, std::vector<std::string> command,
    size_t ringSize, size_t backlog
)
{
    // Put the command back together:
    
    std::string assembledCommand;
    for (int i = 0; i < command.size(); i++) {
        assembledCommand += command[i];
        assembledCommand += " ";
    }
    std::ostringstream msgStream;
    
    msgStream << body << " Consumer command " << assembledCommand
        << " backlog is " << 100.0*backlog/ringSize << "%";
        
    return msgStream.str();
        
}
/**
 * getHistoryIndex
 *    Given a usage consumer index, find the corresponding consumer in the
 *    history entry for that ring; or determine there's no match.  A match
 *    occurs when the consumer PID and command are the same.
 *
 *  @note Since ring consumers can change with time, it's not always the case
 *        that the indices will be the same between history and present.
 
 *
 *  @param usage   - the usage entry.
 *  @param history - The history entry for the corresponding ring
 *  @param uindex  - Consumer index in the usage struct.
 *  @return std::pair<bool, size_t> - The element indicates whether or not
 *                    a corresponding consumer was found and is true if so.
 *                    the second element is only meaningful if the first is true.
 *                    It is the index of the corresponding consumer.
 */
std::pair<bool, size_t>
CPublishRingStatistics::getHistoryIndex(const Usage& usage, const Usage& history, size_t uindex)
{
    std::pair<bool, size_t> result(false, 0);
    
    // Here's what we care about from usage:
    
    pid_t cPid = usage.s_usage.s_consumers[uindex].first;
    const std::vector<std::string>& cCommand(usage.s_consumerCommands[uindex]);
    
    // Hunt for matching info in the history entry:
    
    for (int i = 0; i < history.s_consumerCommands.size(); i++) {
        if (
            (cPid == history.s_usage.s_consumers[i].first) &&
            (cCommand == history.s_consumerCommands[i])
        ) {
            result.first  = true;
            result.second = i;
            return result;
        }
    }
    return result;
}
/**
 * updateRingHistory
 *    Replaces or creates a new ring history for a named ring.
 *
 *  @param  ringUsage - a ring usage entry.
 */
void
CPublishRingStatistics::updateRingHistory(const Usage& ringUsage)
{
    m_history[ringUsage.s_ringName] = ringUsage;
}
/**
 * lastLoggedValue
 *    Determines if a specific consumer has already logged a large backlog
 *    message:
 *
 *  @param ringUsage - a ring usage struct.
 *  @param index     - Consumer index.
 *  @return bool     - true if logged, false if not.
 */
bool
CPublishRingStatistics::lastLoggedValue(const Usage& ringUsage, size_t index)
{
    // If there's no entry for the ring in the map... that's equivalent to false:
    
    std::map<std::string, Usage>::iterator p = m_history.find(ringUsage.s_ringName);
    if(p == m_history.end()) {
        return false;
    }
    // If there's no identical client, that's also a false:
    
    std::pair<bool, size_t> idxInfo = getHistoryIndex(ringUsage, p->second, index);
    if (!idxInfo.first) {
        return false;
    }
    return p->second.s_logged[idxInfo.second];         // The actual value.
}