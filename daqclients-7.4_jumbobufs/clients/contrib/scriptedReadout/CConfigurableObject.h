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


// Dirty to have a private copy but needed to add mechanisms to
// insert arbitrary config params.


/*!

Base class of all objects that have a TCL configurable
 configuration. The configuration object autonomously processes the
config an cget subcommands to maintain a configuration parameter 
database.  Configuration consists of a set of configuration parameter 
objects.

Each of these represents a keyword/value pair. 

*/

// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//
// Copyright 

#ifndef __CCONFIGURABLEOBJECT_H  //Required for current class
#define __CCONFIGURABLEOBJECT_H

//
// Include files:
//

#ifndef __CCONFIGURATIONPARAMETER_H
#include <CConfigurationParameter.h>
#endif

#ifndef __TCLPROCESSOR_H
#include <TCLProcessor.h>
#endif

#ifndef __TCLRESULT_H
#include <TCLResult.h>        //Required for include files  
#endif


#ifndef _STL_LIST
#include <list>
#define _STL_LIST
#endif

#ifndef _STL_STRING
#include <string>
#define _STL_STRING
#endif


// forward definitions. 


class CTCLInterpreter;
class CTCLResult;


class CConfigurableObject : public  CTCLProcessor     
{
  // Public data types.
public:
  typedef list<CConfigurationParameter*> ConfigArray;
  typedef ConfigArray::iterator          ParameterIterator;
private:
  
  string          m_sName;	//!< Name of the command associated with the object.
  ConfigArray     m_Configuration; //!< The configuration.


  // Constructors and other canonical operations.
public:
  CConfigurableObject (const string& rName,
		       CTCLInterpreter& rInterp);
  virtual  ~ CConfigurableObject ( );  

  // The copy like operations are not supported on tcl command processing
  // objects:
private:
  CConfigurableObject (const CConfigurableObject& aCConfigurableObject );
  CConfigurableObject& operator= (const CConfigurableObject& aCConfigurableObject);
  int operator== (const CConfigurableObject& aCConfigurableObject) const;
public:

  // Selectors:

  //!  Retrieve a copy of the name:

  string getName() const
  { 
    return m_sName;
  }   



  // Member functions:

public:

  virtual  int      operator() (CTCLInterpreter& rInterp, 
				CTCLResult& rResult, 
				int nArgs, char** pArgs)   ; //!< Process commands.
  virtual  int      Configure (CTCLInterpreter& rInterp, 
			       CTCLResult& rResult, 
			       int nArgs, char** pArgs)   ; //!< config subcommand 
  virtual  int      ListConfiguration (CTCLInterpreter& rInterp, 
				       CTCLResult& rResult, 
				       int nArgs, char** pArgs); //!< list subcommand 
  ParameterIterator AddConfigParam(CConfigurationParameter* param);

  ParameterIterator AddIntParam (const string& sParamName, 
				 int nDefault=0)   ; //!< Create an int.
  ParameterIterator AddBoolParam (const string& rName,
				  bool          fDefault)   ; //!< Create a boolean. 
  ParameterIterator AddStringParam (const string& rName)   ; //!< Create string param. 
  ParameterIterator AddIntArrayParam (const string&  rParameterName, 
				      int nArraySize, 
				      int nDefault=0)   ; //!< Create array of ints.
  ParameterIterator AddStringArrayParam (const string& rName, 
					 int nArraySize)   ; //!< Create array of strings.
  ParameterIterator Find (const string& rKeyword)   ; //!< Find a param 
  ParameterIterator begin ()   ; //!< Config param start iterator.
  ParameterIterator end ()   ;   //!< Config param end iterator.
  int size ()   ;                //!< Config param number of items.
  string ListParameters (const string& rPattern)   ; //!< List configuration 
  string ListKeywords ()   ;     //!< List keyword/type pairs.

protected:
  string Usage();
private:
  void              DeleteParameters ()   ; //!< Delete all parameters. 
  
};

#endif
