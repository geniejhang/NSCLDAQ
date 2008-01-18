/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

// Author:
//    Ron Fox
//    NSCL
//    Michigan State University
//    East Lansing, MI 48824-1321
//    mailto:fox@nscl.msu.edu
//
#ifndef __CBINDING_H
#define __CBINDING_H

#ifndef __CTYPEFREEBINDING_H
#include <CTypeFreeBinding.h>
#endif

#ifndef __TCLINTERPRETER_H
#include <TCLInterpreter.h>
#endif

#ifndef __RANGEERROR_H
#include <RangeError.h>
#endif

#ifndef __TYPEINFO
#include <typeinfo>
#define __TYPEINFO
#endif

#ifndef __STL_STRING
#include <string>
#define __STL_STRING
#endif

#ifndef __STL_STRING_H
#include <string.h>
#define __STL_STRING_H
#endif

#ifndef __CRT_STDIO_H
#include <stdio.h>
#define __CRT_STDIO_H
#endif

/*!
  This is an abstract base class for the Tcl configuration manager's bindings
  subsystem.  interfaces for the functions required of all bindings
  objects are defined as pure virtual member functions.
  */
template <class T>
class CBinding : public CTypeFreeBinding
{
public:
  /*!
     This function will be called just prior to reading in
     a configuration file.  The Tcl Interpreter has been set up and
     initialized.  The Init function can do any preparation required
     by the binding prior to readin (e.g. a simple binding >might<
     bind the contained variable to a Tcl variable
     \param rInterp - The interpreter on which the config script will be read.
     */
  virtual void InitBindings(CTCLInterpreter& rInterp)    = 0;
  /*!
    This function is called just after a configuration script or set of 
    configuration scripts have been read to perform any actions required to
    commit the read in Tcl values to the variables.  For example, an
    associative array bindings might need to fetch the individual values
    from Tcl array elements.
    \param rInterp - The interpreter in which the config script was read.
    */
  virtual void Commit(CTCLInterpreter& rInterp)  = 0;
  /*!
    This function is called just prior to deleting the interpreter.  Any
    cleanup actions required by the binding should be done at this point.
    For example, if Init mapped a C variable to a TCL variable, that mapping
    should be broken.
    \param rInterp - The interpreter about to be deleted.
  */
  virtual void ShutdownBindings(CTCLInterpreter& rInterp)= 0;
  /*!
    This function is called to write the set of Tcl commands required to
    duplicate the current state.  Note that this may not be identical to
    the set of commands which produced the configuration.
    \param fd (int [in]) The file descriptor on which the dump will be done.
    */
  virtual void Dump(int fd) = 0;

  /*!
     Returns the TCL code for the type of variable being bound to:
     This can be one of:
     - TCL_LINK_INT    -  Variable is an integer.
     - TCL_LINK_DOUBLE -  Variable is a double precision.
     - TCL_LINK_BOOLEAN   -  Variable is a boolean.
     - TCL_LINK_STRING -  Variable is a char*.
     
     \param item - A variable of type T.

     \throws CRangeError - no neat match.

     \bug Really should invent a bad type exception and throw it instead
   
  */
  int VariableType(T item) {
    if(typeid(item) == typeid(int))    return TCL_LINK_INT;
    if(typeid(item) == typeid(double)) return TCL_LINK_DOUBLE;
    if(typeid(item) == typeid(bool))   return TCL_LINK_BOOLEAN;
    if(typeid(item) == typeid(char*))  return TCL_LINK_STRING;
   
    throw CRangeError::CRangeError(TCL_LINK_INT, TCL_LINK_STRING,
		      0, "Attempting to convert data type to TCL Link type");
  }


  /*!
    Item to string conversion: Converts an item of type T to 
    its string representation.  
    \param Item - item to convert.
    \throws CRangeError - no type match
    \bug    Should invent a bad type exception and throw it.
    */
  STD(string) ItemToString(T Item)
  {
    char item[20];

    if(typeid(Item) == typeid(int)) {
      sprintf(item, "%d", Item);
      return STD(string)(item);
    }
    if(typeid(Item) == typeid(double) ) {
      sprintf(item, "%lf", Item);
      return STD(string)(item);
    }
    if(typeid(Item) == typeid(bool)) {
      return STD(string)(item ? "true" : "false");
    }
    if(typeid(Item) == typeid(char*)) {
	union {
	  T t;
	  char* p;
	} Values;
	Values.t = Item;
	if(Values.p == NULL) 
	  return STD(string)("");
	else {
	  Values.t = Item;
	  return STD(string)(Values.p);
	}
    }
    throw CRangeError(TCL_LINK_INT, TCL_LINK_STRING,
		      0, "Attempting to convert data to string form");
  }
};
#endif
