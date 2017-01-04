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
# @file   CTCLStatusDb.cpp
# @brief  Implements the CTCLStatusDb class and its nested CTCLStatusDbInstance
# @author <fox@nscl.msu.edu>
*/

#include "CTCLStatusDb.h"
#include <TCLInterpreter.h>
#include <TCLInterpreterObject.h>
#include <Exception.h>
#include <TCLException.h>

#include "CStatusDb.h"
#include "CStatusMessage.h"
#include "TclUtilities.h"


#include <stdexcept>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <CSqliteWhere.h>
#include <CSqlite.h>
// Map CSqlite open flag strings to their integer equivalents:

static std::map<std::string, int> OpenFlagMap = {
    {"nomutex", CSqlite::nomutex},         {"fullmutex", CSqlite::fullmutex},
    {"sharedcache", CSqlite::sharedcache}, {"privatecache", CSqlite::privatecache},
    {"uri", CSqlite::uri},                 {"readonly", CSqlite::readonly},
    {"readwrite", CSqlite::readwrite},     {"create", CSqlite::create}
};

/*---------------------------------------------------------------------------\
 *  Implementation of CTCLStatusDb
 */

/**
 * constructor (CTCLStatusDb)
 *   Registers the command with the interpreter.  Initializes the per command
 *   instance number.  Note that a per command instance number implies that
 *   only one of these can be registered on any single application interpreter
 *   to avoid command name collisions.
 *
 * @param interp - The interpreter no which the command is being registered.
 * @param name   - Name of the command being registered.
 */
CTCLStatusDb::CTCLStatusDb(CTCLInterpreter& interp, const char* name) :
    CTCLObjectProcessor(interp, name)
{}

/**
 * Destructor
 *    We need to kill off all our instance commands to prevent them from
 *    'leaking'.  This is normally only necessary if one is dynamically
 *    creating and destroying interpreters that will register this command.
 *    destruction of the interpreter results in the destruction of our object.
 */
CTCLStatusDb::~CTCLStatusDb()
{
    for (auto p = m_Instances.begin(); p != m_Instances.end(); p++) {
        delete p->second;
    }
    // The assumption is that the map can destroy its own elements...
}

/**
 * operator()
 *    Gets control if our command is invoked.  The command must have at
 *    least a subcommand ('create' or 'destroy').   Control is
 *    dispatched to the appropriate subcommand handler.  Note that
 *    as usual, error management is done by wrapping all the good stuff inside
 *    a try/catch block that attempts to map exceptions we know might be thrown
 *    to TCL_ERROR returns with error messages in the result.
*/
int
CTCLStatusDb::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    try {
        requireAtLeast(objv, 2);
        std::string subCommand = objv[1];
        
        if (subCommand == "create") {
            create(interp, objv);
        } else if (subCommand == "destroy") {
            kill(interp, objv);
        } else {
            throw std::invalid_argument(
                "Invalid subcommand, must be create or destroy"
            );
        }
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (const char* msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (CException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult("Unanticipated exception caught in CTCLStatusDb");
        return TCL_ERROR;
    }
    
    return TCL_OK;
}

/**
 * create
 *    Creates a new database object, and binds it to a new command object.
 *    -  The CStatusDb object is created.
 *    -  A new command is assigned.
 *    -  The CTCLStatusDbInstance object is created in this interpreter
 *    -  The resulting object is saved in the m_Instances map indexed by
 *       the assigned command name.
 *
 *  @param interp   - interpreter that's running the command.
 *  @param objv     - the vector of command words. At least one parameter must
 *                    follow the subcommand - the database filename.  Any
 *                    additional parameters must translate to a valid
 *                    SQLITE flag value and provides the flag mask for the
 *                    construction of the CStatusDb object.
 */
void
CTCLStatusDb::create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireAtLeast(objv, 3);            // Need at least a filename.
    std::string filename = objv[2];
    int         flags    = 0;
    
    // Build up the flags from the remaining command words:
    
    for (int i = 3; i < objv.size(); i++) {
        flags |= sqliteFlag(std::string(objv[i]));
    }
    CStatusDb* pDb = new CStatusDb(filename.c_str(), flags);
    std::string      name = assignName();
    CTCLStatusDbInstance* pInstance =
        new CTCLStatusDbInstance(interp, name.c_str(), pDb);
    m_Instances[name] = pInstance;
    
    interp.setResult(name);                 // Make the name avail to caller.
}
/**
 * kill
 *   Destroys and existing status db instance command:
 *   -  Ensure the command exists as an instance.
 *   -  Destroy the command
 *   -  Remove it from the dict.
 *
 * @param interp - Interpreter that's running the command.
 * @param objv   - Command words.  In addition to the base and subcommands,
 *                 the invocation must have the name of a valid instance command.
 */
void
CTCLStatusDb::kill(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 3);
    std::string name = objv[2];
    
    auto p = m_Instances.find(name);
    if (p != m_Instances.end()) {
        delete p->second;
        m_Instances.erase(p);
    } else {
        throw std::invalid_argument("No such instance command");
    }
}
/**
 * assignName
 *    Create and return a name for an instance command.  These names
 *    are of the form statusdb_nnn where nnn is a unique number.
 *
 * @return std::string
 */
std::string
CTCLStatusDb::assignName()
{
    m_instanceNumber++;                           // Next instance.
    std::ostringstream nameStream;
    nameStream << "statusdb_" << m_instanceNumber;
    
    return nameStream.str();
}
/**
 * sqliteFlag
 *    Method that translates a string into an SQLITE integer flag.
 *
 *  @param flagString - the stringified flag.
 *  @return int       - The integer flag.
 *  @throws std::invalid_argument - if flagString is not a valid flag string.
 *
 */
int
CTCLStatusDb::sqliteFlag(std::string flagString)
{
    auto p = OpenFlagMap.find(flagString);
    if (p != OpenFlagMap.end()) {
        return p->second;
    } else {
        std::string e = "Invalid sqlite open flag name: ";
        e += flagString;
        throw std::invalid_argument(e);
    }
}
/*-----------------------------------------------------------------------------
 *  Implementation of CTCLStatusDb::CTCLStatusDbInstance nested class.
 */

/**
 * Constructor  (CTCLStatusDb::CTCLStatusDbInstance)
 *
 * @param interp - interpreter the command is being registered on.
 * @param name   - Name of the newly created command.
 * @param pDb    - Pointer to the wrapped status database API object.
 */
CTCLStatusDb::CTCLStatusDbInstance::CTCLStatusDbInstance(
    CTCLInterpreter& interp, const char* pName, CStatusDb* pDb
) :
    CTCLObjectProcessor(interp, pName),
    m_pDb(pDb)
{}

/**
 * destructor:
 */
