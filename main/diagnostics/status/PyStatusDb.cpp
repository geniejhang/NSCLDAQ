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

static PyObject* exception(0);

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
}