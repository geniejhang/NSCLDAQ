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

set here [file dirname [info script]]
set libdir [file normalize [file join $here .. TclLibs]]
lappend auto_path $libdir

##
# @file readoutstatviewer.tcl
# @brief Status display application for readout statistics.
# @author Ron Fox <fox@nscl.msu.edu>
#
package require ReadoutStatModel
package require ReadoutStatView
package require ReadoutStatController
package require statusMessage


if {[llength $argv] != 1} {
    puts stderr "Usage:"
    puts stderr "    readoutstatsviewer dbfile"
    puts stderr "Where:"
    puts stderr "    dbfile - is the database file with the statistics information"
    exit -1
}


# Glue together the MVC and then we're done.

set db [statusdb create $argv readonly]

set model [ReadoutStatModel %AUTO% -dbcommand $db]
set view  [ReadoutStatView .v]
set controller [ReadoutStatController %AUTO% -model $model -view $view]

pack $view -fill both -expand 1
