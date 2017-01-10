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
# @file   PyStatusDb.cpp
# @brief  Python Bindings to CStatusDb
# @author <fox@nscl.msu.edu>
*/

#include <Python.h>
#include <CStatusDb.h>
#include <CStatusMessage.h>
#include <sqlite3.h>
#include <CSqliteWhere.h>


#include <string>
#include <stdexcept>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

static PyObject* exception(0);

typedef struct _statusdb_Data {
    PyObject_HEAD
    CStatusDb*   m_pApi;
} StatusDb, *pStatusDb;

// utilities:

/**
 * getApi
 *    Returnt he API object pointer.
 *
 *  @param self - Pointer to object data.
 *  @return CStatusDb*
 */
static CStatusDb*
getApi(PyObject* self)
{
    pStatusDb pThis = reinterpret_cast<pStatusDb>(self);
    return pThis->m_pApi;
}
/**
 * Dictionary utilities:
 */

/**
 *  getDictItem
 *    Return the object associated with a specific dictionary key or throw a
 *    std::string exception if there's no match.
 *
 *  @param dict - the dictionary to interrogate.  It's the caller's responsibility
 *                to ensure the object is a dict.
 *  @param key  - The key to return - must be a string.
 *  @return PyObject* The object at tyhat key.
 *  @throw std::string - unable to fetch the specified object.
 */
static PyObject*
getDictItem(PyObject* dict, const char* key)
{
    PyObject* result = PyDict_GetItemString(dict, key);
    if (result == nullptr) {
        std::string message = "Dictionary does not have a key value: ";
        message += key;
        throw message;
    }
    return result;
}
/**
 * getDictUint64Item
 *    Return a value from a dict that is a uint64_t.  This requires that the
 *    item at the key be an integer like entity.  If this is not the case,
 *    an std::string exception is thrown.
 *
 *  @param dict - the dict we are interrogating.  Caller must ensure this is a dict.
 *  @param key  - The key we want to retrieve.
 *  @return uint64_t - value associated with key.
 */
static std::uint64_t
getDictUint64Item(PyObject* dict, const char* key)
{
    PyObject* intObj = getDictItem(dict, key);        // can throw if no key.
    uint64_t result  = PyInt_AsUnsignedLongMask(intObj);
    if (PyErr_Occurred()) {
        std::string message = "The item at: ";
        message += key;
        message += " must be an integer value but is not";
        throw message;
    }
    return result;
}
/**
 * getDictBoolItem
 *    Returns a boolean value from the specified item in a dictionary.
 *
 *
 *   @param dict - the dictionary to fetch from.
 *   @param key  - the item's key in the dict.
 *   @return int - nonzero for true and zero for false.
 *   @throws std::string in the event the item is not a bool.
 *   @note the item _must_ be a Python Bool.  It cannot be an int e.g.
 */
static int
getDictBoolItem(PyObject* dict, const char* key)
{
    PyObject* item = getDictItem(dict, key);        // throws if no key.
    if (item == Py_True) return 1;
    if (item == Py_False) return 0;
    
    // Not a bool so:
    
    std::string message = "The item at: ";
    message += key;
    message += " must be a Boolean but is not";
    throw message;
}

/**
 * dictStoreObj
 *    Store an object in a dict at the specified key.
 *
 *  @param dict - the dictionary to store to.
 *  @param key  - the key to store at.
 *  @param valobj - the value obj.
 *
 *  @throws std::string if dict is not actually a dict. or if PyDict_SetItemString
 *          returned an error.
 */
static void
dictStoreObj(PyObject* dict, const char* key, PyObject* valobj)
{
    if (!PyDict_Check(dict)) {
        throw std::string("dictStoreObj first parameter is not a dict");
    }
    if (PyDict_SetItemString(dict, key, valobj)) {
        std::string message("doctStoreObj store to dict key: ");
        message += key;
        message += " failed";
        throw message;
    }
}
/**
 * dictStoreInt
 *   Store an integer valued item in a dict.
 * @param dict - the dict we're storing into.
 * @param key  - Key to store to.
 * @param value - the integer to store.
 */
static void
dictStoreInt(PyObject* dict, const char* key, long value)
{
    PyObject* valObj = PyInt_FromLong(value);
    if (!valObj) {
        throw std::string("Unable to create an integer object in dictStoreInt");
    }
    dictStoreObj(dict, key, valObj);
}
/**
 * dictStoreString
 *    Store a string valued object into a specified dict key.
 *
 *  @param dict - The dictionary object.
 *  @param key  - Value of the key.
 *  @param value - The value to store.
 */
static void
dictStoreString(PyObject* dict,  const char* key, const char* string)
{
    PyObject* strObj = PyString_FromString(string);
    if (!strObj) {
        throw std::string("Unable to create a string object in dictStoreString");
    }
    dictStoreObj(dict, key, strObj);
}
/**
 * dictStoreBool
 *    Store a boolean value:
 *
 *  @param dict - dict being updated.
 *  @param key  - String key.
 *  @param value bool.
 */
static void
dictStoreBool(PyObject* dict, const char* key, bool value)
{
    PyObject* vobj = value ? Py_True : Py_False;
    Py_INCREF(vobj);
    
    dictStoreObj(dict, key, vobj);
}
/**
 * Generic utilities for iterable objects:
 */

/**
 * stringListToVector
 *    Given an iterable composed uniformly of strings, converts it to an
 *    std::vector<std::string>
 *
 *  @param item - iterable item.
 *  @return std::vector<std::string>
 */
static std::vector<std::string>
stringListToVector(PyObject* item)
{
    std::vector<std::string> result;
    // Get the iterator - throw if it's not an iterable:
    
    
    PyObject* o(nullptr);
    PyObject* i = PyObject_GetIter(item);
    if (!i) {
        throw std::string("Parameter is not an iterable and has to be");
    }
    
    // We use try/catch to ensure that errors won't leak resources
    
    
    try {
        while (o = PyIter_Next(i)) {
            const char* s = PyString_AsString(o);
            if (!s) {
                throw std::string("Iterable must only have string objects but has other types");  
            }
            result.push_back(std::string(s));
             
             Py_DECREF(o);
        }
        Py_DECREF(i);
    }
    catch (std::string) {
        Py_XDECREF(o);
        Py_XDECREF(i);
        throw;
    }
    return result;
}

/**
 *  Utilities for addRingStatistics:
*/

/**
 * freeRingResources
 *    
 *    Given a ring id object and a possibly empty vector of ring status clients,
 *    frees the memory associated with all of them.
 *
 *  @param id - the ring id item pointer.  If you don't have one, just supply
 *              nullptr.
 *  @param clients - vector of pointers to clients (by reference).  On exit this
 *              will be empty.
 */
