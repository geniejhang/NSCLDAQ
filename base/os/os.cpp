/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#include "os.h"
#include <pwd.h>
#include <errno.h>
#include <string.h>
/**
 * Get name of current user.
 * @return std::string
 */
std::string
Os::whoami()
{
  struct passwd  Entry;
  struct passwd* pEntry;
  char   dataStorage[1024];	// Storage used by getpwuid_r(3).
  uid_t  uid = getuid();

  if (getpwuid_r(uid, &Entry, dataStorage, sizeof(dataStorage), &pEntry)) {
    int errorCode = errno;
    std::string errorMessage = 
      "Unable to determine the current username in CTheApplication::destinationRing: ";
    errorMessage += strerror(errorCode);
    throw errorMessage;
    
  }
  return std::string(Entry.pw_name);
}
