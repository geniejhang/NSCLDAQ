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


static const char* Copyright= "(C) Copyright Michigan State University 2002, All rights reserved";

#include <config.h>
#include "CNoSuchLinkException.h"
#include <string.h>
#include <stdio.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


const char*
CNoSuchLinkException::ReasonText() const
{
  return m_sReasonText.c_str();
}

void
CNoSuchLinkException::UpdateReasonText()
{
  m_sReasonText = "No such link\n";
  if(m_fName) {
    m_sReasonText += "Key was: ";
    m_sReasonText += m_sName;
  }
  else {
    m_sReasonText += "Id was: ";
    char szBuffer[20];
    sprintf(szBuffer, "%d\n", m_nId);
    m_sReasonText += szBuffer;
  }
}