static void
freeRingResources(
    CStatusDefinitions::RingStatIdentification* id,
    std::vector<const CStatusDefinitions::RingStatClient*>& clients
)
{
    std::free(id);               // free(nullptr) is a no-op.
    while(!clients.empty()) {
        CStatusDefinitions::RingStatClient* pItem =
            const_cast<CStatusDefinitions::RingStatClient*>(clients.back());
        std::free(pItem);
        clients.pop_back();     // back to front is O(n) from front is O(n**2).
    }
}

/**
 * packRingId
 *    Packes a decoded ring id dictionary back into a
 *    CStatusDefinitions::RingStatIdentification struct.
 *    
 *
 *  @param[out] r - A pointer to a RingStatIdentification* pointer.
 *                       the result is dynamically allocated and filled in.
 *                       Normally the storage is freed via a all to freeRingResources
 *                       made by the caller.
 *  @param[in] dict   - A Python object that must be a dict and must have all the
 *                      keys of the dict created by PyStatusMessages.cpp:msg_encodeRingId
 *                      The elements of this dict are what will be packed into the
 *                      final result.
 *
 *  @note that if we come across an error we'll raise a Python exception (not
 *       a C++ one), so the caller will need to ensure that they check the error
 *       flag. r will have a nullptr stored in this case as well.
 */
static void
packRingId(CStatusDefinitions::RingStatIdentification** r, PyObject* dict)
{
    CStatusDefinitions::RingStatIdentification* result(nullptr);
    
    // We need to ensure that dict actually is one.
    
    if (!PyDict_Check(dict)) {
        PyErr_SetString(exception, "ring description parameter must be a dict");
        *r = nullptr;
        return;
    }
    // We need the name and timestamp objects from the dict:
    
    PyObject* tsObj   = PyDict_GetItemString(dict, "timestamp");
    PyObject* nameObj = PyDict_GetItemString(dict, "name");
    
    // Use exceptions to simplify error management:
    
    try {
        // If either one of those objects in nullptr, a needed dict key is missing:
        
        if ((!tsObj) || (!nameObj)) {
            throw std::string("The ring description dict is missing either a 'timestamp' or 'name' key");
        }
        // Extract the two items.  We need to know how long the nameObj string is
        // in order to know  how much storage to allocate for the result:
        
        std::uint64_t timestamp = PyInt_AsLong(tsObj);
        char*    name      = PyString_AsString(nameObj);
        if ((timestamp == -1) || (name == nullptr)) {
            throw std::string("Either the timestamp cannot be converted to an integer or the name is not a string");
        }
        // Allocate storage and fill it in:
        
        result = CStatusDefinitions::makeRingid(name);
        result->s_tod = timestamp;                    // Adjust for tod we're given.
        std::strcpy(result->s_ringName, name);
    }
    catch (std::string msg) {
        PyErr_SetString(exception, msg.c_str());
        *r = nullptr;
        return;
    }
    
    *r = result;
}
/**
 * packRingClient
 *    Packs a single ring client into a CStatusDefinitions::RingStatClient.
 *
 *  @param dict - A python dict object that must have at least the keys
 *                created by pyStatusMessages.cpp:msg_encodeRingClient.
 *  @return CStatusDefinitions::RingStatClient* - dynamically allocated, must
 *          be freed by the caller.
 *  @throws std::string - if errors are detected and that typically means that:
 *          -  dict isn't
 *          -  dict is missing a required key.
 *          -  One of the values in the dict can't be converted to the required
 *             type.
 */
static CStatusDefinitions::RingStatClient*
packRingClient(PyObject* dict)
{
    if (!PyDict_Check(dict)) {
        throw std::string("Ring statistics client struct can only be packed from a dict");
    }
    // Pull all the objects from the dict -- note that getDictxxxItem throws std::string
    // if a key is missing:
    
    std::uint64_t  ops      = getDictUint64Item(dict, "operations");
    std::uint64_t  bytes    = getDictUint64Item(dict, "bytes");
    int       isProd   = getDictBoolItem(dict, "producer");
    PyObject* cmdListObj= getDictItem(dict, "command");
    std::vector<std::string> command = stringListToVector(cmdListObj);
    std::uint64_t  backlog  = getDictUint64Item(dict, "backlog");
    std::uint64_t  pid      = getDictUint64Item(dict, "pid");
    
    return CStatusDefinitions::makeRingClient(
        ops, bytes, backlog, pid, static_cast<bool>(isProd), command
    );
    
}

/**
 *  packRingClients
 *    Given an iterable object where each object is a dict of the type
 *    created by PyStatusMessages.cpp:msg_encodeRingClient, creates a vector
 *    of RingStatClient pointers.
 *
 * @param r  - Reference to the vector to which pointers to the generated
 *             objects will be appended.  Note that all objects will be
 *             dynamically allocated and must be freed by the caller.  This is
 *             normally done by calling freeRingResources.
 * @param iterable - PyObject that must be iterable and whose members must
 *             by dicts with at least the keys created by
 *             PyStatusMessages.cpp:msg_encodeRingClient   If the iterable
 *             is null, this indicates there are no clients and a return is
 *             done without modifying r.
 * @note Errors are reported by raising a _python_ exception not a C++ exception.
 *       the caller should check the state of the python error flag and
 *       operate accordingly if it is set.
 * @note In the event an error has been set, the result vector will be
 *       unchanged and no net storage will have been allocated. 
 */
static void
packRingClients(
    std::vector<const CStatusDefinitions::RingStatClient*>& r, PyObject* iterable
)
{
    if (!iterable) return;
    std::vector<const CStatusDefinitions::RingStatClient*> newItems;
    
    // Use a try/catch block to simplify error handling:
    
    PyObject* pIter(nullptr);
    try {
        // Iterable must be iterable:
        
        pIter = PyObject_GetIter(iterable);
        if( !pIter ) {
            throw std::string("The ring clients list must support iteration");
        }
        PyObject* dict(nullptr);
        while (dict = PyIter_Next(pIter)) {
            
            CStatusDefinitions::RingStatClient* client= packRingClient(dict);  // can throw std::string.
            newItems.push_back(client);
            
            Py_DECREF(dict);
        }
        Py_DECREF(pIter);
    }
    catch (std::string msg) {
        PyErr_SetString(exception, msg.c_str());
        freeRingResources(nullptr, newItems);
        if (pIter) {
            Py_DECREF(pIter);
        }
        return;
    }
    
    // Add the newly created items to the end of the vector.
    
    r.insert(r.end(), newItems.begin(), newItems.end());
}

/**
 * Utilities for addReadoutStatistics:
 */

/**
 *  unpackReadoutCounters
 *    Unpacks a dict of the sort created by PyStatusMessages.cpp:decodeRunCounters
 *    into a CStatusDefinitions::ReadoutStatCounters struct.
 *
 * @param[out] result - references the output struct.
 * @param[in]  dict   - Dictionary object.
 * @throws std::string - if there's an error.
 */
