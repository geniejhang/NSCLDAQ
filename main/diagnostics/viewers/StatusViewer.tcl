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
# @file StatusViewer.tcl
# @brief Provides a one-stop display for status from log messages,
#        ring buffer statistics and readout programs.
#
# @author Ron Fox <fox@nscl.msu.edu>
#

##
# The program will display a ttk::notebook.  The notebook will have three tabs
# left to right:
#  - Ring Buffers - displays the ring buffer statistics.
#  - Readouts     - Displays the readout statistics.
#  - Messages     - Displays the log messages.
#

#  Set up  the auto_path to include the NSCLDAQ Tcl Libraries.
#  we assume that we are living in e.g. $DAQBIN
#

set here [file dirname [info script]]
set libdir [file normalize [file join $here .. TclLibs]]
lappend auto_path $libdir

##
#  Pull in the packages we use:

package require Tk
package require statusMessage

package require ReadoutStatController
package require ReadoutStatView
package require ReadoutStatModel

package require RingStatController
package require RingStatModel
package require RingStatView

package require LogView
package require  LogModel

package require SqlWhere

#
#   We need a database file:

if {[llength $argv] != 1} {
    puts stderr "Usage: "
    puts stderr "    StatusViewer dbfile"
    puts stderr "Where:"
    puts stderr "     dbfile - is the name fo the filew with the status database."
    puts stderr "               The invoker must ensure this database is getting "
    puts stderr "               populated."
    exit -1
}

#  Create the database query object for the models:

set dbObj [statusdb create $argv readonly]

# Create the notebook:

set nb [ttk::notebook .nb]
# ttk::notebook::enableTraversal $nb


#  The Ring statistics tab:

set rf [ttk::frame $nb.rf]
set rmodel [RingStatModel %AUTO% -dbcommand $dbObj]
set rview  [RingStatView $nb.rf.v]
set rctl   [RingStatController %AUTO% -model $rmodel -view $rview]
pack $rview -fill both -expand 1
$nb add $rf -text {Ring Stats}

#  Readout statistics tab:

set rdf [ttk::frame $nb.rdf]
set rdmodel [ReadoutStatModel %AUTO% -dbcommand $dbObj]
set rdview  [ReadoutStatView $rdf.v]
set rdctl   [ReadoutStatController %AUTO% -view $rdview -model $rdmodel]
pack $rdview -fill both -expand 1
$nb add $rdf -text {Readout Stats}

#  Log messages are a bit more trying because we never built a model like we
#  should have.  For now forget about filtering.
#


##
# getLastLogId
#   Returns the largest id in the log messages.
proc getLastLogId {data} {
    set result -1
    foreach datum $data {
        set id [dict get $datum id]
        if {$id > $result} {
            set result $id
        }
    }
    return $result
}

##
# updateLogView
#    Updates a log view and reschedules ourselves to go again.
# @param db    - Database query command.
# @param v     - LogView widget.
# @param id    - Largest message Id previously seen.
#
proc updateLogView {db v id} {
    #  Create the filter:
    
    set filter [RelationToNonStringFilter %AUTO% id > $id]
    set data [$db queryLogMessages $filter]
    $filter destroy
    
    $v load $data
    
    if {[llength $data] > 0} {
        set lastId [getLastLogId $data]
    } else {
        set lastId $id
    }
    
    after 2000 [list updateLogView $db $v $lastId]
}


set lf [ttk::frame $nb.lf]
set lv [LogView $lf.v -colors [dict create INFO green DEBUG orange WARNING purple SEVERE red DEFECT red]]
pack $lv -fill both -expand 1
$nb add $lf -text {Log Messages}

updateLogView $dbObj $lv 0

pack $nb -fill both -expand 1




