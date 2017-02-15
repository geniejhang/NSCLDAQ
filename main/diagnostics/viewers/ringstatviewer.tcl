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
# @file ringstatviewer.tcl
# @brief Pulls together the MVC for a standalone ring status display.
# @author Ron Fox <fox@nscl.msu.edu>
#

package require Tk

# Ensure the package search path has the DAQ tcl package dir.
# we assume we're one level below DAQROOT (e.g. in DAQBIN).

set here [file dirname [info script]]
set libdir [file normalize [file join $here .. TclLibs]]
lappend auto_path $libdir

package require RingStatModel
package require RingStatController
package require RingStatView
package require statusMessage

##
# We need a database file:
#

if {[llength $argv] != 1} {
    set dbfile [file normalize [file join ~ status.db]]
} else {
    set dbfile [lindex $argv 0]
}

puts "Using database $dbfile"

set db [statusdb create $dbfile readonly]

#  Instantiate the model, view and lastly the controller:


set m [RingStatModel %AUTO% -dbcommand $db]
set v [RingStatView .v]
set c [RingStatController %AUTO% -model $m -view $v]

#  Set up the layout:

pack $v -fill both -expand 1

