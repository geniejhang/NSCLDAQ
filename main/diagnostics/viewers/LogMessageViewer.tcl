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
# @file logtest.tcl
# @brief Log message displayer.
# @author Ron Fox <fox@nscl.msu.edu>
#

##
# Displays filtered log messages.
# We create a log display object
# We start multinode aggregation as an internal thread.
# We subscribe to log messages.
# We attach a script to log message receipt that decodes the message
# and dispatches it to the log display object.

set here [file dirname [info script]]
lappend auto_path [file normalize [file join $here .. TclLibs]]


package require LogView
package require LogModel
package require statusMessage
package require dialogWrapper

set dbfile ~/status.db

if {[llength $argv] > 0} {
    set dbfile [lindex $argv 0]
}
set dbfile [file normalize $dbfile]
puts "Using database $dbfile"

set severityList [list DEBUG INFO WARNING SEVERE DEFECT]
set filter [dict create]

##
# populateView
#   Populate the view with the messages more recent than the ones we already had.
#
# @param dt     - milliseconds to the next update.
# @param lastid - id of the last message gotten from the database.
#
proc populateView {dt lastid} {
    set idFilter [list id > $lastid]
    set filter   [list $idFilter]
    
    if {[dict exists $::filter severities]} {
        lappend filter [dict get $::filter severities]
    }
    
    foreach field [list application source] {
        if {[dict exist $::filter $field]} {
            lappend filter [dict get $::filter $field]
        }
    }
    
    
    set newMessages [model get $filter]
    .logview load $newMessages
    
    # Update the last id seen:
    
    if {[llength $newMessages] > 0} {
        set lastid [dict get [lindex $newMessages end] id]
    }
    
    
    set ::updateId [after $dt [list populateView $dt $lastid]]
}
##
# updateFilter
#    -  Update the severities key of the filter dict.
#    -  Cancel updates
#    -  Restart updates with the new filter (and id = -1 so that we rescan
#       the database).
#
proc updateFilter {} {
    after cancel $::updateId;            # cancel updates.
    .logview clear
    set severities [list]
    foreach sev $::severityList {
        set value [set ::$sev]
        if {$value} {
            lappend severities '$sev'
        }
    }
    set sevList "([join $severities ,])"
    dict set ::filter severities [list severity IN $sevList]   
    
    populateView 1000 -1
}

##
# setTextFilter
#
#  prompts for and adds a filter for a text field.
#
# @param which - field to filter on.
#
proc setTextFilter which {
    #
    #  Create a dialog that prompts for the field value:
    #
    toplevel .prompt
    set d [DialogWrapper .prompt.dialog]
    set carea [$d controlarea]
    set f [ttk::frame $carea.frame]
    ttk::label $f.label -text [string totitle "$which "]
    set value [ttk::entry $f.entry]
    grid $f.label $value
    $d configure -form $f
    pack $d
    
    set result [$d modal]
    
    if {$result eq "Ok"} {
        set requested [$value get]
        destroy .prompt;              # Don't need this any more.
        
        #  Note that an empty string removes the filter:
        
        if {$requested eq ""} {
            dict unset ::filter $which
        } else {
            dict set ::filter $which [list $which = '$requested']
        }
        updateFilter
    }
}
##
# clearFilters
#
#    Clears all filters.
#
proc clearFilters {} {
    foreach sev $::severityList {
        set ::$sev 1
    }
    set ::filter [dict create]
    updateFilter
}

# Create/pack the view and create the model

LogModel model -file $dbfile
LogView .logview
pack .logview


# Set up menus for filtering

menu .menubar 
menu .menubar.filter -tearoff 0
.menubar add cascade -label Filter -menu .menubar.filter

. configure -menu .menubar

# Populate the filter menu:


# Severity filtering;  Variables for each of the severity types:

set DEBUG   1
set INFO    1
set WARNING 1
set SEVERE  1
set DEFECT  1

set visibleApplication ""
set visibleSource      ""



.menubar.filter add command -label {Filter Severities:}
foreach sev  $severityList {
    .menubar.filter add checkbutton -onvalue 1 -offvalue 0 -variable $sev \
        -label $sev  -command updateFilter
}
.menubar.filter add separator
.menubar.filter add command -label Application... -command [list setTextFilter application]
.menubar.filter add command -label Source...      -command [list setTextFilter source]

.menubar.filter add separator
.menubar.filter add command -label Clear -command [list clearFilters]

# Start the periodic population.
#

set updateId -1
updateFilter;             # Setup the initial filter information.


