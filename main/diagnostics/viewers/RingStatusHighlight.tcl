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
# @file RingStatusHighlight.tcl
# @brief Follow information about the ringbuffers in the dashboard changing
#        highlights as needed to indicate data flow bottlenecks.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide RingStatusHighlight 1.0
package require Tk
package require snit
package require SqlWhere

##
# Provides a 'class' that will monitor the status of ring buffers we care
# about (all hosts) and highlight them as needed:
# ring buffers that have a client with a high backlog will be highlighted in red.
#
# OPTIONS:
#   *  -canvas    - Canvas containing the dashboard (readonly)
#   *  -rings     - List of dashboard objects that are the rings (readonly).
#   *  -since     - Least recent timestamp allowed (readonly)
#   *  -threshold - Threshold value for the backlog at which rings get highlighted.
#   *  -dbapi     - Database API used to query ring statistics.
#   *  -interval  - Seconds between update polls.
#   *  -command   - Script to execute when a ring item is clicked.
#
# SUBSTITUTIONS:
#   *   %W        - name of RingStatusHighlight object doing the dispatch.
#   *   %R        - Ring dashboard object that triggered the dispatch.
#   *   
#
# @note - the assumption is that the application is largely event loop driven
# @note - Requires the database injector is running on the database
#         for which we got an API.
#
snit::type RingStatusHighlight {
    option -canvas -default {} -readonly 1
    option -rings  -default [list] -readonly 1
    option -since   -readonly 1;    # Earliest time allowed.
    option -threshold -default [expr 7*1024*1024];    #Backlog bytes for highlight.
    option -dbapi  -default ""
    option -interval -default 2;                     # Second bewteen updates.
    option -command -default [list]
    
    variable afterId -1;                           # After ID for next update.
    variable lastId  -1;                           # Most recent stat id seen.
    variable minDateCond;                          # Query term for minimum date
    variable ringsCond;                            # Query term for acceptable rings.
    variable ringLikes [list];                            # list of acceptable ring LIKE clauses.
    
    ##
    # constructor
    #   - Process options.
    #   - Create the fixed conditions.
    #   - Establish events on the rings to dispatch -command.
    #   - Start the update cycle.
    #
    # @param args - option value pairs.
    #
    constructor args {
        set options(-since) [clock seconds]
        $self configurelist $args
        
        $self _createConditions
        $self _establishEvents
        $self _update
    }
    ##
    # Destructor:
    #   - Kill off the 'ringstatus' tag which kills off the events that are
    #     bound to it.
    #   - Kill off the filter chunks we made.
    #   - Cancel the update after id.
    #
    destructor {
        $options(-canvas) dtag ringstatus ringstatus
        
        $minDateCond destroy
        foreach ring $ringLikes {
            $ring destroy
        }
        $ringsCond destroy
        
        after cancel $afterId
    }
    
    #--------------------------------------------------------------------------
    #  Private methods:
    #
    
    ##
    # _createConditions
    #   The query will be an AND or several conditions; some of these we can fix
    #   and pre-build:
    #   - Earliest allowed time for status data.
    #   - The set of rings we care about... as an Or of below.
    #   - Each ring that we do care about -- we're going to get
    #     the query in terms of LIKE ring@host% terms ored together.
    #     doing it this way gives us the remote proxy rings as well as the
    #     primary data source ring.
    #
    method _createConditions {} {
        set minDateCond [RelationToNonStringFilter %AUTO% s.timestamp >= $options(-since)]
        set ringsCond   [OrFilter %AUTO%]
        foreach item $options(-rings) {
            set fullName [$item cget -title]
            set nameElements [split $fullName @]
            set ringName     [lindex $nameElements 0]
            set hostName     [lindex $nameElements 1]%
            
            set cond [RawQueryFilter %AUTO% "((r.name = '$ringName') AND (r.host LIKE '$hostName'))" ]
            $ringsCond addClause $cond
            lappend ringLikes $cond
        }
    }
    
    ##
    # _establishEvents
    #   Tag all of the ring icon items on the canvas with the 'ringstatus' tag
    #   and bind the B1 event to our -command dispatch method.
    #
    method _establishEvents {} {
        set canvas $options(-canvas)
        foreach item $options(-rings) {
            set id [$item cget -id]
            $options(-canvas) addtag ringstatus withtag $id
        }
        $canvas bind ringstatus <Button-1> [mymethod _dispatch -command]
    }
    ##
    # _update
    #   -   Schedule ourselves to run again after -inteval >seconds<
    #   -   Get updated information about each ring.
    #   -   For each ring whose backlog is greater than the -threshold,
    #       ensure the ring is highlighted in red.  For rings not over the
    #       threshold, make sure the ring is highlighted in white.
    #   -  Compute a new lastId value.
    #
    #  Note the highlighting is complicated a bit by the fact that we also are
    #  monitoring remote proxy rings.  Therefore we'll keep a scorecard
    #  of each ring and its state.
    #
    method _update {} {
        # Reschedule this method    
    
        set updateMs [expr {1000 * $options(-interval)}]
        set afterId [after $updateMs [mymethod _update]]
        
        #  Compute the final condition, get the data and
        #  destroy the filter.
        
        set finalFilter [AndFilter %AUTO%]
        set idFilter    [RelationToNonStringFilter %AUTO% s.id > $lastId]
        
        $finalFilter addClause $idFilter
        $finalFilter addClause $minDateCond
        $finalFilter addClause $ringsCond
        
        ### DEBUG ###
        
        
        set newData [$options(-dbapi) queryRingStatistics $finalFilter]
        puts $newData
        puts "---------------------------"
        $finalFilter destroy
        $idFilter    destroy
        
        $self _updateHighlights $newData
        $self _updateLastId     $newData
    }
    ##
    # _updateHighlights
    #   Given the most recent data, updates the highlighting of a ring.
    #   There are several cases consider:
    #   - There's no data from the ring - the prior state is unchanged.
    #   - There's at least one item for the ring over threshold - red highlight.
    #   - There's at least one item for the ring and none of them are overthreshold
    #     (no highlight).
    #
    #  All of this is done by first tossing rings up into an array indexed by
    #  fqrn.  The contents of each of those arrays is a two element list.
    #  the first element is the Dashboard object for the ring. The second element
    #  is an item that is either the value 'unseen' or a number indicating the
    #  number of overthreshold items seen for that ring.  Once all the score
    #  keeping is done; 'unseen' rings are left alone.  Rings with a tally of
    #  0 are, if needed, forced the canvas background.  Rings with a tally
    #  greater than 0 are given a red highlight
    #
    # @param - data  New data from the status message system.
    #
    method _updateHighlights data {
        # Create the lookup array:
        #
        array set scoreboard [list]
        foreach ring $options(-rings) {
            set name [$ring cget -title]
            
            set scoreboard($name) [list $ring unseen]
        }
        #  Now look over the data:
        
        dict for {fqrn value} $data {
            
            #  If this is a proxy reduce the name to the original ring name:
            
            set elements [lrange [split $fqrn @] 0 1];   # only want ring@host
            set ringName [join $elements @]
            
            set clientsAndStats [lindex $value 1]
            foreach client $clientsAndStats {
                set stats [lindex [lindex $client 1] end]
                if {[dict get $stats backlog] >= $options(-threshold) } {
                    set increment 1
                } else {
                    set increment 0
                }
                #  If the ring's value is still 'unseen' make it zero and then
                #  apply the increment selected above to it:
                
                set dashboardInfo $scoreboard($ringName)
                set counter [lindex $dashboardInfo 1]
                if {$counter eq "unseen"} {
                    set counter 0
                }
                incr counter $increment
                set dashboardInfo [lreplace $dashboardInfo 1 1 $counter]
                set scoreboard($ringName) $dashboardInfo
            }
        }
        #  Now run over all the ring dashboard elements that need setting:
        
        foreach name [array names scoreboard] {
            set statInfo $scoreboard($name)
            set counter [lindex $statInfo 1]
            if {$counter ne "unseen"} {
                set dashEntry [lindex $statInfo 0]
                if {$counter > 0} {
                    # must highlight red.
                    if {[$dashEntry cget -highlight] ne "red"} {
                        $dashEntry configure -highlight red
                    }
                } else {
                    #  Must have background color
                    
                    set c $options(-canvas)
                    set bg [$c cget -background]
                    if {[$dashEntry cget -highlight] ne $bg} {
                        $dashEntry configure -highlight $bg
                    }
                }
            }
        }
        
    }
    ##
    # _updateLastId
    #    Set the lastId element to be the largest id in the data we got
    #    this time around.  Doing this properly ensures that the
    #    data we get next time around will be totally new:
    #
    # @param data -data from the queryRingStatistics call we earlier made.
    #  @note the assumption is that the last id in each result set is the largest.
    #
    method _updateLastId data {
        foreach ringAndClient [dict values $data] {
            set clientAndStats [lindex $ringAndClient 1]
            foreach client $clientAndStats {
                set lastStat [lindex [lindex $client 1] end]
                set id [dict get $lastStat id]
                if {$id > $lastId} {set lastId $id}
            }
        }
    }
    method dispatch option {};            # Stub.
}