static void
unpackReadoutCounters(
    CStatusDefinitions::ReadoutStatCounters& result, PyObject* dict
)
{
    // ensure dict is one:
    
    if (!PyDict_Check(dict)) {
        throw std::string("Readout Counters must be a dict and is not");
    }
    
    // Note that getDictUint64Item can throw too.
    
    result.s_tod         = getDictUint64Item(dict, "timestamp");
    result.s_elapsedTime = getDictUint64Item(dict, "elapsed");
    result.s_triggers    = getDictUint64Item(dict, "triggers");
    result.s_events      = getDictUint64Item(dict, "events");
    result.s_bytes       = getDictUint64Item(dict, "bytes");
    
}
/**
 * Generic query utilities:
 */

/**
 *  createFilterObject
 *     Given a Python object that implements an appropriate toString entity
 *     returns a new'd CRawFilter object that was constructed on that string.
 *     It is the script level's responsibility to ensure that toString produces
 *     a valid WHERE clause.
 *
 *  @param filterObj - A Python filter object.  See nscldaq.sqlite.where for
 *                     sample (and useful) filter classes.
 *   @param CQueryFilter* - Pointer to a new filter object. The caller must ensure
 *                      this is eventually deleted.
 */
static CQueryFilter*
createFilterObject(PyObject* filterObj)
{
    PyObject* filterStringObj = PyObject_CallMethod(
        filterObj, const_cast<char*>("toString"), NULL
    );
    if (!filterStringObj) {
        throw std::string("createFilterObj - the object does not have a suitable 'toString' method");
    }
    // filterStringObj must be able to deliver a string:
    
    char* filterString = PyString_AsString(filterStringObj);
    if (!filterString) {
        throw std::string("createFilterObj - the object's 'toString' method does not produce a string");
    }
    // Create/return the result:
    
    return new CRawFilter(std::string(filterString));
}
/**
 * Utilities used by queryLogRecord
 */

/**
 * logRecordToDict
 *    Converts a Log record value to a python dict.  See
 *    statusdb_queryLogRecords for the keys created.
 *
 *  @param rec - the log record to convert.
 *  @return PyObject* that is a dict.
 */
static PyObject*
logRecordToDict(CStatusDb::LogRecord& rec)
{
    PyObject* result = PyDict_New();
    
    dictStoreInt(result, "id", rec.s_id);
    dictStoreInt(
        result, "severity",
        CStatusDefinitions::stringToSeverity(rec.s_severity.c_str())
    );
    dictStoreString(result, "application", rec.s_application.c_str());
    dictStoreString(result, "source", rec.s_source.c_str());
    dictStoreInt(result, "timestamp", rec.s_timestamp);
    dictStoreString(result, "message", rec.s_message.c_str());
    
    return result;
    
}
/**
 * logRecordsToDictTuple
 *    Converts a vector of CStatusDb::LogRecord structs into a tuple of
 *    log record dicts.
 *
 *  @param queryResults - the results of a call to CStatusDb::queryLogRecords.
 *  @return PyObject*   - a tuple (possibly empty) of log record dicts.
 */
static PyObject*
logRecordsToDictTuple(std::vector<CStatusDb::LogRecord>& queryResults)
{
    size_t itemCount = queryResults.size();
    PyObject* result = PyTuple_New(itemCount);
    
    for (int i = 0; i < itemCount; i++) {
        PyObject* logDict = logRecordToDict(queryResults[i]);
        if(PyTuple_SetItem(result, i, logDict)) {
            throw std::string("Unable to set a new item to the tuple of log item dicts");
        }
    }
    
    return result;
}

/**
 * Utilities used by ring statistics queries:
 */


/**
 * ringBufferToDict
 *   Given a CStatusDb::RingBuffer struct reference returns a dict that describes
 *   that struct.
 *
 * @param ring - CStatusDb::RingBuffer& for the item we are converting.
 * @return PyObject* - Dict that contains the pythonized version of the
 *                     ringbuffer definition.
 */
static PyObject*
ringBufferToDict(CStatusDb::RingBuffer& ring)
{
    PyObject* result = PyDict_New();
    
    dictStoreInt(result, "id", ring.s_id);
    dictStoreString(result, "fqname", ring.s_fqname.c_str());
    dictStoreString(result, "name", ring.s_name.c_str());
    dictStoreString(result, "host", ring.s_host.c_str());
    
    
    return result;
}

/**
 * ringListToTuple
 *    Converts a list of ring definitions into a tuple of dicts as described in
 *    statusdb_listRings
 *
 * @param raw - The raw result from CStatusDb::listRings
 * @result PyObject* The resulting tuple.
 */
static PyObject*
ringListToTuple(std::vector<CStatusDb::RingBuffer>& raw)
{
    // Make the tuple:
    
    PyObject* result = PyTuple_New(raw.size());     // one entry per raw element.
    for (int i = 0; i < raw.size(); i++) {
        PyObject* entry = ringBufferToDict(raw[i]);
        PyTuple_SetItem(result, i, entry);
    }
    
    return result;
}
/**
 * clientToDict
 *   Turn a client data structure into the dict descsribed in statusdb_listRingsAndClients
 *
 * @param client - refers to a single CStatusDb::RingClient struct.
 * @return PyObject* - dict created.
 */
static PyObject*
clientToDict(CStatusDb::RingClient& client)
{
    PyObject* result = PyDict_New();
    
    dictStoreInt(result, "id", client.s_id);
    dictStoreInt(result, "pid", client.s_pid);
    dictStoreBool(result, "producer", client.s_isProducer);
    dictStoreString(result, "command", client.s_command.c_str());
    
    return result;
}
/**
 * ringClientsToTuple
 *    Turn a vector of ring clients into a tuple of ring client dicts.
 *    See statusdb_listRingsAndClients for a description of the keys in each dict.
 *
 *  @param clients - A vector of clients for a ring (reference).
 *  @return PyObject* - tuple with one element per each client.
 */
static PyObject*
ringClientsToTuple(std::vector<CStatusDb::RingClient>& clients)
{
    PyObject* result = PyTuple_New(clients.size());
    
    for (int i = 0; i < clients.size(); i++) {
        PyTuple_SetItem(result, i, clientToDict(clients[i]));
    }
    
    return result;
}

/**
 * ringAndClientsToTuple
 *   Turns the ring and clients pair into a tuple of ringbuffer and tuple of
 *   ring client dicts.
 *
 * @param randc - Ring and clients.
 * @return PyObject* the created tuple.
 */
static PyObject*
ringAndClientsToTuple(CStatusDb::RingAndClients& randc) {
    PyObject* result = PyTuple_New(2);       // it's a pair:
    
    PyTuple_SetItem(result, 0, ringBufferToDict(randc.first));
    PyTuple_SetItem(result, 1, ringClientsToTuple(randc.second));
    
    return result;
}

/**
 * ringDirectoryToMap
 *   Takes the CStatusDb::RingDirectory and returns a dict as described
 *   in  statusdb_listRingsAndClients.
 *
 * @param raw - the raw query results.
 * @return PyObject* - The dict created.
 */
