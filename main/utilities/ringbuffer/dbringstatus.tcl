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
#             Jeromy Tompkins 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321



##
# @file dbringstatus.tcl
# @brief Get ring statuses for rings in the experiment definition database.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide DbRingStatus 1.0
package require vardbringbuffer
package require ring


##
# All procs will be in the ::DbRingStatus namespace.

namespace eval ::DbRingStatus {
    namespace export ringsByHost ringStatistics
}
#------------------ Private utilities -----------------------------------------

##
# ::DbRingStatus::filterRings
#   Given output from ringbuffer usage e.g. and a list of ring definition dicts,
#   returns the usage data only for the rings in the list of ring definitions.
#
# @param usage - Ring usage list.
# @param desired - list of dicts describing desired rings.
# @return list - usage filtered by the desired rings.
#
proc ::DbRingStatus::filterRings {usage desired} {
    
    #   First make a list of the desired rings:
    
    set ringList [list]
    foreach ring $desired {
        lappend ringList [dict get $ring name]
    }
    #  Done if there are no rings in the desired list:
    
    if {[llength $ringList] == 0} {
        return [list]
    }
    
    #  Analyze the usage list:
    
    set result [list]
    foreach ring $usage {
        set name [lindex $ring 0]
        if {$name in $ringList} {
            lappend result $ring
        }
    }
    return $result
}


#------------------ Public entries to the namespace ---------------------------

##
# ::DbRingStatus::ringsByhost
#   Determines the set of  ringbuffers that are defined in an experiment
#   database and returns them by host.  The result is a single dict
#   with the keys as the host names and the values as a list of
#   dicts that might come from ringInfo.
#
# @param rbobject - a ::nscldaq::vardbringbuffer object connected to the
#                   database.
# @return dict.  Keys are hosts, values are lists of ring definitions
#                in that host.
proc ::DbRingStatus::ringsByHost {rbobject} {
    set result [dict create]
    set raw    [$rbobject list]
    
    foreach ring $raw {
        set host [dict get $ring host]
        dict lappend result $host $ring
    }
    return $result
}
##
# ::DbRingStatus::ringStatistics
#   Given a dict of the form produced by ringsByHost, produces a dict
#   whose members are the statistics for ringbuffers we care about within that host.
#
# @param ringDict - rings we care about.
# @return dict - keys are host, values are statistics for rings we care about
#                in that host.
#
proc DbRingStatus::ringStatistics {ringDict} {
    set result [dict create]
    dict for {host rings} $ringDict {
        set usageData [ringbuffer usageAt $host]
        dict set result $host [::DbRingStatus::filterRings $usageData $rings]
    }
    return $result
}