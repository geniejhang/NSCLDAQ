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
# @file   CStatusDb.h
# @brief  Class to encapsulate status database.
# @author <fox@nscl.msu.edu>
*/

#ifndef CSTATUSDB_H
#define CSTATUSDB_H
#include <zmq.hpp>
#include <vector>
#include "CStatusMessage.h"
#include <ctime>
#include <map>

#include <sys/types.h>
#include <unistd.h>

class CSqlite;
class CSqliteStatement;
class CQueryFilter;

/**
  * @class CSqlite
  *   Provides a high level interface to a status database for use by
  *   inserters and readers.   The database can be opened in readrwite
  *   or readonly mode.  If opened in readwrite mode the database
  *   schema is created if it does not exist yet.  This also allows the
  *   status database to live on top of an existing database.
  *
  *   The reasons we have a status database at all are:
  *   -   To maintain a persisten record of all status items that have been
  *       aggregated.
  *   -   To simplify filtering for status display applications.
  *   -   To provide the ability to generate reports/queries that span
  *       status message types (e.g. what log messages were emitted during
  *       run 23).
  *
  *   table cases simpler.
  */
class CStatusDb {
    // Public data structures:
public:
    // Query result for log records:
    
    typedef struct _LogRecord {
       unsigned    s_id;
       std::string s_severity;
       std::string s_application;
       std::string s_source;
       std::time_t s_timestamp;
       std::string s_message;
    } LogRecord, *pLogRecord;
    
    
    // Query result structs for ring buffers, their clients and statistics.
    
    // Needs copy construction/assignment.
    
    typedef struct _RingBuffer {
        unsigned    s_id;
        std::string s_fqname;
        std::string s_name;
        std::string s_host;
        _RingBuffer(const _RingBuffer& rhs) {
            copyIn(rhs);
        }
        _RingBuffer() {}
        _RingBuffer& operator=(_RingBuffer& rhs) {
            copyIn(rhs);
            return *this;
        }
        void copyIn(const _RingBuffer& rhs) {
            s_id = rhs.s_id;
            s_fqname = rhs.s_fqname;
            s_name = rhs.s_name;
            s_host = rhs.s_host;
        }
    } RingBuffer, *pRingBuffer;
    
    // Needs copy construction/assignment and comparison.
    
    typedef struct _RingClient {
        unsigned     s_id;
        pid_t        s_pid;
        bool         s_isProducer;
        std::string  s_command;
        _RingClient() {}
        _RingClient(const _RingClient& rhs) {
            copyIn(rhs);
        }
        _RingClient& operator=(const _RingClient& rhs) {
            copyIn(rhs);
            return *this;
        }
        void copyIn(const _RingClient& rhs) {
            s_id = rhs.s_id;
            s_pid = rhs.s_pid;
            s_isProducer = rhs.s_isProducer;
            s_command = rhs.s_command;
        }
        int operator==(const _RingClient& rhs) {
            return  (s_pid == rhs.s_pid)              &&
                    (s_isProducer == rhs.s_isProducer) &&
                    (s_command == rhs.s_command);
        }
        int operator!=(const struct _RingClient& rhs) {
            return !(operator==(rhs));
        }
    } RingClient, *pRingClient;
    
    typedef struct _RingStatistics {
        unsigned     s_id;
        time_t       s_timestamp;
        uint64_t     s_operations;
        uint64_t     s_bytes;
        uint64_t     s_backlog;
    } RingStatistics, *pRingSatistics;
    
    // Rings and client:
    
    typedef std::pair<RingBuffer, std::vector<RingClient> > RingAndClients;
    typedef std::map<std::string, RingAndClients> RingDirectory;
    
    // Rings, clients and statistics:
    
    typedef std::pair<RingClient, std::vector<RingStatistics> > RingClientAndStats;
    typedef std::pair<RingBuffer, std::vector<RingClientAndStats> > RingsAndStatistics;
    typedef std::map<std::string, RingsAndStatistics> CompleteRingStatistics;
    
    // Result struct for state transitions:
    
    typedef struct _StateApp {
        unsigned      s_id;
        std::string   s_appName;
        std::string   s_appHost;
        _StateApp() {}
        _StateApp(const _StateApp& rhs) {
            copyIn(rhs);
        }
        _StateApp& operator=(const _StateApp& rhs) {
            copyIn(rhs);
            return *this;
        }
        void copyIn(const _StateApp& rhs) {
            s_id = rhs.s_id;
            s_appName = rhs.s_appName;
            s_appHost = rhs.s_appHost;
        }
    } StateApp, *pStateApp;
    typedef struct _StateTransition {
        StateApp    s_app;
        unsigned    s_appId;
        unsigned    s_transitionId;                 
        time_t      s_timestamp;
        std::string s_leaving;
        std::string s_entering;
        _StateTransition() {}
        _StateTransition(const _StateTransition& rhs) {
            copyIn(rhs);
        }
        _StateTransition& operator=(const _StateTransition& rhs) {
            copyIn(rhs);
            return *this;
        }
        void copyIn(const _StateTransition& rhs) {
            s_appId        = rhs.s_appId;
            s_transitionId = rhs.s_transitionId;
            s_timestamp    = rhs.s_timestamp;
            s_leaving      = rhs.s_leaving;
            s_entering     = rhs.s_entering;
            s_app.copyIn(rhs.s_app);
        }
    } StateTransition, *pStateTransition;
    
    // Readout statistics structs:
    