CTCLStatusDb::CTCLStatusDbInstance::~CTCLStatusDbInstance() {}

/**
 * operator()
 *   Top level dispatcher for the subcommands of this object.
 *
 *   @param interp - interpreter that's running the command.
 *   @param objv   - Vector of command words.
 *   @return int   - TCL_OK Or TCL_ERROR.
 */
int
CTCLStatusDb::CTCLStatusDbInstance::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    bindAll(interp, objv);
    try {
        requireAtLeast(objv, 2);
        std::string subcommand = objv[1];
        
        if (subcommand == "insert") {
            insert(interp, objv);
        } else if (subcommand == "addRingStatistics") {
            addRingStatistics(interp, objv);
        } else if (subcommand == "addStateChange") {
            addStateChange(interp, objv);
        } else if (subcommand == "addReadoutStatistics") {
            addReadoutStatistics(interp, objv);
        } else if (subcommand == "addLogMessage") {
            addLogMessage(interp, objv);
        } else if (subcommand == "queryLogMessages") {
            queryLogMessages(interp, objv);
        } else if (subcommand == "listRings") {
            listRings(interp, objv);
        } else if (subcommand == "listRingsAndClients") {
            listRingsAndClients(interp, objv);
        } else if (subcommand == "queryRingStatistics") {
            queryRingStatistics(interp, objv);
        } else if (subcommand == "listStateApplications") {
            listStateApplications(interp, objv);
        } else if (subcommand == "queryStateTransitions") {
            queryStateTransitions(interp, objv);
        } else if (subcommand == "listReadoutApps") {
            listReadoutApps(interp, objv);
        } else if (subcommand == "listRuns") {
            listRuns(interp, objv);
        } else if (subcommand == "queryReadoutStatistics") {
            queryReadoutStatistics(interp, objv);
        } else {
            throw std::invalid_argument("Status Database Instance - invalid subcommand");
        }
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (CException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (const char* msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult("DB Status instance unexpected exception caught");
        return TCL_ERROR;
    }
    return TCL_OK;
}
/*----------------------------------------------------------------------------
 *  Record creating methods:
 */
/**
 * insert
 *    Insert an arbitrary entry into the database.  The command words that
 *    follow the subcommand are binary data that make up the message parts.
 *    These will be marshalled back into an std::vector<zmq::message_t*> objects
 *    before being passed to the wrapped CStatusDb::insert method.
 *
 *  @param interp   - interpreter that's executing this command.
 *  @param objv     - the command words, [0] is the base command [1] is the
 *                    subcommand, [2] on are the message parts as Tcl byte array
 *                    objects.
 */
void
CTCLStatusDb::CTCLStatusDbInstance::insert(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    std::vector<zmq::message_t*> message = marshallMessage(interp, objv, 2);
    try {
        m_pDb->insert(message);
    }
    catch (...) {
        freeMessageVector(message);             // Ensure release of storage on error
        throw;
    }
    
    freeMessageVector(message);               // ... and on success.
}
/**
 * addRingStatistics
 *    Add a ring statistics item to the database.
 *
 *  @param interp  - Reference to the interpreter that executes the command.
 *  @param objv    - The command words.  In addition to the command and subcommand,
 *                   We must have:
 *                    -  Severity of the message (normally INFO)
 *                    -  Application name.
 *                    - Source host FQDN
 *                    - ringidDict - dict that is a ring identification (e.g. from
 *                      decode -- see CTCLDecodeMessage::decodeRingIdent).
 *                    - Remaining command words are decoded statistics dicts.
 */
void
CTCLStatusDb::CTCLStatusDbInstance::addRingStatistics(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireAtLeast(objv, 6);   // Stats are not required (no clients e.g.).
    
    uint32_t severity
        = CStatusDefinitions::stringToSeverity(std::string(objv[2]).c_str());
    std::string appName = objv[3];
    std::string fqdn    = objv[4];
    CStatusDefinitions::RingStatIdentification* id =
        decodeRingIdDict(interp, objv[5]);
    std::vector<const CStatusDefinitions::RingStatClient*> clients;
    try {
        for (int i = 6; i < objv.size(); i++) {
            clients.push_back(decodeRingClientDict(interp, objv[i]));
        }
        m_pDb->addRingStatistics(
            severity, appName.c_str(), fqdn.c_str(), *id, clients
        );
    }
    catch (...) {
        std::free(id);
        freeRingClients(clients);
        throw;
    }
    std::free(id);
    freeRingClients(clients);  
}
/**
 * addStateChange
 *     Add a state change to the database.
 *
 *  @param interp - interpreter executing the command.
 *  @param objv   - Command words.  In addition to the command adn subcommand
 *                  we need exactly:
 *                  *   Severity String.
 *                  *   application name.
 *                  *   message source (FQDN)
 *                  *   timestamp - 64 bit timestamp for the actual change.
 *                  *   from - State being exited.
 *                  *   to   - State being left.
 */
void
CTCLStatusDb::CTCLStatusDbInstance::addStateChange(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 8);
    
    uint32_t sev = TclMessageUtilities::stringToSeverity(std::string(objv[2]).c_str());
    std::string app = objv[3];
    std::string source = objv[4];
    uint64_t    tod    = double(objv[5]);
    std::string from   = objv[6];
    std::string to     = objv[7];
    
    m_pDb->addStateChange(
        sev, app.c_str(), source.c_str(), tod, from.c_str(), to.c_str()
    );
}
/**
 * addReadoutStatistics
 *    Add statistics for a readout program or just add the program.
 *
 * @param interp - intepreter executing the command.
 * @param objv   - Pointer to the command words.  In addition to the
 *                 command and keyword, we should have the following:
 *                 *  severity  string.
 *                 *  app application name.
 *                 *  source of message (FQDN).
 *                 *  Run start time.
 *                 *  run number.
 *                 *  title string.
 *                 *  dict contanining the trigger counters. (optional)
 */
void
CTCLStatusDb::CTCLStatusDbInstance::addReadoutStatistics(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireAtLeast(objv, 8);
    requireAtMost(objv, 9);
    uint32_t sev = TclMessageUtilities::stringToSeverity(std::string(objv[2]).c_str());
    std::string app = objv[3];
    std::string src = objv[4];
    uint64_t    start = double(objv[5]);
    uint64_t    run   = double(objv[6]);
    std::string title = objv[7];
    CStatusDefinitions::ReadoutStatCounters* pCounters(0);
    CStatusDefinitions::ReadoutStatCounters stats;
    
    if (objv.size() == 9) {
        decodeReadoutCounterStats(stats, interp, objv[8]);
        pCounters = &stats;
    }
    m_pDb->addReadoutStatistics(
        sev, app.c_str(), src.c_str(), start, run, title.c_str(),
        pCounters
    );
    
}

