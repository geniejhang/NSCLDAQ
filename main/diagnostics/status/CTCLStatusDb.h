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
# @file   CTCLStatusDb.h
# @brief  TCL Bindings for the CStatusDb class.
# @author <fox@nscl.msu.edu>
*/

#ifndef CTCLSTATUSDB_H
#define CTCLSTATUSDB_H

#include <TCLObjectProcessor.h>
#include <map>
#include <string>
#include <vector>
#include <zmq.hpp>
#include "CStatusMessage.h"
#include "CStatusDb.h"

class CTCLInterpreter;
class CTCLObject;
class CQueryFilter;

/**
* @class CTCLStatusDb
*    Provides a class that generates and destroycs CTCLStatusDbInstance objects
*    those objects are TCL wrappers of instances of CStatusDb objects.
*    Note that the CTCLStatusDb class is private nested and therefore not accessible
*    to the public.
*/
class CTCLStatusDb : public CTCLObjectProcessor
{
    // Internal data types:
private:
    class CTCLStatusDbInstance;                  // Forward definition.
    
    typedef std::map<std::string, CTCLStatusDbInstance*> InstanceMap;

    // Object attributes:
private:
    InstanceMap    m_Instances;
    unsigned       m_instanceNumber;
    
    // Canonicals:

public:
    CTCLStatusDb(CTCLInterpreter& interp, const char* name);
    virtual ~CTCLStatusDb();
    
    // object processor interface:

public:
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

    // Utility functions:
private:
    void create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void kill(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    std::string assignName();
    int sqliteFlag(std::string flagString);
    
    // Nested class:

private:
    class CTCLStatusDbInstance : public CTCLObjectProcessor
    {
        private:
            CStatusDb*  m_pDb;
        public:
            CTCLStatusDbInstance(
                CTCLInterpreter& interp, const char* name,
                CStatusDb* pDb
            );
            virtual ~CTCLStatusDbInstance();
        
        public:
            int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& obv);
            
            // Insert top level command handlers:
        private:
            void insert(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
            void addRingStatistics(
                CTCLInterpreter& interp, std::vector<CTCLObject>& objv
            );
            void addStateChange(
                CTCLInterpreter& interp, std::vector<CTCLObject>& objv
            );
            void addReadoutStatistics(
                CTCLInterpreter& interp, std::vector<CTCLObject>& objv
            );
            void addLogMessage(
                CTCLInterpreter& interp, std::vector<CTCLObject>& objv
            );
            void savepoint(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
            
            // Query top level command handlers:
            
        private:
            void queryLogMessages(
                CTCLInterpreter& interp, std::vector<CTCLObject>& objv
            );
            
            void listRings(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
            void listRingsAndClients(
                CTCLInterpreter& interp, std::vector<CTCLObject>& objv
            );
            void queryRingStatistics(
                CTCLInterpreter& interp, std::vector<CTCLObject>& objv
            );
            
            void listStateApplications(
                CTCLInterpreter& interp, std::vector<CTCLObject>& objv
            );
            void queryStateTransitions(
                CTCLInterpreter& interp, std::vector<CTCLObject>& objv
            );
            
            void listReadoutApps(
                CTCLInterpreter& interp, std::vector<CTCLObject>& objv
            );
            void listRuns(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
            void queryReadoutStatistics(
                CTCLInterpreter& interp, std::vector<CTCLObject>& objv
            );
        // Utilities:
        private:
            std::vector<zmq::message_t*> marshallMessage(
                CTCLInterpreter& interp, std::vector<CTCLObject>& objv, unsigned start
            );
            void freeMessageVector(std::vector<zmq::message_t*>& message);
            CStatusDefinitions::RingStatIdentification* decodeRingIdDict(
                CTCLInterpreter& interp, CTCLObject& dict
            );
            CStatusDefinitions::RingStatClient* decodeRingClientDict(
                CTCLInterpreter& interp, CTCLObject& obj
            );
            void freeRingClients(
                std::vector<const CStatusDefinitions::RingStatClient*>& clients
            );
            void decodeReadoutCounterStats(
                CStatusDefinitions::ReadoutStatCounters& counters,
                CTCLInterpreter& interp, CTCLObject& obj
            );
            void createLogRecordDict(CTCLObject& result, CStatusDb::LogRecord& rec);
            void createRingInfoDict(CTCLObject& result, CStatusDb::RingBuffer& rec);
            void createRingClientDict(CTCLObject& result, CStatusDb::RingClient& rec);
            void createRingStatisticsDict(
                CTCLObject& result, CStatusDb::RingStatistics& rec
            );
            void createAppDictionary(CTCLObject& result, CStatusDb::StateApp& rec);
            void createTransitionDict(CTCLObject& result, CStatusDb::StateTransition& rec);
            void createRunDictionary(CTCLObject& result, CStatusDb::RunInfo& rec);
            void createRunStatsDict(CTCLObject& result, CStatusDb::ReadoutStatistics& rec);
            CQueryFilter* createRawFilter(CTCLInterpreter& interp, CTCLObject& tclFilter);
            
    };
};

#endif
