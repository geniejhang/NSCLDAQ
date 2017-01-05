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
#include <sqlite3.h>

#include <string>
#include <stdexcept>
#include <cstdint>

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
    
    Py_RETURN_NONE;
}

// Tables and data types for the statusdb type:

static PyMethodDef statusdbMethods[] = {
    {"addLogMessage", statusdb_addLogMessage, METH_VARARGS,
     "Add a log message to the database"
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