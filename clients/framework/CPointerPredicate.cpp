/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2008

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


#ifndef __CPOINTERPREDICATE_CPP
#define __CPOINTERPREDICATE_CPP
#include <config.h>
#include "CPointerPredicate.h"

// For sprintf:

#ifndef __CRT_STDIO_H
#include <stdio.h>
#ifndef __CRT_STDIO_H
#define __CRT_STDIO_H
#endif
#endif


#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

// Initialize static member m_nAutoIndex
template <class T> unsigned int CPointerPredicate<T>::m_nAutoIndex = 0;



/*!

    Automatically names an object given its base class(es)

  \param  rBaseName
     The base name of the object being named
 */
template<typename T>
string
CPointerPredicate<T>::GetAutoName(const string& rBaseName)
{
  char numberpart[100];
  sprintf(numberpart, "(%u)", m_nAutoIndex);
  m_nAutoIndex++;
  string Name(rBaseName);
  Name += numberpart;
  
  return Name;
}

#endif