static PyObject*
ringDirectoryToMap(CStatusDb::RingDirectory& raw)
{
    PyObject* result = PyDict_New();
    
    for (auto p = raw.begin(); p != raw.end(); p++) {
        std::string key = p->first;
        CStatusDb::RingAndClients& value(p->second);
        PyObject* valueObj = ringAndClientsToTuple(value);
        
        dictStoreObj(result, key.c_str(), valueObj);
    }
    
    return result;
}
/**
 * statToDict
 *   Given a ring statistics item, return a dict that represents it:
 *
 *  @param stat - Single stastics item.
 *  @return PyObject* the dict.
 */
static PyObject*
statToDict(CStatusDb::RingStatistics& stat)
{
    PyObject* result = PyDict_New();
    
    dictStoreInt(result, "id", stat.s_id);
    dictStoreInt(result, "timstamp", stat.s_timestamp);
    dictStoreInt(result, "operations", stat.s_operations);
    dictStoreInt(result, "bytes", stat.s_bytes);
    dictStoreInt(result, "backlog", stat.s_backlog);
    
    return result;
}

/**
 * statVectorToTuple
 *    Given a reference to a vector of ring statistics, return a tuple of
 *    ring statistics dicts.
 *
 * @param stats - reference to the vector of stats.
 * @return PyObject* - the tuple created.
 */
static PyObject*
statVectorToTuple(std::vector<CStatusDb::RingStatistics>& stats)
{
    PyObject* result = PyTuple_New(stats.size());
    
    for (int i = 0; i < stats.size(); i++) {
        PyTuple_SetItem(result, i, statToDict(stats[i]));
    }
    
    return result;
}

/**
 * clientAndStatsToPair
 *    Given a reference to a CStatusDb::RingClientAndStats, create and return
 *    a pair containing a ring client dict and a tuple of statistics dicts.
 *
 *  @param clientAndstats - The struct to convert.
 *  @return PyObject*
 */
static PyObject*
clientAndStatsToPair(CStatusDb::RingClientAndStats& clientAndStats)
{
    PyObject* result = PyTuple_New(2);
    
    PyTuple_SetItem(result, 0, clientToDict(clientAndStats.first));
    PyTuple_SetItem(result, 1, statVectorToTuple(clientAndStats.second));
    
    return result;
}

/**
 * clientAndStatsVecTo Pairs
 *    Given a vector of RingClientAndStats, produce a tuple of pairs of of
 *    ring client and statistics tuple
 * @param vec - the vector of RingClientAndStats.
 * @return PyObject* tuple.
 */
static PyObject*
clientAndStatsVecToPairs(std::vector<CStatusDb::RingClientAndStats>& vec)
{
    PyObject* result = PyTuple_New(vec.size());
    
    for (int i = 0; i < vec.size(); i++) {
        PyTuple_SetItem(result, i, clientAndStatsToPair(vec[i]));
    }
    
    return result;
}

/**
 * ringClientAndStatsToPair
 *  Given a CStatusDb::RingsAndStatistics  reference, returns a pair consisting
 *  of ring statistics and a tuple with the clients and their associated
 *  statistics.
 */
static PyObject*
ringClientAndStatsToPair(CStatusDb::RingsAndStatistics& ringAndStats)
{
    PyObject* result = PyTuple_New(2);
    
    PyTuple_SetItem(result, 0, ringBufferToDict(ringAndStats.first));
    
    PyTuple_SetItem(result, 1, clientAndStatsVecToPairs(ringAndStats.second));
    
    return result;
}

/**
 * ringStatisticsToMap
 *   Pythonizes a complete set of ring statistics.
 *
 *  @param stats - Reference to a CStatusDb::CompleteRingStatistics structure.
 *  @return PyObject* dict as described in statusdb_queryRingStatistics
 */
static PyObject*
ringStatisticsToMap(CStatusDb::CompleteRingStatistics& stats)
{
    PyObject* result = PyDict_New();
    
    for (auto p = stats.begin(); p != stats.end(); p++) {
        std::string key = p->first;
        
        dictStoreObj(result, key.c_str(), ringClientAndStatsToPair(p->second));
    }
    
    return result;
}
/**
 * utilities for state transition queries:
 */

/**
 * stateAppToDict
 *    Takes a CStatusDb::StateApp struct as input and generates a dict
 *    that describes that struct.
 *
 *  @param app - the item to convert.
 *  @return PyObject* - a dict that describes the state application.
 */
static PyObject*
stateAppToDict(CStatusDb::StateApp& app)
{
    PyObject* result = PyDict_New();;
    
    dictStoreInt(result, "id", app.s_id);
    dictStoreString(result, "name", app.s_appName.c_str());
    dictStoreString(result, "host", app.s_appHost.c_str());
    
    return result;
}

/**
 * transitionVecToTuple
 *    Takes the vector of CStatusDb::StateTransition items and returns a
 *    tuple of dicts that contain the same data for python scripts.  See
 *    statusdb_queryStateTransitions for more information about the dicts
 *    that are in this tuple.
 *
 *  @param vec        - the vector of state transition information.
 *  @return PyObject* - the tuple of dicts created.
 *
 */
static PyObject*
transitionVecToTuple(std::vector<CStatusDb::StateTransition>& vec)
{
    PyObject* result = PyTuple_New(vec.size());
    
    for (int i = 0; i < vec.size(); i++) {
        PyObject* item = PyDict_New();
        
        dictStoreObj(item, "application", stateAppToDict(vec[i].s_app));
        dictStoreInt(item, "appid", vec[i].s_appId);
        dictStoreInt(item, "transitionId", vec[i].s_transitionId);
        dictStoreInt(item, "timestamp", vec[i].s_timestamp);
        dictStoreString(item, "leaving", vec[i].s_leaving.c_str());
        dictStoreString(item, "entering", vec[i].s_entering.c_str());
        
        PyTuple_SetItem(result, i, item);
    }
    
    return result;
}


/**
 * stateAppVecToTuple
 *   Takes a std::vector<CStatusDb::StateApp> and returns a tuple of dicts that
 *   describe the contents of the input vector.
 *
 * @param vec - input vector of state app structs
 * @return PyObject*  - Actually a tuple of dicts.
 */
static PyObject*
stateAppVecToTuple(std::vector<CStatusDb::StateApp>& vec)
{
    PyObject* result = PyTuple_New(vec.size());
    
    for (int i = 0; i < vec.size(); i++) {
        PyTuple_SetItem(result, i, stateAppToDict(vec[i]));
    }
    
    return result;
}

// Implementation of the statusdb type:

// canonical methods:

/**
 * statusdb_new
 *    Allocate storage for the statusdb type.
 *  @param type - pointer to the type struct.
 *  @param args - positional args passed to __new__
 *  @param kwargs - Keword args (dict) passed to __new__
 *  @return PyObject*  - pointer to unintialized object storage.
 */