/**
 * addLogMessage
 *    Adds a log message to the database.
 *
 *  @param interp    - interpreter that's running the command.
 *  @param objv      - The command words.  In addition to the command/subcommand
 *                     we must have:
 *                     *   Severity string
 *                     *   application name.
 *                     *   source of message (FQDN)
 *                     *   timestamp
 *                     *   message string.
 */
void
CTCLStatusDb::CTCLStatusDbInstance::addLogMessage(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 7);
    
    uint32_t sev = TclMessageUtilities::stringToSeverity(std::string(objv[2]).c_str());
    std::string app = objv[3];
    std::string src = objv[4];
    uint64_t    tod = double(objv[5]);
    std::string msg = objv[6];
    
    m_pDb->addLogMessage(sev, app.c_str(), src.c_str(), tod, msg.c_str());
}

/*----------------------------------------------------------------------------
 * Record retrieval methods.
 */

/**
 *  queryLogMessages
 *     Retrieve a set of log message records.  The result of this
 *     query is a possibly empty list of dicts.  Each dict has the keys:
 *     -  id          - The primary key of a log entry.
 *     -  severity    - A severity string
 *     -  application - The application name.
 *     -  source      - The source host (fqdn)
 *     -  timestamp   - The time at which the log message was initially created.
 *     -  message     - The text of the message.
 *
 *  @param interp  - interpreter running the command.
 *  @param objv    - Command words.  In addition to the command/subcommand
 *                   an optional Tcl Where item can be supplied.  If omitted the
 *                   filter supplied to the uderlying query will be
 *                   DAQ::acceptAll.   If supplied the toString method of the
 *                   filter will be invoked and used to create a CRawFilter
 *                   filter.
 */
void
CTCLStatusDb::CTCLStatusDbInstance::queryLogMessages(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireAtMost(objv, 3);
    CQueryFilter* pFilter = &(::DAQ::acceptAll);        // Default.
    CQueryFilter*   pRawFilter(0);
    try {
        if (objv.size() == 3) {
            pRawFilter = createRawFilter(interp, objv[2]);
            pFilter = pRawFilter; 
        }
        std::vector<CStatusDb::LogRecord> rawResult;
        m_pDb->queryLogMessages(rawResult, *pFilter);
        
        // Create the interpreter result:
        
        CTCLObject interpResult;
        interpResult.Bind(interp);
        for (int i = 0; i < rawResult.size(); i++) {
            CTCLObject item;
            item.Bind(interp);
            createLogRecordDict(item, rawResult[i]);
            interpResult += item;
        }
        interp.setResult(interpResult);
    }
    catch(...) {
        delete pRawFilter;               // delete 0 is no-op.
        throw;
    }
    delete pRawFilter;                   // Note delete 0 is no-op.
}
/**
 * listRings
 *   Returns a list of dicts that describe the rings that are known to the
 *   database.  Each dict has the following key/values:
 *
 *   -  id   - The primary key of the entry.
 *   -  name - The name of the ring buffer.
 *   -  host - The host the ring buffer lives in (FQDN)
 *   -  fqname - The fully qualified ring name (name@host).  Note that
 *             this is a synthetic field that is generated by the underlying
 *             library rather than being stored in the database itself.
 *
 *  @note fqname might be of the form name@host1@host2 if the ring is a proxy
 *        ring for a ring that lives in host1.  This indicates that the
 *        ringbuffer lives in host2 but is a proxy for a ring of the same
 *        name in host1.  For example, suppose I create a ring1 in the host
 *        spdaq20.nscl.msu.edu;  This ring has an fqname of
 *        ring1@spdaq20.nscl.msu.edu.  Suppose further that I now run e.g. dumper
 *        in u6pc2.nscl.msu.edu specifying --source=tcp://spdaq20/ring1.  The
 *        fqname of the resulting ring is:
 *        ring1@spdaq20.nscl.msu.edu@u6pc2.nscl.msu.edu
 *
 *  @param interp - The interpreter executing this command.
 *  @param objv   -  The objects that make up the command words.  In addition
 *                   to the command, subcommand, there can be an optional
 *                   Tcl command that has a subcommand 'toString'.  This is normally
 *                   a TclWhere object.  The result from executing toString
 *                   on that command is used to construct a CRawFilter object
 *                   used to filter the set of rings returned.
 */
void
CTCLStatusDb::CTCLStatusDbInstance::listRings(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireAtMost(objv, 3);
    
    // Figure out the filter - defaults to acceptAll.
    
    CQueryFilter* pFilter = &DAQ::acceptAll;        // Default value.
    CQueryFilter* pSuppliedFilter(0);               // holds the override.
    
    if (objv.size() == 3) {
        pSuppliedFilter = createRawFilter(interp, objv[2]);
        pFilter         = pSuppliedFilter;
    }
    try {
        std::vector<CStatusDb::RingBuffer> rawResult;
        CTCLObject                         result;
        result.Bind(interp);
        
        m_pDb->listRings(rawResult, *pFilter);
        
        for (int i =0; i < rawResult.size(); i++) {
            CTCLObject item;
            item.Bind(interp);
            createRingInfoDict(item, rawResult[i]);
            result += item;
        }
        interp.setResult(result);
        
    }
    catch (...) {
        delete pSuppliedFilter;                  // Clean up dyn. storage.
        throw;
    }
    delete pSuppliedFilter;
}
/**
 * listRingsAndClients
 *    Returns information about all of the ringbuffers and their clients.  The
 *    return value is a dict whose keys are the fully qualified names of each ring.
 *    The value of each key is a two element list consisting of
 *    - Full information about the ring (see listRings for the format of this item),
 *    - A list of dicts.  Each dict containing information about a single client
 *      of that ring.
 *
 *     The client information dict has the following keys:
 *
 *     -   id   - Primary key of the item in the ring client table.
 *     -   pid  - Process id of the client (the process must live in the same
 *                host as the ring -- only local clients, including proxy hoisters
 *                and proxy receivers are listed).
 *     -  isProducer - Bool that is true if the client produces for the ring.
 *     -  command - The command string used to start the client.
 *
 * @param interp - interpreter that is running the command (reference).
 * @param objv   - The objects that make up the command.  In addition to the
 *                 base command, this include an optional Tcl command that has
 *                 a 'toString' subcommand.  This subcommand is expected to return
 *                 an SQL Where clause body which will be used to filter the
 *                 results of the underlying query. Typcially this command is
 *                 a TclFilter object.
 */