    typedef StateApp ReadoutApp, *pReadoutApp;    // For now identical.
    typedef struct _RunInfo {
        unsigned   s_id;
        uint64_t   s_startTime;
        uint32_t  s_runNumber;
        std::string s_runTitle;
        _RunInfo() {}
        _RunInfo(const _RunInfo& rhs) {
            copyIn(rhs);
        }
        _RunInfo& operator=(const _RunInfo& rhs) {
            copyIn(rhs);
            return *this;
        }
        void copyIn(const _RunInfo& rhs) {
            s_id        = rhs.s_id;
            s_startTime = rhs.s_startTime;
            s_runNumber = rhs.s_runNumber;
            s_runTitle  = rhs.s_runTitle;
        }

    } RunInfo, *pRunInfo;
    
    typedef std::pair<ReadoutApp, std::vector<RunInfo> > ApplicationRun, *pApplicationRun;
    typedef std::map<unsigned, ApplicationRun> RunDictionary, *pRunDictionary;

        typedef struct _ReadoutStatistics {
        unsigned      s_id;
        time_t        s_timestamp;
        unsigned      s_elapsedTime;
        uint64_t      s_triggers;
        uint64_t      s_events;
        uint64_t      s_bytes;
        
    } ReadoutStatistics, *pReadoutStatistics;
    typedef std::pair<RunInfo, std::vector<ReadoutStatistics> > RunStatistics;
    typedef std::pair<ReadoutApp, std::vector<RunStatistics> >  ReadoutAppStats;
    typedef std::map<unsigned, ReadoutAppStats> ReadoutStatDict, *pReadoutStatDict;
    
private:
    CSqlite&        m_handle;             // Database handle.
    
    // Stored creation queries.
    
    CSqliteStatement* m_pLogInsert;      // Insert log message.

    CSqliteStatement* m_addRingBuffer;   // Insert into ring_buffer.
    CSqliteStatement* m_addRingClient;   // Insert into ring client.
    CSqliteStatement* m_addRingStats;    // Add statistics for a ring/client.
        
    CSqliteStatement* m_getRingId;    // Check existence of a ring buffer.
    CSqliteStatement* m_getClientId;  // Check for sqlite client.
    
    CSqliteStatement* m_getSCAppId;
    CSqliteStatement* m_addSCApp;
    CSqliteStatement* m_addSC;

    CSqliteStatement* m_getReadoutId;
    CSqliteStatement* m_addReadout;
    CSqliteStatement* m_getRunId;
    CSqliteStatement* m_addRun;
    CSqliteStatement* m_addRunStats;
    
public:
    CStatusDb(const char* dbSpec, int flags);
    virtual ~CStatusDb();

    // Insertion operations:
    
public:    
    void insert(std::vector<zmq::message_t*>& message);
    void addRingStatistics(
        uint32_t severity, const char* app, const char* src,
        const CStatusDefinitions::RingStatIdentification& ringId,
        const std::vector<const CStatusDefinitions::RingStatClient*>& clients
    );
    void addStateChange(
        uint32_t severity, const char* app, const char* src,
        int64_t  tod, const char* from, const char* to
    );
    void addReadoutStatistics(
        uint32_t severity, const char* app, const char* src,
        int64_t startTime, uint32_t runNumber, const char* title,
        const CStatusDefinitions::ReadoutStatCounters* pCounters = NULL
    );
    void addLogMessage(
        uint32_t severity, const char* app, const char* src,
        int64_t  time, const char* message
    );
    
    // Queries:
public:
    void queryLogMessages(std::vector<LogRecord>& result, CQueryFilter& filter);
    
    void listRings(std::vector<RingBuffer>& result, CQueryFilter& filter);
    void listRingsAndClients(RingDirectory& result, CQueryFilter& filter);
    void queryRingStatistics(CompleteRingStatistics& result, CQueryFilter& filter);

    void listStateApplications(
        std::vector<StateApp>& result, CQueryFilter& filter
    );
    void queryStateTransitions(
        std::vector<StateTransition>& result, CQueryFilter& filter
    );
    
    void listReadoutApps(std::vector<ReadoutApp>& result, CQueryFilter& filter);
    void listRuns(RunDictionary& result, CQueryFilter& filter);
    void queryReadoutStatistics(ReadoutStatDict& result, CQueryFilter& filter);
    
            // Transitional methods between insert and addXXXX
private:
    void marshallRingStatistics(
        const CStatusDefinitions::Header*   header,
        const std::vector<zmq::message_t*>& message
    );
    void marshallStateChange(
        const CStatusDefinitions::Header*   header,
        const std::vector<zmq::message_t*>& message
    );
    void marshallReadoutStatistics(
        const CStatusDefinitions::Header*   header,
        const std::vector<zmq::message_t*>& message    
    );
    void marshallLogMessage(
        const CStatusDefinitions::Header*   header,
        const std::vector<zmq::message_t*>& message 
    );
private:
    void createSchema();
    
    int getRingId(const char* name, const char* host);
    int addRingBuffer(const char* name, const char* host);
    int getRingClientId(
        int ringId, const CStatusDefinitions::RingStatClient& client
    );
    int addRingClient(
        int ringId, const CStatusDefinitions::RingStatClient& client
    );
    int addRingClientStatistics(
        int ringId, int clientId, uint64_t timestamp,
        const CStatusDefinitions::RingStatClient& client
    );
    
    int getStateChangeAppId(const char* appName, const char* host);
    int addStateChangeApp(const char* appName, const char* host);
    int addStateChange(int appId, int64_t timestamp, const char* from, const char* to);
    
    int getReadoutProgramId(const char* app, const char* src);
    int addReadoutProgram(const char* app, const char* src);
    int getRunInfoId(int rdoId, int runNumber, const char* title, int64_t startTime);
    int addRunInfo(int rdoId, int runNumber, const char* title, int64_t startTime);
    int addRdoStats(
        int readoutId, int runId, int64_t timestamp, int64_t elapsedTime,
        int64_t triggers, int64_t events, int64_t bytes
    );
    
    std::string marshallWords(const char* words);
};  

#endif