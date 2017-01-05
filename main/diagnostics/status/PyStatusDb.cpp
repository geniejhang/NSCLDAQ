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
        if(o) Py_DECREF(o);
        if(i) Py_DECREF(i);
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