void
CTCLStatusDb::CTCLStatusDbInstance::CTCLStatusDbInstance::listRingsAndClients(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireAtMost(objv, 3);
    
    // Figure out the query filter - defaults to DAQ::accept all:
    
    CQueryFilter* pFilter = &::DAQ::acceptAll;
    CQueryFilter* pSupplied(0);               // The final supplied filter.
    
    if (objv.size() == 3) {
        pSupplied = createRawFilter(interp, objv[2]);
        pFilter = pSupplied;
    }
    // This try block ensures that pSupplied gets deleted:
    
    try {
        // Execute the query:
        
        CStatusDb::RingDirectory rawResult;
        m_pDb->listRingsAndClients(rawResult, *pFilter);
        
        // Iterate over the map and the contents to create the Tcl result:
        
        CTCLObject result;
        result.Bind(interp);
        for(auto pDirEntry = rawResult.begin(); pDirEntry != rawResult.end(); pDirEntry++)
        {
            // Break apart the elements of each map entry:
            
            std::string                         key = pDirEntry->first;
            CStatusDb::RingAndClients&          info(pDirEntry->second);
            CStatusDb::RingBuffer&              ring(info.first);
            std::vector<CStatusDb::RingClient>& clients(info.second);
            
            // Build up the dict entry:
            
            CTCLObject dictEntry;
            dictEntry.Bind(interp);
            
            // Ring info:
            
            CTCLObject ringInfoDict;
            ringInfoDict.Bind(interp);
            createRingInfoDict(ringInfoDict, ring);
            dictEntry += ringInfoDict;
            
            //Client Info - list of dicts:
            
            CTCLObject clientList;
            clientList.Bind(interp);
            for (int i =0; i < clients.size(); i++) {
                CTCLObject clientInfo;
                clientInfo.Bind(interp);
                
                createRingClientDict(clientInfo, clients[i]);
                clientList += clientInfo;
            }
            dictEntry += clientList;
            
            TclMessageUtilities::addToDictionary(
                interp,  result, key.c_str(), dictEntry
            );
        }
        
        interp.setResult(result);
    }
    catch (...) {
        delete pSupplied;
        throw;
    }
    
    delete pSupplied;
}
/**
 * queryRingStatistics
 *     Returns information about the rings, clients and their statistics.
 *     The result is a dict with keys that are fully qualified ring names.
 *     The contents of each key are a two element list consisting of
 *     the ring information (see listRings for information about that dict),
 *     and a list of two element lists.  The first element of each of _those_
 *     lists is the dict described in listRingsAndClients that descsribes the
 *     ring client.  The second element is, itself a list of statistics dicts.
 *     Each statistics dict has the following keys:
 *
 *     -  id  - primary key of the record in its table.
 *     -  timestamp - the [clock seconds] at which the statistics item was emitted.
 *     -  operations - the number of ring operations performed by the client
 *     -  bytes      - the number of bytes of data the client has consumed/produced.
 *     -  backlog    - only meaningful if this is a consumer - number of bytes
 *                     backlogged for this client in the ringbuffer.
 *
 * @param interp - interpreter executing the command.
 * @param objv   - The objects that make up the command.  In addition to the
 *                 base command, this include an optional Tcl command that has
 *                 a 'toString' subcommand.  This subcommand is expected to return
 *                 an SQL Where clause body which will be used to filter the
 *                 results of the underlying query. Typcially this command is
 *                 a TclFilter object.
 */
void
CTCLStatusDb::CTCLStatusDbInstance::queryRingStatistics(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireAtMost(objv, 3);
    
    // Sort out the filter:
    
    CQueryFilter* pFilter(&::DAQ::acceptAll);    // Default filter.
    CQueryFilter* pRaw(0);
    
    if (objv.size() == 3) {
        pRaw = createRawFilter(interp, objv[2]);
        pFilter = pRaw;
    }
    // The try block below ensures that the pRaw filter, if used will get destroyed
    
    try {
        CStatusDb::CompleteRingStatistics rawResult;
        m_pDb->queryRingStatistics(rawResult, *pFilter);
        
        CTCLObject result;                          // Tcl-ized result.
        result.Bind(interp);
        
        // Iterate over the raw result map:
        
        for(auto pKeyVal = rawResult.begin(); pKeyVal != rawResult.end(); pKeyVal++) {
            // break part the bits of pKeyVal and its subcomponents
            
            std::string key                           = pKeyVal->first;
            CStatusDb::RingsAndStatistics&              value(pKeyVal->second);
            CStatusDb::RingBuffer&                      ringInfo(value.first);
            std::vector<CStatusDb::RingClientAndStats>& clientsAndStats(value.second);
            
            // Build up the item we'll insert for this dict:
            
            CTCLObject dictValue;
            dictValue.Bind(interp);
            
            CTCLObject ringInfoDict;
            ringInfoDict.Bind(interp);
            createRingInfoDict(ringInfoDict, ringInfo);
            dictValue+= ringInfoDict;
            
            // Now turn the vector of RingClientAndStats into a list:
            
            CTCLObject clientAndStatsList;
            clientAndStatsList.Bind(interp);
            
            // This iteration is over the cients and their stats vectors.
            
            for(int c = 0; c < clientsAndStats.size(); c++) {
                CTCLObject perClientInfo;
                perClientInfo.Bind(interp);
                
                CTCLObject ringClient;
                ringClient.Bind(interp);
                createRingClientDict(ringClient, clientsAndStats[c].first);
                perClientInfo += ringClient;
                
                CTCLObject statsList;
                statsList.Bind(interp);
                
                // Now loop over all statistics entries for the client:
                
                std::vector<CStatusDb::RingStatistics>& stats(clientsAndStats[c].second);
                for(int s = 0; s < stats.size(); s++) {
                    CTCLObject statsDict;
                    statsDict.Bind(interp);
                    createRingStatisticsDict(statsDict, stats[s]);
                    
                    statsList += statsDict;
                }
                
                perClientInfo += statsList;
                
                clientAndStatsList += perClientInfo;
            }
            
            
            dictValue+= clientAndStatsList;
            TclMessageUtilities::addToDictionary(
                interp, result, key.c_str(), dictValue
            );
        }
        
        interp.setResult(result);
    }
    catch(...) {
        delete pRaw;
        throw;
    }
    
    delete pRaw;
}
/**
 * listStateApplications
 *     Lists the applications that contribute to state transition records.
 *     The result from this is a list of dicts.  Each dict describes one
 *     application and contains the following keys:
 *
 *      - id   - Primary key of the app in its database table.
 *      - name - application name chosen by the application.
 *      - host - Host the application runs on.
 *
 * @param interp   - interpreter running the command.
 * @param objv     - Words that make up the command line.  In addition to the
 *                   command and the subcommand this can include an optional filter
 *                   command that must implement a toString subcommand.  The
 *                   subcommand must return a WHERE body which is then used to
 *                   construct a CRawFilter that is passed to the API to
 *                   restrict the set of records returned.  If this command
 *                   is not supplied, no filtering is done.
 */
