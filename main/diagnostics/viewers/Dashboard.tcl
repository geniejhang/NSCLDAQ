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
# @file Dashboard.tcl
# @brief Top level of the dashboard status program.
# @author Ron Fox <fox@nscl.msu.edu>
#
package require Tk
lappend auto_path [file normalize [file join [file dirname [info script]] .. TclLibs]]
package require Dashboard
package require RingStatusHighlight
package require statusMessage
package require BacklogSummary

# Load the canvas with the experiment.

if {[llength $argv] != 2} {
    puts stderr "Usage:"
    puts stderr "   Dashboard expdbfile statusdbfile"
    puts stderr "Where:"
    puts stderr "   expdbfile is the path to the experiment database description file."
    puts stderr "   statusdbfile is the path to the database getting status info."
    exit -1
}

canvas .c
pack   .c -fill both -expand 1

set dburl file://[file normalize [lindex $argv 0]]
set statusDb [lindex $argv 1]

set objects [::DashboardDatabase::load $dburl .c]

#  Go over all the object bounding boxes and resize the canvas so that they all
#  fit:

set xmax 0
set ymax 0

foreach obType $objects {
    foreach item $obType {
        set id [$item cget -id]
        set bbox [.c bbox $id]
        set lrx [lindex $bbox 2]
        set lry [lindex $bbox 3]
        
        if {$lrx > $xmax} {set xmax $lrx}
        if {$lry > $ymax} {set ymax $lry}
        
    }
}

#  For now allow some fixed size slop for the titles.

incr xmax 100
incr ymax 100

.c configure -width $xmax -height $ymax

set startTime [clock seconds];  # We don't want any data older than _now_.


set api [statusdb create $statusDb readonly]

set highlighter [RingStatusHighlight %AUTO% \
    -canvas .c -rings [lindex $objects 0] \
    -dbapi $api  -command [list showSummaryWindow %W %R %L]]
    

proc getProperty {proplist name} {
    set prop [$proplist find $name]
    if {$prop ne ""} {
        return [$prop cget -value]
    }
    error "No such property $name"
}

proc showSummaryWindow {highlighter ringobj lastid} {
    
    # Construct the toplevel widget from the ring FQN with . -> _
    # If that window does not yet exist create it and title it with the fqn:
    
    set fqn [$ringobj cget -title]
    set widget .[string map ". _" $fqn]
    set existingWindows [winfo children .]
    
    if {$widget in $existingWindows } {
        return
    }
    # Figure out the ring and host names:
    
    set ringObject [$ringobj cget -data]
    set ringProps [$ringObject getProperties]
    set rname [getProperty $ringProps name]
    set rhost [getProperty $ringProps host]
    
    
    
    # Create and display the widget:
    
    toplevel $widget
    wm title $widget $fqn
    BacklogSummary $widget.summary   \
        -dbcommand [$highlighter cget -dbapi] -ringname $rname \
        -ringhost $rhost -threshold [$highlighter cget -threshold]
    pack $widget.summary -fill both -expand 1
    
}
