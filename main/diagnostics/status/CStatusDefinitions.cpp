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
# @file   CStatusDefinitions.cpp
# @brief  Executable members of the outer CStatusDefinitions class
# @author <fox@nscl.msu.edu>
*/

#include "CStatusMessage.h"
#include <string>
#include <map>
#include <os.h>
#include <stdexcept>
#include <cstring>
#include <ctime>

static std::map<std::string, std::uint32_t> messageTypeLookup = {
        {"RING_STATISTICS", CStatusDefinitions::MessageTypes::RING_STATISTICS},
        {"EVENT_BUILDER_STATISTICS",
                CStatusDefinitions::MessageTypes::EVENT_BUILDER_STATISTICS},
        {"READOUT_STATISTICS", CStatusDefinitions::MessageTypes::READOUT_STATISTICS},
        {"LOG_MESSAGE", CStatusDefinitions::MessageTypes::LOG_MESSAGE},
        {"STATE_CHANGE", CStatusDefinitions::MessageTypes::STATE_CHANGE}
    };
static std::map<uint32_t, std::string> messageTypeStringLookup = {
        {CStatusDefinitions::MessageTypes::RING_STATISTICS, "RING_STATISTICS"},
        {CStatusDefinitions::MessageTypes::EVENT_BUILDER_STATISTICS,
            "EVENT_BUILDER_STATISTICS",
        },
        {CStatusDefinitions::MessageTypes::READOUT_STATISTICS, "READOUT_STATISTICS"},
        {CStatusDefinitions::MessageTypes::LOG_MESSAGE, "LOG_MESSAGE"},
        {CStatusDefinitions::MessageTypes::STATE_CHANGE, "STATE_CHANGE"}
    
};


static std::map<std::string, uint32_t> messageSeverityLookup = {
        {"DEBUG", CStatusDefinitions::SeverityLevels::DEBUG},
        {"INFO", CStatusDefinitions::SeverityLevels::INFO},
        {"WARNING", CStatusDefinitions::SeverityLevels::WARNING},
        {"SEVERE", CStatusDefinitions::SeverityLevels::SEVERE},
        {"DEFECT", CStatusDefinitions::SeverityLevels::DEFECT}
};

static std::map<uint32_t, std::string> messageSeverityStringLookup = {
        {CStatusDefinitions::SeverityLevels::DEBUG, "DEBUG"},
        {CStatusDefinitions::SeverityLevels::INFO, "INFO"},
        {CStatusDefinitions::SeverityLevels::WARNING, "WARNING"},
        {CStatusDefinitions::SeverityLevels::SEVERE, "SEVERE"},
        {CStatusDefinitions::SeverityLevels::DEFECT, "DEFECT"}
};

/**
 * sizeStringList
 *    Return the storage required for a list of strings when it is
 *    flattened e.g. turned into "string1\0string2\0...stringn\0\0"
 * @param strings - vector of strings to be flattened.
 */
size_t
CStatusDefinitions::sizeStringList(const std::vector<std::string>& strings)
{
    size_t result(0);
    for (int i = 0; i < strings.size(); i++) {
        result += strings[i].size() + 1;                // +1 for \0
    }
    result++;                                          // +1 for \0 sentinel.
    return result;
}
/**
 * sizeStringList
 *     Return the storage used by a flattened list of strings.  See above
 *     for what that looks like in memory:
 */
size_t
CStatusDefinitions::sizeStringList(const char* strings)
{
    size_t result = 0;
    while ( *strings) {
        size_t slen = std::strlen(strings) + 1;   // +1 for \0 terminator.
        result  += slen;
        strings += slen;                           // next string.
    }
    result++;                                     // count the end sentinell.
    return result;
}
/**
 * copyStrings
 *    Flattens a string list (vector) into a char* stroage.
 *  @param pDest  - pointer to destination - caller must ensure it's big enough.
 *  @param strings - Vector of strings to flatten.
 */
void
CStatusDefinitions::copyStrings(
    char* pDest, const std::vector<std::string>& strings
)
{
    
    for (int i = 0; i < strings.size(); i++) {
        std::strcpy(pDest, strings[i].c_str());
        pDest += strings[i].size() + 1;          // Count the null.
    }
    *pDest = '\0';                             // Finalizing sentinnel.
}
/**
 * stringListToVector
 *    Convert a string list to a vector of strings.
 *
 *  @param strings - pointer to the strings in flattened form.
 *  @return std::vector<std::string> - vector of strings in the flattened list.
 */
std::vector<std::string>
CStatusDefinitions::stringListToVector(const char* strings)
{
    std::vector<std::string> result;
    while (* strings) {    
        std::string s(strings);
        result.push_back(s);
        strings += s.size() + 1;
    }
    return result;
}

/**
 * messageTypeToString
 *    Convert a message type value to a string.
 *
 *  @param type         - the message type value.
 *  @return std::string - the stringified version of it.
 *  @throw std::invalid_argument if the type value is invalid.
 */
std::string
CStatusDefinitions::messageTypeToString(uint32_t type)
{
    auto p = messageTypeStringLookup.find(type);
    
    // If not found throw:
    
    if (p == messageTypeStringLookup.end()) {
        throw std::invalid_argument("Invalid message type value");    
    }
    return p->second;
}
/**
 *  stringToMessageType
 *     Converts a string value into a message type id.
 *
 *  @param typeString  - Stringified message type to convert.
 *  @return uint32_t   - corresponding message type.
 *  @throw std::invalid_argument - if the string is not a type string.
 */