void
CTCLStatusDb::CTCLStatusDbInstance::listStateApplications(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireAtMost(objv, 3);
    
    // Figure out what filter we'll pass to the query:
    
    CQueryFilter* pFilter(&::DAQ::acceptAll);      // Default is 'true'.
    CQueryFilter* pSupplied(0);
    
    if (objv.size() == 3) {
        pSupplied = createRawFilter(interp, objv[2]);
        pFilter = pSupplied;
    }
    // The try block below ensures that pSupplied is deleted no matter what:
    
    try {
        // Do the query to get the raw result:
        
        std::vector<CStatusDb::StateApp> rawResult;
        m_pDb->listStateApplications(rawResult, *pFilter);
        
        // Marshall the raw result into the list of dicts we promise the user:
        
        CTCLObject result;
        result.Bind(interp);
        
        for (int i =0; i < rawResult.size(); i++) {
            CTCLObject item;
            item.Bind(interp);
            
            createAppDictionary(item, rawResult[i]);
            
            result += item;
        }
        
        interp.setResult(result);
    }
    catch(...) {
        delete pSupplied;
        throw;
    }
    
    
    delete pSupplied;
}
/**
 * queryStateTransitions
 *    Queries the set of state transitions that have occured.  This
 *    will produce a list of dicts.  Each dict will have a subdict called
 *    'application' that contains the application information. Additional keys:
 *    
 *    -  id        - Id of the transition.
 *    -  timestamp - [clock seconds] at which the transition message was emitted.
 *    -  leaving   - Name of the state being left.
 *    -  entering  - Name of the state being entered.
 *
 * @param interp - Interpreter that is running the command.
 * @param objv   - Command words.  In addition to the command name and subcommand,
 *                 this can optionally have a filter command name.
 */
void
CTCLStatusDb::CTCLStatusDbInstance::queryStateTransitions(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireAtMost(objv, 3);
    
    // Sort out what the query filter will be:
    
    CQueryFilter* pFilter(&::DAQ::acceptAll);
    CQueryFilter* pSupplied(0);
    
    if (objv.size() == 3) {
        pSupplied = createRawFilter(interp, objv[2]);
        pFilter = pSupplied;
    }
    // The try/catch block ensures any pSupplied is destroyed.
    
    try {
        // Do the actual underlying query:
        
        std::vector<CStatusDb::StateTransition> rawResult;
        m_pDb->queryStateTransitions(rawResult, *pFilter);
        
        // Map the raw result into the final result and set it in the interp:
        
        CTCLObject result;
        result.Bind(interp);
        for (int i =0; i < rawResult.size(); i++) {
            
            CTCLObject item;
            item.Bind(interp);
            
            createTransitionDict(item, rawResult[i]);
            
            result += item;
        }
        
        interp.setResult(result);
    }
    catch(...) {
        delete pSupplied;
        throw;
    }
    
    delete pSupplied;
}
/**
 * listReadoutApps
 *    Returns a list of applications that are registered as readout programs.
 *    This is a list of dicts that are application dicts.
 *
 *  @param interp - interpreter that's executing the command.
 *  @param objv   - The command words.  In addition to the command and subcommand
 *                  this can have an optional filter command.
 */
void
CTCLStatusDb::CTCLStatusDbInstance::listReadoutApps(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireAtMost(objv,3);
    
    // Figure out the filter for the query:
    
    CQueryFilter* pFilter(&::DAQ::acceptAll);
    CQueryFilter* pSupplied(0);
    
    if (objv.size() == 3) {
        pSupplied = createRawFilter(interp, objv[2]);
        pFilter = pSupplied;
    }
    // This try/catch block ensure pSupplied gets deleted.  Note delete 0 is a noop.
    
    try {
        // do the undelying query to get the raw results:
        
        std::vector<CStatusDb::ReadoutApp> rawResult;
        m_pDb->listReadoutApps(rawResult, *pFilter);
        
        // Turn that vector into a list of dicts that get set as the result:
        
        CTCLObject result;
        result.Bind(interp);
        
        for (int i =0; i < rawResult.size(); i++) {
            CTCLObject item;
            item.Bind(interp);
            
            createAppDictionary(item, rawResult[i]);   // ReadoutApp aliased to StateApp.
            
            result += item;
        }
        
        interp.setResult(result);
    }
    catch(...) {
        delete pSupplied;
        throw;
    }
    
    delete pSupplied;
    
}
/**
 * listRuns
 *    Produces a list of the runs each application has produced.
 *    This is orgainzed as a dict indexed by the application's id (primary key).
 *    The contents of each dict are a two element list containing
 *    the Application's dict and a list of run information dicts.  Each run
 *    information dict has the following keys:
 *
 *    - id - primary key of the run information entry.
 *    - startTime - When the run started ([clock seconds]).
 *    - runNumber - the run number.
 *    - runTitle  - The title of the run.
 *
 * @param interp - interpreter running the command.
 * @param objv   - Command words.  In addition to the command and subcommand,
 *                 this can have an optional filter command.
 */
void
CTCLStatusDb::CTCLStatusDbInstance::listRuns(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireAtMost(objv, 3);
    
    // Figure out the query filter to use.
    
    CQueryFilter* pFilter(&::DAQ::acceptAll);
    CQueryFilter* pSupplied(0);
    
    if(objv.size() == 3) {
        pSupplied = createRawFilter(interp, objv[2]);
        pFilter = pSupplied;
    }
    // from now on we need to ensure pSupplied is deleted if it was made.
    // Hence this try/catch blocK;
    
    try {
        // Perform the underyling query:
        
        CStatusDb::RunDictionary rawResult;         // std::map.
        m_pDb->listRuns(rawResult, *pFilter);
        
        CTCLObject result;
        result.Bind(interp);
        
        // Iterate over the map to produce the dict.  Note that the keys
        // are numeric and must therefore be turned into strings to avoid
        // the proliferation of TclMessageUtilities::addToDictionary overloads.
        
        for (auto pRaw = rawResult.begin(); pRaw != rawResult.end(); pRaw++) {
            
            // Pick apart the map iteration:
            
            unsigned iKey = pRaw->first;
            CStatusDb::ApplicationRun&       appRuns(pRaw->second);
            CStatusDb::ReadoutApp&           appInfo(appRuns.first);
            std::vector<CStatusDb::RunInfo>& runs(appRuns.second);
            
            // Turn the key into some text we'll just use Tcl for that:
            
            CTCLObject keyobj;
            keyobj.Bind(interp);
            keyobj = int(iKey);
            std::string key = keyobj;           // All objs have string reps
            
            // Produce the value for that key:
            
            CTCLObject value;
            value.Bind(interp);
            
            // First element of value is the readout program info.
            
            CTCLObject app;
            app.Bind(interp);
            createAppDictionary(app, appInfo);
            
            value += app;
            
            CTCLObject runList;
            runList.Bind(interp);
            
            for (int i =0; i < runs.size(); i++) {
                CTCLObject run;
                run.Bind(interp);
                createRunDictionary(run, runs[i]);
                
                runList += run;
            }
            // Second element of value is the run list:
            
            value += runList;
            
            // Fill in the dict item:
            
            TclMessageUtilities::addToDictionary(interp, result, key.c_str(), value);
        }
        interp.setResult(result);
        
    }
    catch(...) {
        delete pSupplied;
        throw;
    }
    
    delete pSupplied;
}
/**
 * queryReadoutStatistics
 *   Performs a query that returns full Readout statistics information.  This returns
 *   a dict indexed on the primary key of readout programs.  Each key's value
 *   is a pair that consists of a Readout Application dict and a list of pairs.
 *   The list of pairs contains a run information dict and a vector of readout
 *   statistics dicts.  Each of those dicts has the following keys:
 *
 *   -  id - Primary key of the record in the database table.
 *   -  timestamp - the [clock seconds] at which the statistics were emitted.
 *   -  elapsedTime - the number of seconds into the run at which the statistics
 *                   were emitted.
 *   -  triggers  - The number of triggers the program has reacted to.
 *   -  events    - The number of events the program has emitted.
 *   -  bytes     - The number of bytes the program has emitted.
 *
 * @param interp - interpreter that is executing the command.
 * @param objv   - The command words.  This can have an optional filter command
 *                 in addition to the mandatory command and subcommands.
 */