static PyObject*
statusdb_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
    PyObject* self  = type->tp_alloc(type, 0);    // allocate the storage.
    if (!self) {
        PyErr_SetString(exception, "Failed to allocate a statusdb.statusdb object");
    } else {
        pStatusDb pThis = reinterpret_cast<pStatusDb>(self);
        pThis->m_pApi  = nullptr;           // Flag that we need to instantiate.
    }
    return self;
}

/**
 * statusdb_init
 *    Initializes the statusdb storage.  This means creating a CStatusDb
 *    object and storing it for later use.  This also implies that we
 *    require the following parameters:
 *
 * @param self - Actually a pStatusDb to our object storage.
 * @param args - positional parameters (see below).
 * @param kwargs - keword arguments (actually a dict).
 * @return Status of the initialization:  0 - success, -1 for failure.
 *
 * @note  The method requires one or two parameters.  The first parameter is always
 *        the database connection specification.  This is generally a file path
 *        but can also be :memory: for an in memory database.  If supplied,
 *        the second parameter is a boolean that is true if the database is
 *        writable and false if readonly.  The default is for the database to
 *        be writable.
 *
 * @note If the database file does not yet exist it will be created.
 */
static int
statusdb_init(PyObject* self, PyObject* args, PyObject* kwargs)
{
    const char*   connection;
    PyObject*     openType(Py_True);             // Default is for true flag.
    
    if (PyTuple_Size(args) == 1) {
        if(!PyArg_ParseTuple(args, "s", &connection)) {
            return -1;
        }
    } else {
        if (!PyArg_ParseTuple(args, "sO", &connection, &openType)) {
            return -1;
        }
    }
    // Figure out the open flags:
    
    int openFlags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    if (openType == Py_False) {
        openFlags = SQLITE_OPEN_READONLY;  // so another creator cancome later.
    }
    
    pStatusDb pThis = reinterpret_cast<pStatusDb>(self);
    
    // Ready to try the open with throws mapping to python exceptions:
    
    try {
        pThis->m_pApi = new CStatusDb(connection, openFlags);
        return 0;
    }
    catch (const char* msg) {
        PyErr_SetString(exception,  msg);
        
    }
    catch (std::string msg) {
        PyErr_SetString(exception, msg.c_str());
        
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
    }
    catch (...) {
        PyErr_SetString(exception, "Unanticipated c++ exception type caught");
    }
    // We can only get here on errors since the try block returns:
    
    return -1;
}
/**
 *  statusdb_delete
 *    Release storage assocated with the type instance.  This means free-ing
 *    the statusdb object.
 *
 *  @param self - pointer to the object's storage.
 */
static void
statusdb_delete(PyObject* self)
{
    pStatusDb pThis = reinterpret_cast<pStatusDb>(self);
    delete pThis->m_pApi;
}

// Action methods for statusdb type instances.

/**
 * addLogMessage
 *    Adds a log message to the database.
 *
 *  @param self - pointer tothe pStatusDb struct that is our instance data.
 *  @param args - positional arguments.  See below.
 *  @return Py_None  - Returns the None object.
 *
 * @note The method requires the following parameters:
 *    -   severity - The severity (e.g. statusmessages.SeverityLevels.INFO).
 *    -   app      - Application name string.
 *    -   src      - source of the message (FQDN string).
 *    -   time     - 64  bit time_t.
 *    -   message - Message text.
 */