uint32_t
CStatusDefinitions::stringToMessageType(const char* typeString)
{
    auto p = messageTypeLookup.find(std::string(typeString));
    
    // throw if lookup failed:
    
    if (p == messageTypeLookup.end()) {
        throw std::invalid_argument("Invalid message type string");
    }
    return p->second;
}
/**
 *  severityToString
 *     Convert a message severity value to a string.
 *
 *  @param severity - the severity value.
 *  @return std::string - The corresponding stringified value.
 *  @throw std::invalid_argument if severity is not a valid  severity value.
 */
std::string
CStatusDefinitions::severityToString(uint32_t severity)
{
    auto p = messageSeverityStringLookup.find(severity);
    
    // Throw if lookup failed:
    
    if (p == messageSeverityStringLookup.end()) {
        throw std::invalid_argument("Invalid message severity value");
    }
    
    return p->second;
}

/**
 * stringToSeverity
 *    Convert a stringified severity into is uint32_t value.
 *
 *  @parameter severityString - stringified severit.
 *  @return uint32_t          - Severity value.
 *  @throw std::invalid_argument - severityString has no corresponding severity
 *                                 value.
 */
uint32_t
CStatusDefinitions::stringToSeverity(const char* severityString)
{
    auto p = messageSeverityLookup.find(std::string(severityString));
    
    // Throw if lookup failed:
    
    if ( p == messageSeverityLookup.end()) {
        throw std::invalid_argument("Invalid severity string");
    }
    
    return p->second;
}
/**
 *  makeRingid
 *      Allocate and create a ring id message part struct:
 *
 * @param ringName - name of the ring
 * @return RingStatIdentification* Pointer to dynamically allocated/filled in
 *                   struct.
 * @note the s_tod field is filled in with the current unix time.
 * @note the caller is reponsible for invoking std::free to release the storage
 *       allocated by this method.
 */
CStatusDefinitions::RingStatIdentification*
CStatusDefinitions::makeRingid(const char* ringName)
{
    size_t totalSize = sizeof(RingStatIdentification) + strlen(ringName) + 1;
    
    RingStatIdentification* result =
        reinterpret_cast<RingStatIdentification*>(malloc(totalSize));

    result->s_tod = std::time(NULL);
    std::strcpy(result->s_ringName, ringName);
    
    return result;
}
/**
* ringIdSize
*    Given a ring id struct that is already filled in, determine how bit it is.
*
*  @param pRingId - the item to size.
*  @return size_t
*/
size_t
CStatusDefinitions::ringIdSize(RingStatIdentification* pRingId)
{
   return sizeof(RingStatIdentification) + std::strlen(pRingId->s_ringName) + 1;
}
/**
 * makeRingClient
 *    Allocate and create a ring client struct.
 *
 *  @param ops - number of operations.
 *  @param bytes - number of bytes transferred
 *  @param backlog - Number of bytes backlogged in the queue.
 *  @param pid     - Pid of the client.
 *  @param isProducer - true if this is the ring producer.
 *  @param command    - Client command string.
 *  @return RingStatClient* dynamically allocated/filled in struct.
 *  @note The client must release the storage for this struct via std::free
 */
CStatusDefinitions::RingStatClient*
CStatusDefinitions::makeRingClient(
    uint64_t ops, uint64_t bytes, uint64_t backlog, pid_t pid, bool isProducer,
    const std::vector<std::string>& command
)
{
    size_t totalSize = sizeof(RingStatClient) + sizeStringList(command);
    
    RingStatClient* pResult =
        reinterpret_cast<RingStatClient*>(std::malloc(totalSize));
    
    pResult->s_operations = ops;
    pResult->s_bytes      = bytes;
    pResult->s_backlog    = backlog;
    pResult->s_pid        = pid;
    pResult->s_isProducer   = isProducer ? true : false;
    copyStrings(pResult->s_command,  command);
    
    return pResult;
}
/**
 * ringClientSize
 *    Return the size of a ring client strucst that has been filled in
 *
 *  @param pClient - pointer to the filled  in struct.
 *  @return size_t
 */
size_t
CStatusDefinitions::ringClientSize(RingStatClient* pClient)
{
    return sizeof(RingStatClient) + sizeStringList(pClient->s_command);
}

/*-----------------------------------------------------------------------------
 *  Private methods.
/**
 * formatHeader
 *    Formats a message header.  The message header is the first message
 *    segment in a status message.  Therefore this method is used by all of the
 *    nested classes.
 *
 *   @param type     -- Type of message being created.
 *   @param severity -- Severity of the message.
 *   @param appName  -- Name of the application that's formatting this message.
 *   @return CStatusDefinitions::Header - the formatted header object.
 */
void
CStatusDefinitions::formatHeader(Header& hdr, uint32_t type, uint32_t severity, const char* appName)
{
    /* Build/send the header. */
    
    hdr.s_type = type;
    hdr.s_severity = severity;

      // Fill in the application name.
      
    std::strncpy(
        hdr.s_application, appName,
        sizeof(hdr.s_application) - 1
    );
    hdr.s_application[sizeof(hdr.s_application) -1 ] = '\0';
    
        // Fill in the source with the fqdn of this host:
    
    std::string host = Os::hostname();
    std::strncpy(hdr.s_source, host.c_str(), sizeof(hdr.s_source) -1);
    hdr.s_source[sizeof(hdr.s_source) -1]  = 0;
     
}