void
CTCLStatusDb::CTCLStatusDbInstance::queryReadoutStatistics(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireAtMost(objv, 3);
    
    // Figure out how to filter the query,.
    
    CQueryFilter* pFilter(&::DAQ::acceptAll);
    CQueryFilter* pSupplied(0);
    
    if (objv.size() == 3) {
        pSupplied = createRawFilter(interp, objv[2]);
        pFilter   = pSupplied;
    }
    
    // Everything now must be in a try/catch block to ensure that any
    // raw filter created above gets destroyed.
    
    try {
        //  Perform the query to get the raw result:
        
        CStatusDb::ReadoutStatDict rawResult;
        m_pDb->queryReadoutStatistics(rawResult, *pFilter);
        
        // Now map this to the interpreter result described in the comment header:
        
        CTCLObject result;
        result.Bind(interp);
        
        // First iterate over the map that contains this all
        
        for(auto pItem = rawResult.begin(); pItem != rawResult.end(); pItem++) {
            
            // pull apart the chunks of the map and its key - key is an int we'll
            // need to change into a string.
            
            unsigned iKey = pItem->first;
            CStatusDb::ReadoutAppStats&            appStats(pItem->second);
            CStatusDb::ReadoutApp&                 rdoApp(appStats.first);
            std::vector<CStatusDb::RunStatistics>& runStats(appStats.second);
            
            // Build up the item and the stringified key:
            
            CTCLObject keyObj;
            keyObj.Bind(interp);
            keyObj = int(iKey);
            std::string key = keyObj;
            
            CTCLObject value;
            value.Bind(interp);
            
            CTCLObject rdoAppDict;
            rdoAppDict.Bind(interp);
            createAppDictionary(rdoAppDict, rdoApp);
            value += rdoAppDict;
            
            CTCLObject appStatsObj;
            appStatsObj.Bind(interp);
            
            // Loop over the applications and the runs it's created:
            
            for (int ap = 0; ap != runStats.size(); ap++) {
                CStatusDb::RunInfo                         run = runStats[ap].first;
                std::vector<CStatusDb::ReadoutStatistics>& stats= runStats[ap].second;
                
                // The run info and stats go into an object that is appended to
                // appStatsObj:
                
                CTCLObject runStatObject;
                runStatObject.Bind(interp);
                
                CTCLObject runInfoDict;
                runInfoDict.Bind(interp);
                createRunDictionary(runInfoDict, run);
                runStatObject += runInfoDict;
                
                CTCLObject runStats;                 // List of readout stats dicts.
                runStats.Bind(interp);
                
                // Loop over the run statistics:
                
                for (int r = 0; r < stats.size(); r++) {
                    CTCLObject statDict;
                    statDict.Bind(interp);
                    createRunStatsDict(statDict, stats[r]);
                    
                    runStats+= statDict;
                }
                
                runStatObject += runStats;
                appStatsObj   += runStatObject;
            
            }
            
            
            value += appStatsObj;
            TclMessageUtilities::addToDictionary(interp, result, key.c_str(), value);
        }
        
        interp.setResult(result);
    }
    catch (...) {
        delete pSupplied;
        throw;
    }
    
    
    
    delete pSupplied;
}
/*----------------------------------------------------------------------------
 * CTCLStatusDb::CTCLStatusDbInstance utilities
 */

/**
 * marshallMessage
 *    Takes a set of command words that are Tcl byte array objects and turns
 *    them into a vector of zmq::message_t* objects.
 *
 *   @param   interp - references the interpreter running the command.
 *   @param   objv   - full command vector.
 *   @param   start  - index of first message part.
 *   @return  std::vector<zmq::message_t*>
 *   
 */
std::vector<zmq::message_t*>
CTCLStatusDb::CTCLStatusDbInstance::marshallMessage(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv, unsigned start
)
{
    std::vector<zmq::message_t*> result;
    try {
        for (int i = start; i < objv.size(); i++) {
            int nBytes;
            char* part = reinterpret_cast<char*>(
                Tcl_GetByteArrayFromObj(objv[i].getObject(), &nBytes)
            );
            zmq::message_t* msgPart = new zmq::message_t(nBytes);
            std::memcpy(msgPart->data(), part, nBytes);
            result.push_back(msgPart);
        }
    }
    catch (...) {
        freeMessageVector(result);
        throw;
    }
    return result;
}
/**
 *  decodeRingIdDict
 *      Break down a ring id dict from the Tcl decode into a ring id message
 *     part (note that this is dynamically malloc-d and must be deleted by
 *     the caller eventually).
 *
 *  @param interp    - Interpreter executing the current command.
 *  @param dict      - CTCLObject encapsulated dictionary.
 *  @return CStatusDefinitions::RingStatIdentification*
 */
CStatusDefinitions::RingStatIdentification*
CTCLStatusDb::CTCLStatusDbInstance::decodeRingIdDict(
    CTCLInterpreter& interp, CTCLObject& obj
)
{
   uint64_t timestamp =  TclMessageUtilities::getLongFromDictItem(interp, obj, "timestamp");
    std::string name   = TclMessageUtilities::getStringFromDictItem(interp, obj, "name");
    
    // The Status message class Creates a ring id struct but it puts the
    // current timestamp into it:
    
    CStatusDefinitions::RingStatIdentification* result =
        CStatusDefinitions::makeRingid(name.c_str());
    result->s_tod = timestamp;
    
    return result;
}
/**
 * decodeRingClient
 *     Decodes a ring client object from a dictionary.
 *
 *  @param interp - interpreter used to decode the dict.
 *  @param dict   - The dictionary.
 *  @return CStatusDefinitions::RingStatClient* - On errors an exception
 *        is thrown.
 */
