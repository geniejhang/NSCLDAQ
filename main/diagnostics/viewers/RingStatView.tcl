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
# @file <filename>.tcl
# @brief <brief purpose>
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide RingStatView 1.0
package require Tk
package require snit
package require png


namespace eval RingStatViewNS {
    set here [file dirname [info script]];      # Install directory of script.
}

##
# @class RingStatView
#   Provides a view of ring buffers their clients and their statistics.
#   Ring buffers appear as folders.  Clients are lines under their ring buffers
#   clients are shown in two colors: Black for consumers and green for producers.
#

#
#   The columns for each consumer line are:
#      *   Last update time.
#      *   operations - Number of operations performed.
#      *   bytes      - Number of bytes performed.
#      *   Backlog    - Backlog in bytes  N/A is displayed for producers.
#
#   Consumers can be opened as well and have lines below them that desribe
#      *  PID of the consumer.
#
snit::widgetadaptor RingStatView {
    component tree;                # Will be a ttk:: treeview.
    
    delegate option * to tree
    
    
    # Array indexed by fully qualified ring names whose contents are the
    # Tree IDs of the ring folders.
    
    variable ringfolders -array [list]
    
    # Array indexed by tree id of the ring buffer folder.
    # The contents are lists of dicts containing:
    #   - tree id
    #   - PID
    #   - command.
    #      Note that there is other information about each of the clients but
    #      that's held in the children of the process.
    #
    
    variable appfolders -array [list]
    ##
    # typeconstructor
    #    Create the icon images.
    #    blip out some credit text for the icon images.
    #
    typeconstructor  {
        image create photo ringstat_folder -format png \
            -file [file join $::RingStatViewNS::here folder.png]
        image create photo ringstat_app    -format png \
            -file [file join $::RingStatViewNS::here program.png]
        set fd [open [file join $::RingStatViewNS::here iconcredit.txt] r]
        set credit [read $fd]
        close $fd
        puts $credit
    }    
    ##
    # constructor
    #   - Hull is a ttk::frame.
    #   - The frame contains a treeview on the left and a
    #   - scrollview on the right.
    #
    constructor args {
        installhull using ttk::frame
        
        # Subwidgets:
        
        set colnames [list properties updated operations bytes backlog]
        install tree using ttk::treeview $win.tree   \
            -show [list tree headings] -selectmode none \
            -columns $colnames
        
        $tree column  #0 -anchor w -stretch 1
        foreach name $colnames {
            $tree heading $name -text $name
            $tree column $name -anchor w -stretch 1
        }
        
        ttk::scrollbar $win.vscroll -orient vertical
        
        #  Setup the scrollbar/tree interaction.
        
        $win.vscroll configure -command [list $tree yiew]
        $tree        configure -yscrollcommand [list $win.vscroll set]
        
        #  Grid so that scrollbar stays fixed but the tree can expand in both
        #  directions.
        
        grid $tree -column 0 -row 0 -sticky nsew
        grid $win.vscroll -column 1  -row 0 -sticky nse
        grid rowconfigure $win   0    -weight 1
        
        grid columnconfigure $win 0     -weight 1
        grid columnconfigure $win 1     -weight 0
        
        # Make the tags:
        
        $tree tag configure producer -foreground green
        $tree tag configure consumer -foreground black
    }
    # Users should grid with non zero weight or pack with -expand 1 -fill both
    
    #-------------------------------------------------------------------------
    #  Public entry points.
    
    #  The main method adds to the tree.  This gets broken down into a bunch
    #  of little things as the logic can be a bit tricky.  See the comments
    #  of the methods called to accomplish each sub-piece.
    #
    # @param dict - result from a call to queryRingStatistics.
    #
    method newStatistics {dict} {
        dict for {fqn value} $dict {
            set ring [lindex $value 0]
            set clientsAndStats [lindex $value 1]
            $self _newRingInfo $ring $clientsAndStats
        }
    }
    #--------------------------------------------------------------------------
    #  Private entry points.
    
    ##
    # _addRingFolder
    #   Adds a new folder for a ring.  Folders have an image that is
    #   the folder icon
    #
    # @param ringDict  - dictionary with the ring specification.
    # @return string   - Item id.
    # @note as a side effect the item id is put in the ringfolders array.
    #
    method _addRingFolder {ringDict} {
        set fqname [dict get $ringDict fqname]
        set id [$tree insert {} end -text $fqname -image ringstat_folder]
        
        set ringfolders($fqname) $id
        set appfolders($id) [list]
        
        return $id
    }
    ##
    # _getPriorStats
    #    Given a stats list and the id of a client return the statistics prior to
    #    the last element of stats.   See _updateStats for an idea of what we
    #    have to deal with.
    #
    # @param id   - Id of client node for which we want the prior stats.
    # @param stats - List of statistics gotten since the last update.
    # @return dict - empty if there is no prior stat else a dict with the keys
    #                - timestamp - Timestamp at wich the previous item was made
    #                -  operations - Total number of ring operations performed at timestamp.
    #                -  bytes      - Total number of bytes transferred at timestamp.
    #                -  backlog    - Total number of backlogged bytes.
    #
    method _getPriorStats {id stats} {
        
        if {[llength $stats] > 1} {
            # Prior stat is the end-1 stat in the stats parameter.
            
            return [lindex $stats end-1]
        }
        #  Need to see if we have anything in the client node:
        
        set info [$tree item $id -value]
        if {[lindex $info 1] ne ""} {
            return [dict create timestamp [clock scan [lindex $info 1]]       \
                    operations [lindex [lindex $info 2] 0]                    \
                    bytes      [lindex [lindex $info 3] 0]                    \
                    backlog    [lindex [lindex $info 4] 0]                    \
            ]  
        } else {
            # There's no info in the node
            
            return [dict create];           # Can shimmer to an empty list.
        }
    }
    ##
    # _computeStatsItems
    #   Given the prior and current statistics dicts, compute the
    #   statistics items.  This will be a list of everything but the
    #   client properties.
    #
    # @param prior   - prior statistics dict - could e empty.
    # @param current - Current statistics.
    # @return list    containing in order:
    #                - Formatted timestamp.
    #                - ops  ops/sec
    #                - bytes  bytes/sec
    #                - backlog
    #
    method _computeStatsItems {prior current} {
        
        # Timestamp always come from the current item:
        
        set timestamp [clock format   [dict get $current timestamp] -format "%D %T"] 

        # Backlogs don't have a rate so:
        
        set backlog [dict get $current backlog]
        
        # all remaining items depend on whether or not pior is empty:
        
        if {[llength [dict keys $prior]] == 0} {
            # No prior information
            
            set operations [format %5d [dict get $current operations]]
            set bytes      [format %5d [dict get $current bytes]]
            set backlog    [format %5d [dict get $current backlog]]
        } else {
            # Prior info from the prior parameter
            # -- can compute rates in {ops,bytes}/sec
            
            set dt [expr {[dict get $current timestamp] - [dict get $prior timestamp]}]
            
            # If there's no time difference, somehow we've got the same messages
            #  again -e.g. duplicat aggregator.
            
            if {$dt == 0} {
                return ""
            }
            
            # Operations:
            
            set rawops [dict get $current operations]
            set opsrate [expr {($rawops - [dict get $prior operations]/$dt)}]
            set operations [format "%5d %5.2f/sec" $rawops $opsrate]
            
            # Bytes:
            
            set rawbytes [dict get $current bytes]
            set byterate [expr {($rawbytes - [dict get $prior bytes])/$dt}]
            set bytes [format "%d %.2f/sec" $rawbytes $byterate]
        }
        
        return [list $timestamp "$operations" "$bytes" "$backlog"]
    }
    
    ##
    # _updateStats
    #    Update the statistics entries on a client.
    #    The following cases must be handled:
    #    -   There's more than one statitics entry;  in that case rates can
    #        be computed fromt he last two entries in the list and totals
    #        come from the last entry.
    #    -  There's only one statistics entry and the node in the tree has
    #       statistics.  In that case, rates can be computed from the information
    #       in the node and the statistics entry.  Totals will come from the
    #       stat entry.
    #    -  There's one statistics entry but there is not any information in the
    #       node.   In that case, rates cannot be computed and totals come from
    #       the statistics entry.
    #  @note - because of how SQL queries work, there will never be an empty stats
    #        list.
    #
    # @param clientid   - Id of the node we're updating (a client node).
    # @param stats      - List of statistics entries.
    #
    #
    method _updateStats {clientId stats} {
        set last [lindex $stats end];    # Pull out the last statistices entry.
        set prior [$self _getPriorStats $clientId $stats]
        set items [$self _computeStatsItems $prior $last]
        
        if {$items eq ""} return
        
        # we need to fold in the properties part of the current stats line:
        
        set currentItems [$tree item $clientId -values]
        set currentItems [lreplace $currentItems 1 end {*}$items];   # Keep the client info.
        $tree item $clientId -values $currentItems
        
    }
    ##
    # _findClient
    #   Given the ID of a ringbuffer folder locates a specified client
    #   in the list of children.  Note the client may not exist.
    #
    # @param parent - Tree Id of the ringbuffer folder.
    # @param pid    - Pid of the client.
    # @param cmd    - Command the client is running
    # @return string - Id of the command in the tree.
    # @retval ""    - If the command is not found.
    # @note The parent is assumed to already exist.
    method _findClient {parent pid cmd} {
        foreach client $appfolders($parent) {
            set cpid [dict get $client pid]
            set ccmd [dict get $client command]
            if {($pid == $cpid) && ($ccmd eq $cmd)} {
                return [dict get $client id]
            }
        }
        return "";             # Not found.
    }
    ##
    # _createClient
    #    Create a new client entry
    #
    # @param parent - Id of the ring buffer node for which this is a client.
    # @param client - Client statistics/description dict.
    # @return string - id of newly created node.
    #
    method _createClient {parent client} {
        set tag consumer
        if {[dict get $client isProducer]} {
            set tag producer
        }
        set pid [dict get $client pid]
        set cmd [dict get $client command]
        set id [$tree insert $parent end                                        \
                -text $cmd -image ringstat_app -tags $tag \
                -values [list "PID = $pid" "" "" "" ""]                   \
        ]
        lappend appfolders($parent) [dict create pid $pid command $cmd id $id]
        
        return $id
    }
    ##
    # _addClientAndStats
    #    Adds clients to a ringbuffer parent folder.
    #    If necessary the client is added to the display as a parent of its
    #    properties.
    #
    # @param parent  - ID of the ringbuffer folder under which this client will
    #                  be put.
    # @parent clientDict - Dictionary that describes the parent program.
    # @param  statList   -  list of statistics.  Note that in most cases, only the
    #                        last entry of this list is relevant.
    #
    #
    method _addClientAndStats {parent clientDict statList} {
        set pid [dict get $clientDict pid]
        set cmd [dict get $clientDict command]
        set clientId [$self _findClient $parent $pid $cmd]
        
        if {$clientId eq ""} {
            set clientId [$self _createClient $parent $clientDict]
        }
        $self _updateStats $clientId $statList
    }
    
    ##
    # _newRingInfo
    #   Adds new ring information to the tree:
    #   - If the ring is not yet a folder on the treewidget, it is
    #     added.  Otherwise, it's id is looked up.
    #   - _newClientInfo is then called for each client to add client and
    #     statistics information.
    #
    # @param ringdict - ring buffer description dict.
    # @param List of pairs - first item of each pair is a client description dict,
    #                        remainder of each pair is a list of statistics.
    #                        for that client.
    #
    method _newRingInfo {ringdict clientlist} {
        set fqn [dict get $ringdict fqname]
        if {$fqn in [array names ringfolders]} {
            set parentId $ringfolders($fqn)
        } else {
            set parentId [$self _addRingFolder $ringdict]
        }
        #  At this point, parentId is the id of the folder for the ring in the tree:
        
        foreach entry $clientlist {
            set client [lindex $entry 0]
            set stats  [lindex $entry 1]
            $self _addClientAndStats $parentId $client $stats
        }
    }
    
}