#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#             Ron Fox
#             Giordano Cerriza
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


##
# @file  ringutils.tcl
# @brief ringutils package provides ring buffer utility methods.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide ringutils 1.0
package require portAllocator



##
# getRingUsage
#
#  Get the rings used/known by a ringmaster.
#
# @param host - defaults to localhost Host for which to ask for this
#               information
#
# @return list - Returns the list from the LIST command to that ringmaster.
# @note factored out from eventlogBundle.
# @note Unlike ringbuffer usage in CRingCommand.cpp, this can contact a remote
#       ringmaster and therefore return information about the ring buffers in
#       other hosts.
#
proc getRingUsage {{host localhost}} {
    portAllocator create manager -hostname $host
    set ports [manager findServerAllUsers RingMaster]
    manager destroy

    if {[llength $ports] == 0} {
      error "No RingMaster server  on $host"
    }
    if {[llength $ports] !=1} {
      error "Multiple ring masters are running chaos!!"
    }
    
    set service [lindex $ports 0]
    set port    [lindex $service 1]

    
    set sock [socket $host $port]
    fconfigure $sock -buffering line
    puts $sock LIST
    gets $sock OK
    if {$OK ne "OK"} {
        error "Expected OK from Ring master but got $OK"
    }
    gets $sock info
    close $sock
    return $info
}

##
# ringExists
#   True if a specific ring buffer exists on 
#   a host:
#
# @param name - name (not URL) of ringbuffer.
# @param host - Optional host name  on which to check
#               defaults to localhost
# @return bool - true if the ring exists.
# @throw error if the host name does not exist or is not running
#   a ring master.
proc ringExists {name {host localhost}} {
  set fullUsage [getRingUsage $host]

  #  See if there's a ring that matches:

  set index [lsearch -exact -index 0 $fullUsage $name]
  return [expr {$index != -1}]
}

## 
# waitForRing
#   Wait for a ringbuffer to be created.  
#  
#  @param name - name (not url) of the ring.
#  @param timeout - number of seconds to wait.
#  @param interval - Number of milliseconds between checks.
#  @param host  - Optional host name the ring lives in defaults to localhost.
#  @return bool - indicating the wait succeeded.
#  @throws an error if the host does not exist or is not running a functional
#    ring amster.
# 
proc waitForRing {name timeout interval {host localhost}} {
  # Compute the number of polls we'll perform:
  
  set timeoutms [expr {$timeout*1000}]
  if {$interval > 0 && $interval < $timeoutms} {
    set polls [expr {$timeoutms/$interval}]
  } else {
    set polls 1;    # interval > timeoutms or is idiotic.
    set timeout 1;  # Make sure it's no longer itdiotic, if it was.
  }
  
  while {$polls > 0} {
    incr polls -1;        # So we don't wait on the last poll:

    if {[ringExists $name $host]} {
      return 1;           # Got it.
    }
    if {$polls > 0} {
      after $interval;   # Only wait if not last pass.    
    }
    
  }
  return 0
}