CStatusDefinitions::RingStatClient*
CTCLStatusDb::CTCLStatusDbInstance::decodeRingClientDict(
    CTCLInterpreter& interp, CTCLObject& obj
)
{  
    uint64_t ops =
        TclMessageUtilities::getLongFromDictItem(interp, obj, "ops");
    uint64_t bytes =
        TclMessageUtilities::getLongFromDictItem(interp, obj, "bytes");
    bool isProducer =
        TclMessageUtilities::getBoolFromDictItem(interp, obj, "producer");
    uint64_t backlog = TclMessageUtilities::getLongFromDictItem(interp, obj, "backlog");
    pid_t    pid     = TclMessageUtilities::getLongFromDictItem(interp, obj, "pid");
    std::vector<std::string> command =
        TclMessageUtilities::getStringListFromDictItem(interp, obj, "command");
    
    return CStatusDefinitions::makeRingClient(ops, bytes, backlog, pid, isProducer, command);
}
/**
 * freeMessageVector
 *    Free the elements of  an std::vector<zmq::message_t*>
 * @param message - vector of message parts.
 */
void
CTCLStatusDb::CTCLStatusDbInstance::freeMessageVector(
    std::vector<zmq::message_t*>& message
)
{
    for (int i = 0; i < message.size(); i++) {
        delete message[i];
    }
}
/**
 * freeRingClients
 *     Free's the ring clients in a vector:
 *
 *  @param clients - vetor of clients
 */
void
CTCLStatusDb::CTCLStatusDbInstance::CTCLStatusDbInstance::freeRingClients(
    std::vector<const CStatusDefinitions::RingStatClient*>& clients
)
{
    for (size_t i = 0; i < clients.size(); i++) {
        std::free(const_cast<CStatusDefinitions::RingStatClient*>(clients[i]));
    }
}
/**
 * decodeReadoutCounterStats
 *    Decodes a readout counter dict (from e.g. decode) into a
 *    ReadoutStatCounters struct.
 *
 *  @param counters - reference to the result counters.
 *  @param interp   - The interpreter used to decode stuff.
 *  @param obj      - The dict. object.
 */
void
CTCLStatusDb::CTCLStatusDbInstance::decodeReadoutCounterStats(
    CStatusDefinitions::ReadoutStatCounters& counters,
    CTCLInterpreter& interp, CTCLObject& obj
)
{
    counters.s_tod         = TclMessageUtilities::getLongFromDictItem(interp, obj, "timestamp");
    counters.s_elapsedTime = TclMessageUtilities::getLongFromDictItem(interp, obj, "elapsed");
    counters.s_triggers    = TclMessageUtilities::getLongFromDictItem(interp, obj, "triggers");
    counters.s_events      = TclMessageUtilities::getLongFromDictItem(interp, obj, "events");
    counters.s_bytes       = TclMessageUtilities::getLongFromDictItem(interp, obj, "bytes");
    
}
/**
 * createLogRecordDict
 *    Create a log record dictionary.
 *
 *  @param dict - the dictionary that will be created.
 *  @param record - the raw log message record.
 *  @note dict is assumed bound to an interpreter.
 */
void
CTCLStatusDb::CTCLStatusDbInstance::createLogRecordDict(CTCLObject& dict, CStatusDb::LogRecord& record)
{
    CTCLInterpreter* interp = dict.getInterpreter();
    
    TclMessageUtilities::addToDictionary(*interp, dict, "id", record.s_id);
    TclMessageUtilities::addToDictionary(
        *interp, dict, "severity",
        record.s_severity.c_str()               // Put the string severity in.
    );
    TclMessageUtilities::addToDictionary(
        *interp, dict, "application", record.s_application.c_str()
    );
    TclMessageUtilities::addToDictionary(*interp, dict, "source", record.s_source.c_str());
    TclMessageUtilities::addToDictionary(*interp, dict, "timestamp", record.s_timestamp);
    TclMessageUtilities::addToDictionary(*interp, dict, "message", record.s_message.c_str());
}
/**
 * createRingInfoDict
 *    Creates a dict that describes the information about a ring.
 *    See listRings above for the key/value pairs.
 *
 *  @param result - reference to a CTCLObject that will be set to the
 *                  require dict.  Note that result must be bound to an interpreter
 *                  and that interpreter will be used to handle all the
 *                  conversions.
 *  @param rec    - References a CStatusDb::RingBuffer object that contains
 *                  the raw record to be converted.
 */
void
CTCLStatusDb::CTCLStatusDbInstance::createRingInfoDict(CTCLObject& result, CStatusDb::RingBuffer& rec)
{
    CTCLInterpreter* pInterp = result.getInterpreter();
    
    TclMessageUtilities::addToDictionary(*pInterp, result, "id",     rec.s_id);
    TclMessageUtilities::addToDictionary(*pInterp, result, "name",   rec.s_name.c_str());
    TclMessageUtilities::addToDictionary(*pInterp, result, "host",   rec.s_host.c_str());
    TclMessageUtilities::addToDictionary(*pInterp, result, "fqname", rec.s_fqname.c_str());
}
/**
 *  createRingClientDict
 *      Create a dictionary that contains information about a ring client.
 *      See listRingsAndClients for information about what that dict looks like.
 *
 *  @param dict - the dictionary we're creating/adding to.  Note this object
 *                must be bound to an interpreter and that interp. will be used
 *                to construct the dict.
 *  @param rec  - the CStatusDb::RingClient being translated
 */
void
CTCLStatusDb::CTCLStatusDbInstance::createRingClientDict(CTCLObject& dict, CStatusDb::RingClient& rec)
{
    CTCLInterpreter* pInterp = dict.getInterpreter();
    
    TclMessageUtilities::addToDictionary(*pInterp, dict, "id",         rec.s_id);
    TclMessageUtilities::addToDictionary(*pInterp, dict, "pid",        rec.s_pid);
    TclMessageUtilities::addToDictionary(*pInterp, dict, "isProducer", rec.s_isProducer);
    TclMessageUtilities::addToDictionary(*pInterp, dict, "command",    rec.s_command.c_str());
}
/**
 * createRingStatisticsDict
 *    Creates a dict containing one ring statistics record.   See
 *    queryRingStatistics for the keys in this dict.
 *
 *  @param dict - the dict that we'll put key/value pairs into.
 *  @param rec  - The CStatusDb::RingStatistics struct we're converting to a dict.
 */
