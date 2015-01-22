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

/*!
  ObjectRegistry.h:
  
  This file defines the CObjectRegistry class.
  
  Author:
    Jason Venema
    NSCL
    Michigan State University
    East Lansing, MI 48824-1321
    mailto:venemaja@msu.edu
*/

#ifndef __COBJECTREGISTRY_H
#define __COBJECTREGISTRY_H

#ifndef __CNAMEDOBJECT_H
#include "CNamedObject.h"
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_MAP
#include <map>
#ifndef __STL_MAP
#define __STL_MAP
#endif
#endif

typedef STD(map)<STD(string), CNamedObject*>::iterator ObjectIterator;
typedef STD(map)<STD(string), CNamedObject*>::const_iterator constObjectIterator;

/*!
  Implements a registry of named objects.  
  Registries allow you to determine which instances of particular
  types of objects exist.  Typically a programmer wanting to 
  allow this level of introspection will subclass a class hierarchy
  from CRegisteredObject such that the constructor of each
  class will register that class.  One can then programmatically
  search for named instances of a class as well as iterate through
  the registry to determine which instances exist.
*/

class CObjectRegistry : public CNamedObject
{
  STD(map)<STD(string), CNamedObject*> m_Objects; /*! STD(Map) containing the name key and a
                                           pointer to the object.
					 */
 public:

  // Default constructor
  CObjectRegistry (STD(string) am_sName) :
    CNamedObject(am_sName)
    { AppendClassInfo(); }

  // Destructor
  virtual ~CObjectRegistry () { }

  // Selectors
 public:

  STD(map)<STD(string), CNamedObject*> getObjects() const
    {
      return m_Objects;
    }

  // Mutators
 protected:

  void setObjects(STD(map)<STD(string), CNamedObject*> am_Objects)
    {
      m_Objects = am_Objects;
    }

  // Public member functions
 public:

  void Add(CNamedObject& rObject);
  void Remove(const STD(string)& rName);
  void Remove(const CNamedObject& rObject);
  constObjectIterator Find(const STD(string)& rObjectName) const;
  ObjectIterator begin();
  ObjectIterator end();
  virtual STD(string) DescribeSelf();
};

#endif
