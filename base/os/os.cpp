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
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>

static const unsigned NSEC_PER_SEC(1000000000); // nanoseconds/second.


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
/**
 * Os::authenticateUser:  Authenticate a user given a username and password:
 *
 * @param sUser - the username.
 * @param sPassword - the cleartext passwordl
 *
 * @return - true if the username/password authenticates in the underlying os:
 *
 */
bool
Os::authenticateUser(std::string sUser, std::string sPassword)
{
  struct passwd Entry;
  struct passwd* pEntry;
  char   dataStorage[1024];

  if (getpwnam_r(sUser.c_str(), &Entry, dataStorage, sizeof(dataStorage), &pEntry)) {
    int errorCode = errno;
    std::string errorMsg = "Call to getpwnam_r failed at os level: ";
    errorMsg += strerror(errorCode);
    throw errorMsg;
  }
  if(!pEntry) return false;	// No such user.
  std::string EncryptedPassword(pEntry->pw_passwd);
  std::string EncryptedEntry(crypt(sPassword.c_str(), EncryptedPassword.c_str()));
  
  return EncryptedPassword == EncryptedEntry;
}
/**
 * Os::usleep
 *
 *    Wrapper for nanosleep since usleep is deprecated in POSIX
 *    but nanosleep is consider good.
 *
 * @param usec - Number of microseconds to sleep.
 * @return int - Status (0 on success, -1 on error)
 * 
 * @note No attempt is made to map errnos from usleep -> nanosleep...so you'll 
 *       get then nanosleep errnos directly.
 * @note We assume useconds_t is an unsigned int like type.
 */
int
Os::usleep(useconds_t usec)
{
  // usec must be converted to nanoseconds and then busted into
  // seconds and remaning nanoseconds
  // we're going to assume there's no overflow from this:
  
  useconds_t nsec = usec* 1000;			// 1000 ns in a microsecond.

  // Construct the nanosleep specification:

  struct timespec delay;
  delay.tv_sec  = nsec/NSEC_PER_SEC;
  delay.tv_nsec = nsec % NSEC_PER_SEC;


  struct timespec remaining;
  int stat;

  // Usleep is interrupted with no clue left about the remainnig time:

  return nanosleep(&delay, &remaining);

 
}
/**
 * Os::blockSignal
 *   Blocks the specified signal.
 *
 * @param sigNum - Number of the signal to block.
 *
 * @return value from sigaction
 */
int
Os::blockSignal(int sigNum)
{

  // Build the sigaction struct:

  struct sigaction action;
  action.sa_handler = 0;		// No signal handler.
  sigemptyset(&action.sa_mask);
  sigaddset(&action.sa_mask, sigNum);
  action.sa_flags = 0 ;

  struct sigaction oldAction;

  return sigaction(sigNum, &action, &oldAction);



  
}