void
CTCLStatusDb::CTCLStatusDbInstance::createRingStatisticsDict(
    CTCLObject& dict, CStatusDb::RingStatistics& rec
)
{
    CTCLInterpreter* pInterp  = dict.getInterpreter();
    
    TclMessageUtilities::addToDictionary(*pInterp, dict, "id",         rec.s_id);
    TclMessageUtilities::addToDictionary(*pInterp, dict, "timestamp",  rec.s_timestamp);
    TclMessageUtilities::addToDictionary(*pInterp, dict, "operations", rec.s_operations);
    TclMessageUtilities::addToDictionary(*pInterp, dict, "bytes",      rec.s_bytes);
    TclMessageUtilities::addToDictionary(*pInterp, dict, "backlog",   rec.s_backlog);
}
/**
 * createAppDictionary
 *    Creates an application dictionary from an underlying application structs.
 *    This recognizes that state and readout applications currently share
 *    the same struct.
 *    See listStateApplications for information about the contents of the dict.
 *
 *  @param dict - Reference to the dict we're going to fill in.  Note that
 *                this object must already be bound to an interpreter and that
 *                interpreter is used in building the dict.
 *  @param rec - References a CStatusDb::StateApp struct that will be converted.
 */
void
CTCLStatusDb::CTCLStatusDbInstance::createAppDictionary(
    CTCLObject& dict, CStatusDb::StateApp& rec
)
{
    CTCLInterpreter* pInterp = dict.getInterpreter();

    TclMessageUtilities::addToDictionary(*pInterp, dict, "id",   rec.s_id);
    TclMessageUtilities::addToDictionary(*pInterp, dict, "name", rec.s_appName.c_str());
    TclMessageUtilities::addToDictionary(*pInterp, dict, "host", rec.s_appHost.c_str());
}
/**
 * createTransitionDict
 *    Creates the state transition outer and inner dicts.
 *
 * @param dict - Dict being filled in.
 * @param rec - CStatusDb::StateTransition record.
 */
void
CTCLStatusDb::CTCLStatusDbInstance::createTransitionDict(
    CTCLObject& dict, CStatusDb::StateTransition& rec
)
{
    CTCLInterpreter* pInterp = dict.getInterpreter();
    
    // the inner dict from ths s_app field:
    
    CTCLObject innerDict;
    innerDict.Bind(*pInterp);
    
    createAppDictionary(innerDict, rec.s_app);
    TclMessageUtilities::addToDictionary(*pInterp, dict, "application", innerDict);
    TclMessageUtilities::addToDictionary(*pInterp, dict, "id",        rec.s_transitionId);
    TclMessageUtilities::addToDictionary(*pInterp, dict, "timestamp", rec.s_timestamp);
    TclMessageUtilities::addToDictionary(*pInterp, dict, "leaving",   rec.s_leaving.c_str());
    TclMessageUtilities::addToDictionary(*pInterp, dict, "entering", rec.s_entering.c_str());
}
/**
 * createRunDictionary
 *    Takes a CStatusDb::RunInfo struct and creates a dict that represents h
 *    that struct.
 *
 *  @param dict   - The dict into which we'll put keys and values.
 *  @param rec    - The RunInfo struct we're mapping to a dict.
 */
void
CTCLStatusDb::CTCLStatusDbInstance::createRunDictionary(
    CTCLObject& dict, CStatusDb::RunInfo& rec
)
{
    CTCLInterpreter* pInterp = dict.getInterpreter();
    
    TclMessageUtilities::addToDictionary(*pInterp, dict, "id",        rec.s_id);
    TclMessageUtilities::addToDictionary(*pInterp, dict, "startTime", rec.s_startTime);
    TclMessageUtilities::addToDictionary(*pInterp, dict, "runNumber", rec.s_runNumber);
    TclMessageUtilities::addToDictionary(*pInterp, dict, "runTitle",  rec.s_runTitle.c_str());
}
/**
 * createRunStatsDict
 *    Create a run statistics dictionary from a run statistics struct.
 *
 * @param dict  - the resulting dict.  This object must have already been
 *                bound to an interpreter.  That interpreter will be used when
 *                constructing the dict.
 * @param rec  - Reference to the CStatusDb::ReadouStatistics item that will
 *               be mapped to a dict.
 */
void
CTCLStatusDb::CTCLStatusDbInstance::createRunStatsDict(
    CTCLObject& dict, CStatusDb::ReadoutStatistics& rec
)
{
    CTCLInterpreter* pInterp = dict.getInterpreter();
    
    TclMessageUtilities::addToDictionary(*pInterp, dict, "id",          rec.s_id);
    TclMessageUtilities::addToDictionary(*pInterp, dict, "timestamp",   rec.s_timestamp);
    TclMessageUtilities::addToDictionary(*pInterp, dict, "elapsedTime", rec.s_elapsedTime);
    TclMessageUtilities::addToDictionary(*pInterp, dict, "triggers",    rec.s_triggers);
    TclMessageUtilities::addToDictionary(*pInterp, dict, "events",      rec.s_events);
    TclMessageUtilities::addToDictionary(*pInterp, dict, "bytes",       rec.s_bytes);
}

/**
 * createRawFilter
 *    Takes a Tcl filter and turns it into a CRawQueryFilter object.  The
 *    resulting filter can be used the CStatusDb query methods to filter
 *    the output. Note that the filter we get is assumed to be a Tcl command
 *    that has a [toString] subcommand which, when executed, provides us with
 *    the query string we can use to construct the raw filter.
 *
 *  @param interp - The Tcl interpreter that will be used to extract the raw filter
 *                  string from the command.
 *  @param filter - A CTCL object that contains the filter command.
 *  @return CQueryFilter*  - The resulting filter.  Not that this is dynamically
 *                  instantiated and therefore must be deleted by the caller at
 *                  some point.
 */
CQueryFilter*
CTCLStatusDb::CTCLStatusDbInstance::createRawFilter(
    CTCLInterpreter& interp, CTCLObject& tclFilter
)
{
    Tcl_Interp*  rawInterp = interp.getInterpreter();
    
    // Create the command we want to execute, and run it:
    
    tclFilter += "toString";
    int status = Tcl_EvalObjEx(
        rawInterp, tclFilter.getObject(), TCL_EVAL_GLOBAL | TCL_EVAL_DIRECT
    );
    if (status != TCL_OK) {
        throw CTCLException(interp, status, "Attempting to evaluate Tcl Filter");
    }
    // The result of the command is the string we want.. use that to
    // construct a Raw query filter to return:
    
    const char* queryText = Tcl_GetStringResult(rawInterp);
    CQueryFilter* result =  new CRawFilter(std::string(queryText));
    Tcl_ResetResult(rawInterp);                  // Don't let this linger.
    
    return result;
}
