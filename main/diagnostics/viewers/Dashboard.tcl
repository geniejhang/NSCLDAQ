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

package require LogView
package require LogModel
package require LogController
#------------------------------------------------------------------------------
#
#   Utility procedures
#


##
# getProperty
#   @param proplist - a property list.
#   @param name     - Name of the desired property.
#   @return string  - Value of the property.  Throws an error if that property
#                     does not exist.
#
proc getProperty {proplist name} {
    set prop [$proplist find $name]
    if {$prop ne ""} {
        return [$prop cget -value]
    }
    error "No such property $name"
}
##
# showSummaryWindow
#    Pops up a backlog summary window for the ring that was just clicked on.
#    The backlog summary window shows the set of programs that have
#    exceeded the backlog threshold.
#
# @param highlighter - The RingStatusHiglight object that called us.
# @param ringobj     - The dashboard object of the ring clicked.
# @param lastid      - Id of the last statistics entry from the database.
#
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
##
# setMenu
#   Set the appropriate top level menu for the tab that just got selected
#   by the user.
#
# @param top      - Top level widget.
# @param notebook - Widget of notebook.
# @param menus    - An list that can be used in an array set command.
#                   indices are notebook windows and values are menus.
# @note - if there is not a menu for the window, the top level will have no
#          menu.
#
proc setMenu {top notebook menus} {
    
    array set notebookMenus $menus
    
    # Figure out which window just became current:
    
    set index [$notebook index current]
    set nbWindow [lindex [$notebook tabs] $index]
    
    #  If there's a menu configure it into top otherwise no menu:
    
    if {[array names notebookMenus $nbWindow] ne ""} {
        $top configure -menu $notebookMenus($nbWindow)
    } else {
        $top configure -menu {}
    }
}
#------------------------------------------------------------------------------
#  Script entry.
#


if {[llength $argv] != 2} {
    puts stderr "Usage:"
    puts stderr "   Dashboard expdbfile statusdbfile"
    puts stderr "Where:"
    puts stderr "   expdbfile is the path to the experiment database description file."
    puts stderr "   statusdbfile is the path to the database getting status info."
    exit -1
}
# Make a notebook to contain everything:

ttk::notebook .n

#  Add a canvas with the dataflow dashboard.

canvas .n.c
pack .n -fill both -expand 1

.n add .n.c -sticky nsew -text {Data Flow}


set dburl file://[file normalize [lindex $argv 0]]
set statusDb [lindex $argv 1]

set objects [::DashboardDatabase::load $dburl .n.c]

#  Go over all the object bounding boxes and resize the canvas so that they all
#  fit:

set xmax 0
set ymax 0

foreach obType $objects {
    foreach item $obType {
        set id [$item cget -id]
        set bbox [.n.c bbox $id]
        set lrx [lindex $bbox 2]
        set lry [lindex $bbox 3]
        
        if {$lrx > $xmax} {set xmax $lrx}
        if {$lry > $ymax} {set ymax $lry}
        
    }
}

#  For now allow some fixed size slop for the titles.

incr xmax 100
incr ymax 100

.n.c configure -width $xmax -height $ymax

set startTime [clock seconds];  # We don't want any data older than _now_.


set api [statusdb create $statusDb readonly]

set highlighter [RingStatusHighlight %AUTO% \
    -canvas .n.c -rings [lindex $objects 0] \
    -dbapi $api  -command [list showSummaryWindow %W %R %L]]
    

#  Add a tab with the log messages.

LogView .n.l -colors [dict create \
    SEVERE red DEFECT red WARNING orange INFO green DEBUG violet    \
]
set logModel [LogModel %AUTO% -file $statusDb]
set logController [LogController %AUTO% -model $logModel -view .n.l]
.n add .n.l -text {LogMessages}
menu .logmenu
$logController fillFilterMenu .logmenu

bind .n <<NotebookTabChanged>> [list setMenu . .n [list .n.l .logmenu]]