static PyObject*
statusdb_addLogMessage(PyObject* self, PyObject* args)
{
    int           severity;
    const char*   app;
    const char*   src;
    std::uint64_t time;
    const char*   message;
    
    if (!PyArg_ParseTuple(args, "issLs", &severity, &app, &src, &time, &message)) {
        return NULL;
    }
    
    CStatusDb* pApi = getApi(self);
    try {
        pApi->addLogMessage(severity, app, src, time, message);
    }
    catch (const char* message) {
        PyErr_SetString(exception, message);
        return NULL;
    }
    catch (std::string message) {
        PyErr_SetString(exception, message.c_str());
        return NULL;
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    catch (...) {
        PyErr_SetString(exception, "Unanticipated C++ exception type caught");
        return NULL;
    }
    
    Py_RETURN_NONE;
}

/**
 * statusdb_addLogMessage
 *    Wrapper for CStatusDb::addRingStatistics.
 *    
 *  @param self  - Pointer to our instance storage.
 *  @param args  - positional parameters which are:
 *    - severity (from statusmessages.SeverityLevels).
 *    - app  name of application emitting the message (e.g. ringstatdaemon )
 *    - from fqdn of the host that emitted the message.
 *    - ringid - Dict that describes the ringbuffer.
 *    - clients - Possibly empty iterable of client/stats dicts (optional)
 *    
 *  @return None
 *  
 *  The format of the dicts is defined in PyStatusMessages. See:
 *    -  msg_encodeRingId for the format of the ringid dict.
 *    -  msg_encodeRingClient for the format of individual clients dicts.
 */
static PyObject*
statusdb_addRingStatistics(PyObject* self, PyObject* args)
{
    int severity;
    const char* app;
    const char* source;
    PyObject*   ringid;
    PyObject*   clients(NULL);
    
    // There may or may not be a clients iterable.
    
    if (PyTuple_Size(args) == 4) {
        if (!PyArg_ParseTuple(args, "issO", &severity, &app, &source, &ringid)) {
            return NULL;                // Exception already raised.
        }
    } else {
        if(!PyArg_ParseTuple(
            args, "issOO", &severity, &app, &source, &ringid, &clients
        )) {
            return NULL;
        }
    }
    
    // Transform the dicts into the appropriate objects for the API.
    
    CStatusDefinitions::RingStatIdentification* id(0);
    packRingId(&id, ringid);
    
    std::vector<const CStatusDefinitions::RingStatClient*> clientStructs;
    packRingClients(clientStructs, clients);
    if (PyErr_Occurred()) {
        return NULL;            // Some exception has been raised.
    }
    
    // The try /catch block below ensures that C++ exceptions get mapped to python raises:
    
    try {
        CStatusDb* pApi = getApi(self);
        pApi->addRingStatistics(severity, app, source, *id, clientStructs);
        freeRingResources(id, clientStructs);
    }
    catch (const char* msg) {
        PyErr_SetString(exception, msg);
        freeRingResources(id, clientStructs);
        return NULL;
    }
    catch (std::string msg) {
        PyErr_SetString(exception, msg.c_str());
        freeRingResources(id, clientStructs);
        return NULL;
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        freeRingResources(id, clientStructs);
        return NULL;
    }
    catch (...) {
        PyErr_SetString(exception, "Unanticipated C++ exception type caught");
        return NULL;
    }
    
    
    Py_RETURN_NONE;
}
/**
 * statusdb_addStateChange
 *    Wraps CStatusDB::addStateChange for python applications.
 *
 *  @param self - Pointer to our instance data.
 *  @param args - positional parameters.  These are:
 *     -  severity - from e.g. statusmessage.SeverityLevels
 *     -  app      - Name of the emitting application.
 *     -  src      - FQDN of the host that emitted the message.
 *     -  timestamp - time_t at which the transition was logged.
 *     -  from     - Prior state.
 *     -  to       - State transitioned to.
 * @return PyNone.
 */
static PyObject*
statusdb_addStateChange(PyObject* self, PyObject* args)
{
    int severity;
    std::uint64_t timestamp;
    char* app;
    char* src;
    char* from;
    char* to;
    
    if (!PyArg_ParseTuple(args, "isslss", &severity, &app, &src, &timestamp, &from, &to)) {
        return NULL;
    }
    
    // Use the try/catch block to map C++ exceptions into C++
    
    try {
        CStatusDb*  pApi = getApi(self);
        pApi->addStateChange(severity, app, src, timestamp, from, to);
    }
    catch (const char* msg) {
        PyErr_SetString(exception, msg);
        return NULL;
    }
    catch (std::string msg) {
        PyErr_SetString(exception, msg.c_str());
        return NULL;
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    catch (...) {
        PyErr_SetString(exception, "Unanticipated C++ exception type caught");
        return NULL;
    }
    Py_RETURN_NONE;
}

/**
 * statusdb_addReadoutStatistics
 *    Wraps CStatusDb::addReadoutStatistics - logs a readout statistics entry
 *    into the database.
 *
 *  @param self - pointer to our storage.
 *  @param args - positional parameters.  This consists of:
 *      -  severity - message severity (from statusmessages.SeverityLevels).
 *      -  app      - Application that's logged this record.
 *      -  src      - FQDN Of host app is running in.
 *      -  start    - time_t at which the record was created.
 *      -  runNumber - Number of the active run.
 *      -  title     - Title of the active run.
 *      -  counters  - optional dict with the statistics.   This dict must
 *                     have at least the keys from
 *                     PyStatusMessages::msg_decodeRunCounters.
 *  @return PyNone.
 */
static PyObject*
statusdb_addReadoutStatistics(PyObject* self, PyObject* args)
{
    int severity;
    char* app;
    char* src;
    std::int64_t startTime;
    int   runNumber;
    char* title;
    PyObject* pCounterDict(nullptr);
    CStatusDefinitions::ReadoutStatCounters* pCounters(nullptr);
    
    // Decoding depends on the length of the args tuple:
    
    if (PyTuple_Size(args) == 6) {
        if (!PyArg_ParseTuple(
            args, "isslis", &severity, &app, &src, &startTime, &runNumber, &title
        )) {
                return NULL;
            }
        } else {
            if (!PyArg_ParseTuple(
            args, "isslisO", &severity, &app, &src, &startTime, &runNumber, &title,
            &pCounterDict
        )) {
                return NULL;
            }
        
    }
    // Manage c++ exceptions with this try /catch that maps them to python:
    
    try {
        // If pCounterDict is not null, unpack it.
        
        CStatusDefinitions::ReadoutStatCounters counters;
        if (pCounterDict != nullptr) {
            unpackReadoutCounters(counters, pCounterDict);
            pCounters = &counters;
        }
        CStatusDb* m_pApi = getApi(self);
        m_pApi->addReadoutStatistics(
            severity, app, src, startTime, runNumber, title, pCounters
        );
    }
    catch (const char* msg) {
        PyErr_SetString(exception, msg);
        return NULL;
    }
    catch (std::string msg) {
        PyErr_SetString(exception, msg.c_str());
        return NULL;
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        return NULL;
    }
    catch (...) {
        PyErr_SetString(exception, "Unanticipated exception type caught");
        return NULL;
    }
    
    Py_RETURN_NONE;
}

/**
 * statusdb_queryLogMessages
 *   Wrapper for CStatusDb::queryLogMessages
 *
 *  @param self - Pointer to our object storage.
 *  @param args - positional parameters for the call.
 *     This is an optional object with a toString method that takes no parameters
 *     and returns a string that can be used to build a raw query filter.
 *     See the nscldaq.sqlite.where module e.g.
 *  @return PyObject*  A possibly empty tuple of dicts.  Each dict represents
 *     a log record.  The dict keys are:
 *       - 'id'   - Primary key of the record (uint).
 *       - 'timestamp'  - The time at which the log message was emitted (int).
 *       - 'message'    - The message itself (string).
 *       - 'severity'   - The log message severity (int)
 *       - 'application' - Application that emitted the log message.
 *       - 'source'      - FQDN of the source node.
 */
static PyObject*
statusdb_queryLogMessages(PyObject* self, PyObject* args)
{
    // Figure out which query filter to use in the  query.  Everything is in
    // a try/catch block to map c++ exceptions to python exceptions:
    
    CQueryFilter* userFilter(nullptr);
    try {
        // Figure out the filter we're going to use:
        
        CQueryFilter* filter = &DAQ::acceptAll;     // Default filter.
        PyObject*     filterObj;
        if (PyTuple_Size(args) > 0) {    
            if (!PyArg_ParseTuple(args, "O", &filterObj)) {
                return NULL;
            }
            userFilter = createFilterObject(filterObj);
            filter = userFilter;
        }
        // Do the query:
        
        CStatusDb* pApi = getApi(self);
        std::vector<CStatusDb::LogRecord> queryResults;
        pApi->queryLogMessages(queryResults, *filter);
        delete userFilter;                     // Done with any user filter.
        
        // Pythonize the result:
        
        PyObject* result = logRecordsToDictTuple(queryResults);
        return result;
    }
    catch(const char* message) {
        PyErr_SetString(exception, message);
        delete userFilter;
        return NULL;
    }
    catch (std::string message) {
        PyErr_SetString(exception, message.c_str());
        delete userFilter;
        return NULL;
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        delete userFilter;
        return NULL;
    }
    catch (...) {
        PyErr_SetString(exception, "Unanticipated C++ exception caught");
        delete userFilter;
        return NULL;

    }
    // Control should not pass here as the try block has a return too:
    
    PyErr_SetString(exception, "queryLogMessage bug detected in logic flow");
    return NULL;

}
/**
 * statusdb_listRings
 *   Lists the set of ringbuffers that are defined  in the database.
 *   This is returned as a tuple of dicts.  Each dict contains the following
 *   keys:
 *   - id   - primary key of the ringbuffer record in the database.
 *   - name - Ring buffer name (not URI).
 *   - host - Host in which the ring buffer is defined.
 *   - fqname - Fully qualified ring name.  This is just name@host
 *            Note that for proxy rings there will be two @ signs:
 *            name@sourcehost@proxyhost  where sourcehost is the host from which
 *            the data is being hoisted and proxyhost is the host in which the
 *            proxy ring lives.  For example, consider the ring
 *            tcp://spdaq19.nscl.msu.edu/aring where the proxy is located in
 *            u6pc2.nscl.msu.edu, this will have the fqname of
 *            aring@spdaq19.nscl.msu.edu@u6pc2.nscl.msu.edu
 *
 *   @param self - Pointer to our instance data.
 *   @param args - Positional parameters.  In this case there's a single optional
 *                 parameter that is query filter.   Query filters are objects
 *                 that have a method named 'toString' that returns a WHERE
 *                 clause for sql.
 *   @return PyObject*  See above for a definition of the object returned.
 */
static PyObject*
statusdb_listRings(PyObject* self, PyObject* args)
{
   // Figure out which query filter to use in the  query.  Everything is in
    // a try/catch block to map c++ exceptions to python exceptions:
    
    CQueryFilter* userFilter(nullptr);
    try {
        // Figure out the filter we're going to use:
        
        CQueryFilter* filter = &DAQ::acceptAll;     // Default filter.
        PyObject*     filterObj;
        if (PyTuple_Size(args) > 0) {    
            if (!PyArg_ParseTuple(args, "O", &filterObj)) {
                return NULL;
            }
            userFilter = createFilterObject(filterObj);
            filter = userFilter;
        }
        // Do the query and marshall the results:
        
        std::vector<CStatusDb::RingBuffer> rawResult;
        CStatusDb* pDb = getApi(self);
        pDb->listRings(rawResult, *filter);
        
        PyObject* result = ringListToTuple(rawResult);
        return result;
    }
    catch(const char* message) {
        PyErr_SetString(exception, message);
        delete userFilter;
        return NULL;
    }
    catch (std::string message) {
        PyErr_SetString(exception, message.c_str());
        delete userFilter;
        return NULL;
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        delete userFilter;
        return NULL;
    }
    catch (...) {
        PyErr_SetString(exception, "Unanticipated C++ exception caught");
        delete userFilter;
        return NULL;

    }
    // Control should not pass here as the try block has a return too:
    
    PyErr_SetString(exception, "queryLogMessage bug detected in logic flow");
    return NULL;
}

/**
 * statusdb_listRingsAndClients
 *   Lists the ringbuffers and their known clients.
 *
 * @param self - Points to our obect instance data.
 * @param args - Positional parameters.  This is an optional filter object.
 *               Filter objects are objects with a 'toString' method that
 *               returns the contents of an SQL WHERE clause.
 * @return PyObject*  - This is actually a dict.  The
 *               keys are fully qualified ring names the contents are a pair.
 *               The first element of the pair is the ring dict as described
 *               in statusdb_listRings.  The second is a tuple of
 *               dicts where each dict describes a client.  The client dict has
 *               the following keys:
 *               -  id    - Primary key of the record.
 *               -  pid   - PID of the client.
 *               -  producer - Boolean that is true if the client was a producer.
 *               - command  - The command used to start the client.
 */
static PyObject*
statusdb_listRingsAndClients(PyObject* self, PyObject* args)
{
   // Figure out which query filter to use in the  query.  Everything is in
    // a try/catch block to map c++ exceptions to python exceptions:
    
    CQueryFilter* userFilter(nullptr);
    try {
        // Figure out the filter we're going to use:
        
        CQueryFilter* filter = &DAQ::acceptAll;     // Default filter.
        PyObject*     filterObj;
        if (PyTuple_Size(args) > 0) {    
            if (!PyArg_ParseTuple(args, "O", &filterObj)) {
                return NULL;
            }
            userFilter = createFilterObject(filterObj);
            filter = userFilter;
        }
        // Do the query and marshall the results:
        
        CStatusDb::RingDirectory raw;
        CStatusDb* pApi = getApi(self);
        
        pApi->listRingsAndClients(raw, *filter);
        
        PyObject* result = ringDirectoryToMap(raw);
        return result;

    }
    catch(const char* message) {
        PyErr_SetString(exception, message);
        delete userFilter;
        return NULL;
    }
    catch (std::string message) {
        PyErr_SetString(exception, message.c_str());
        delete userFilter;
        return NULL;
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        delete userFilter;
        return NULL;
    }
    catch (...) {
        PyErr_SetString(exception, "Unanticipated C++ exception caught");
        delete userFilter;
        return NULL;

    }
    // Control should not pass here as the try block has a return too:
    
    PyErr_SetString(exception, "queryLogMessage bug detected in logic flow");
    return NULL;        
}
/**
 * statusdb_queryRingStatistics
 *
 *    Wraps CStatusDb::queryRingStatistics.
 *
 *  @param self - Pointer to our instance data.
 *  @param args - positional parameters.  This is an optional filter.
 *  @return PyObject* Actually a pointer to a dict that is indexed by the fully
 *               qualified name of each ring.  The contents of each key are
 *               the same as that of listRingsAndClients except that intead of having
 *               a vector of client dicts, there's a vector of pairs where the
 *               first element is a client dict and the second a tuple of
 *               statistics for that client.  Each statistic is a dict
 *               that contains:
 *               -  id  - Primary key of the statistic in the ring_statistics table.
 *               -  timestamp   - Time at which the entry was produced. (int(time.time()))
 *               -  operations  - Total number of operations the client has performed.
 *               -  bytes       - Total number of bytes the client has transferred
 *                                into or out of the ring.
 *               -  backlog     - Number of bytes of backlog in the ring at the time
 *                                the entry was made. 
 */
static PyObject*
statusdb_queryRingStatistics(PyObject* self, PyObject* args)
{
   // Figure out which query filter to use in the  query.  Everything is in
    // a try/catch block to map c++ exceptions to python exceptions:
    
    CQueryFilter* userFilter(nullptr);
    try {
        // Figure out the filter we're going to use:
        
        CQueryFilter* filter = &DAQ::acceptAll;     // Default filter.
        PyObject*     filterObj;
        if (PyTuple_Size(args) > 0) {    
            if (!PyArg_ParseTuple(args, "O", &filterObj)) {
                return NULL;
            }
            userFilter = createFilterObject(filterObj);
            filter = userFilter;
        }
        // Do the query and marshall the results:
        
        CStatusDb* pApi = getApi(self);
        CStatusDb::CompleteRingStatistics raw;
        
        pApi->queryRingStatistics(raw, *filter);
        
        PyObject* result = ringStatisticsToMap(raw);
        return result;

    }
    catch(const char* message) {
        PyErr_SetString(exception, message);
        delete userFilter;
        return NULL;
    }
    catch (std::string message) {
        PyErr_SetString(exception, message.c_str());
        delete userFilter;
        return NULL;
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        delete userFilter;
        return NULL;
    }
    catch (...) {
        PyErr_SetString(exception, "Unanticipated C++ exception caught");
        delete userFilter;
        return NULL;

    }
    // Control should not pass here as the try block has a return too:
    
    PyErr_SetString(exception, "queryLogMessage bug detected in logic flow");
    return NULL;        
    
}
/**
 * statusdb_listStateApplications
 *   Lists the set of applications that can emit state information.
 *
 *  @param self - pointer to the instance's member storage.
 *  @param args - Positional parameters.  This is an optional query filter.
 *  @return PyObject* - actually a tuple of dicts that contain the following keys:
 *                      - id   - Primary key of the application in the
 *                               state_application table.
 *                      - name - Name of the state application.
 *                      - host - Host the application runs in.
 */
static PyObject*
statusdb_listStateApplications(PyObject* self, PyObject* args)
{
   // Figure out which query filter to use in the  query.  Everything is in
    // a try/catch block to map c++ exceptions to python exceptions:
    
    CQueryFilter* userFilter(nullptr);
    try {
        // Figure out the filter we're going to use:
        
        CQueryFilter* filter = &DAQ::acceptAll;     // Default filter.
        PyObject*     filterObj;
        if (PyTuple_Size(args) > 0) {    
            if (!PyArg_ParseTuple(args, "O", &filterObj)) {
                return NULL;
            }
            userFilter = createFilterObject(filterObj);
            filter = userFilter;
        }
        // Do the query and marshall the results:
        
        CStatusDb* pApi = getApi(self);
        std::vector<CStatusDb::StateApp> raw;
        
        pApi->listStateApplications(raw, *filter);
        
        PyObject* result = stateAppVecToTuple(raw);
        return result;

    }
    catch(const char* message) {
        PyErr_SetString(exception, message);
        delete userFilter;
        return NULL;
    }
    catch (std::string message) {
        PyErr_SetString(exception, message.c_str());
        delete userFilter;
        return NULL;
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        delete userFilter;
        return NULL;
    }
    catch (...) {
        PyErr_SetString(exception, "Unanticipated C++ exception caught");
        delete userFilter;
        return NULL;

    }
    // Control should not pass here as the try block has a return too:
    
    PyErr_SetString(exception, "queryLogMessage bug detected in logic flow");
    return NULL;        
    
}
/**
 *   statusdb_queryStateTransitions
 *      Wrapper for CStatusDb::queryStateTransitions.  In keeping with the C++
 *      API, we return a tuple of dicts.  Each dict has the following keys:
 *      -  application - Contains the application dict for the app that generated
 *                       the transition.
 *      -  appid       - Contains the id of the application (should match the id
 *                       key of the application dict).
 *      - transitionid - Id of the transition in the state_transitions table.
 *      - timestamp    - the int(time.time()) at when the transition was issued.
 *      - leaving      - Name of the state that's being left.
 *      - entering     - name of the state that's being entered.
 *
 *  @param self - pointer to our instance storage.
 *  @param args - positional args that can contain an optional filter object.
 *  
 */
static PyObject*
statusdb_queryStateTransitions(PyObject* self, PyObject* args)
{
   // Figure out which query filter to use in the  query.  Everything is in
    // a try/catch block to map c++ exceptions to python exceptions:
    
    CQueryFilter* userFilter(nullptr);
    try {
        // Figure out the filter we're going to use:
        
        CQueryFilter* filter = &DAQ::acceptAll;     // Default filter.
        PyObject*     filterObj;
        if (PyTuple_Size(args) > 0) {    
            if (!PyArg_ParseTuple(args, "O", &filterObj)) {
                return NULL;
            }
            userFilter = createFilterObject(filterObj);
            filter = userFilter;
        }
        // Do the query and marshall the results:
        
        CStatusDb* pApi = getApi(self);
        std::vector<CStatusDb::StateTransition> raw;
        
        pApi->queryStateTransitions(raw, *filter);
        
        PyObject* result = transitionVecToTuple(raw);
        return result;

    }
    catch(const char* message) {
        PyErr_SetString(exception, message);
        delete userFilter;
        return NULL;
    }
    catch (std::string message) {
        PyErr_SetString(exception, message.c_str());
        delete userFilter;
        return NULL;
    }
    catch (std::exception& e) {
        PyErr_SetString(exception, e.what());
        delete userFilter;
        return NULL;
    }
    catch (...) {
        PyErr_SetString(exception, "Unanticipated C++ exception caught");
        delete userFilter;
        return NULL;

    }
    // Control should not pass here as the try block has a return too:
    
    PyErr_SetString(exception, "queryLogMessage bug detected in logic flow");
    return NULL;        
    
    
}

// Tables and data types for the statusdb type:

static PyMethodDef statusdbMethods[] = {
    {"addLogMessage", statusdb_addLogMessage, METH_VARARGS,
     "Add a log message to the database"
    },
    {"addRingStatistics", statusdb_addRingStatistics, METH_VARARGS,
      "Add ring buffer definitions, clients and statistics to the database"
    },
    {"addStateChange", statusdb_addStateChange, METH_VARARGS,
     "Add a state change to the database"
    },
    {"addReadoutStatistics", statusdb_addReadoutStatistics, METH_VARARGS,
     "Log a readout statistics entry into the database."
    },
    {"queryLogMessages", statusdb_queryLogMessages, METH_VARARGS,
      "Query the log messages."
    },
    {"listRings", statusdb_listRings, METH_VARARGS,
    "List the set of ringbuffers defined in the database"},
    {"listRingsAndClients", statusdb_listRingsAndClients, METH_VARARGS,
    "Lists the ringbuffers and all of their known clients"},
    {"queryRingStatistics", statusdb_queryRingStatistics, METH_VARARGS,
        "Query ring statistics from the database"
    },
    {
      "listStateApplications", statusdb_listStateApplications, METH_VARARGS,
      "List the emitters of state change records."
    },
    {
        "queryStateTransitions", statusdb_queryStateTransitions, METH_VARARGS,
        "Generic query of state transitions"
    },
    {NULL, NULL, 0, NULL}  
};

static PyTypeObject statusdb_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "statusdb.statusdb",       /*tp_name*/
    sizeof(StatusDb), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)(statusdb_delete), /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    "Encapsulation of StatusDb class.", /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    statusdbMethods,           /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)statusdb_init,      /* tp_init */
    0,                         /* tp_alloc */
    statusdb_new,                 /* tp_new */
};


// Module level initialization:

static PyMethodDef ModuleMethods[] = {
    {NULL, NULL, 0, NULL}                        // End of methods sentinel.
};

PyMODINIT_FUNC
initstatusdb(void)
{
    PyObject* module;
    
    // Initialize the module
    
    module = Py_InitModule3(
        "statusdb", ModuleMethods,
        "Python bindings to the status database."
    );
    if (module == nullptr) {
        return;                                // no way to signal our displeasure
    }
    
    // Initialize the module's exception.
    
    exception = PyErr_NewException(
        const_cast<char*>("statusdb.exception"), nullptr, nullptr
    );
    PyModule_AddObject(module, "exception", exception);
    
    // Initialize the statusdb type.
    
    if (PyType_Ready(&statusdb_Type) > 0) {
        return;
    }
    Py_INCREF(&statusdb_Type);
    PyModule_AddObject(
        module, "statusdb", reinterpret_cast<PyObject*>(&statusdb_Type)
    );
}