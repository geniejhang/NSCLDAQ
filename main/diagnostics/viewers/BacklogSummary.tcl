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
# @file BacklogSummary.tcl
# @brief Display a summary of the programs that are making deep backlogs.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide BacklogSummary 1.0
package require Tk
package require snit
package require SqlWhere
##
# Locate icons:
#

namespace eval BacklogSummaryNS {
    variable here [file dirname [info script]]
}

##
# @class BacklogSummary
#
#  This snidget provides a display of the ring clients that are causing large
#  backlogs.  The widget is just a table of:
#
#    HOST, PID, Command, Backlog in bytes
#
#   For programs that have backlogs over a specified threshold.
#
# @note - the display is not live.  It requires manual update and this is
#         intentional so that there's no twinkling if there are programs close
#         to the threshold....unless the twinkling is manually caused.
#
#
# LAYOUT:
#     +------------------------------------------------------------+
#     |  [ Update ]                                                |
#     +------------------------------------------------------------+
#     |   Treeview with the actual data (vscroll)                  |
#                            ...
#     +------------------------------------------------------------+
#
snit::widgetadaptor BacklogSummary {
    option -dbcommand  -readonly 1
    option -ringname   -readonly 1
    option -ringhost   -readonly 1
    option -threshold  -default [expr 7*1024*1024]
    
    component table
    
    
    typevariable updateImage
    
    variable lastid     1
    
    ##
    # typeconstructor
    #     create/load the update image.
    #
    typeconstructor {
        set updateImage [image create photo -format png -file [file join $::BacklogSummaryNS::here refresh.png]]
    }
    
    ##
    # Constructor:
    #    Install a ttk::frame as the hull.
    #    Process the configuration options.
    #    Create the individual widgets.
    #    Lay them out.
    #    Run an update.
    #
    # @param args - configuration option/value pairs.
    #
    constructor args {
        installhull using ttk::frame
        $self configurelist $args
        
        set Columns [list Host PID Command Backlog]
        
        ttk::button $win.refresh -image $updateImage -command [mymethod _update]
        
        install table using ttk::treeview $win.table                           \
            -columns $Columns -displaycolumns $Columns -show headings          \
            -selectmode none -yscrollcommand [list $win.vscroll set]
        foreach col $Columns {
            $table heading $col -text $col -anchor w
        }
        
        ttk::scrollbar $win.vscroll -orient vertical -command [$table yview]
        
        grid $win.refresh -sticky nsw
        grid $table $win.vscroll -sticky nsew
        
        # Allow the table and only the table to expand.
        
        grid columnconfigure $win 0 -weight 1
        grid columnconfigure $win 1 -weight 0
        grid rowconfigure    $win 0 -weight 0
        grid rowconfigure    $win 1 -weight 1
        
        $self _update
    }
    
    ##
    # _update
    #   Update the display.  We're going to clear all the items from the
    #   tree and re populate it only with applications that are above -treshold.
    #
    method _update {} {

        $table delete [$table children {}]

        # Build up the filter:  Need the right ring/host name, items over threshold
        # and statistics with ids greater than lastid.
        
        set filter [AndFilter %AUTO%]
        set elements [list];            # Memorize components so we can kill them off.
        lappend elements [RelationToStringFilter %AUTO% r.name = $options(-ringname)]
        lappend elements [RelationToStringFilter %AUTO% r.host = $options(-ringhost)]
        lappend elements [RelationToNonStringFilter %AUTO% s.id > $lastid]
        lappend elements [RelationToNonStringFilter %AUTO% s.backlog >= $options(-threshold)]
        lappend elements [RelationToNonStringFilter %AUTO% s.timestamp >= [expr {[clock seconds] - 2}]]
        foreach element $elements {
            $filter addClause $element
        }
        
        #  Get the new data and kill off the filter:
        

        set data [$options(-dbcommand) queryRingStatistics $filter]
        foreach element $elements {
            $element destroy
        }
        $filter destroy
        
        
        #  If there's new data add it to the display...while we're at it we'll
        #  update lastid too.
        #
        dict for {fqn ringStatinfo} $data {
            set host [dict get [lindex $ringStatinfo 0] host]
            set clientStatList [lindex $ringStatinfo 1]
            foreach client $clientStatList {
                 set clientInfo [lindex $client 0]
                 set stats      [lindex $client 1]
                 set pid [dict get $clientInfo pid]
                 set command [dict get $clientInfo command]
                 set lastStat [lindex $stats end]
                 set id [dict get $lastStat id]
                 set backlog [dict get $lastStat backlog]
                 
                 if {$id > $lastid } {
                    set lastid $id
                 }
                 $table insert {} end  -value [list $host $pid $command $backlog]
            }
            
        }
        
    }
}