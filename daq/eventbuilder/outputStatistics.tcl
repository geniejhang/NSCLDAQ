#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


##
# @file outputStatistics.tcl
# @author Ron Fox
# @brief Event builder output statistics.
#
# This file provides the widgets needed to create a set of output statistics.
# the ouptut statistics is a single widget named:
#  
#  EVB::outputStatistics defined in this file below.
#

package require Tk
package require snit
package provide EVB::ouputStatistics 1.0

##
#  Ensure that the EVB namespace is defined so that we can put stuff into it.

namespace eval EVB {
}

#-----------------------------------------------------------------------------------

##
# @class EVB::outputSummary
#
#  Widget that summarizes the output statistics of the event orderer.
#
# OPTIONS:
#  - -fragments        - Number of fragments that have been output
#  - -hottestid        - Source id with the most fragments output.
#  - -hottestcount     - Number of fragments output by -hottestid.
#  - -coldestid        - Source id with the fewest output fragments.
#  - -coldestcount     - Number of fragments output by -coldestid.
#
#  LAYOUT:
#   Rough layout.
# \verbatim
#
#   +-----------------------------------------+
#   |Total Fragments          <fragcount>     |
#   |           id            count           |
#   | hottest   <id>          <fragcount>     |
#   | coldest   <id>          <fragcount>     |
#   +-----------------------------------------+
# \endverbatim
#
snit::widget ::EVB::outputSummary {
    option -fragments    -default 0
    option -hottestid    -default ""
    option -hottestcount -default ""
    option -coldestid    -default ""
    option -coldestcount -default ""
    
    
    constructor args {
        #  Create the widgets (all ttk::label s)
        
        ttk::label $win.fragl   -text {Total fragments}
        ttk::label $win.idl     -text {source id}
        ttk::label $win.countl  -text {fragment count}
        ttk::label $win.hotl    -text {Hottest}
        ttk::label $win.coldl   -text {Coldest}
        
        ttk::label $win.frag           -textvariable ${selfns}::options(-fragments)
        ttk::label $win.hotid          -textvariable ${selfns}::options(-hottestid)
        ttk::label $win.hotcount       -textvariable ${selfns}::options(-hottestcount)
        ttk::label $win.coldid         -textvariable ${selfns}::options(-coldestid)
        ttk::label $win.coldcount      -textvariable ${selfns}::options(-coldestcount)
        
        # Lay them out
        
        grid $win.fragl   -           $win.frag
        grid x            $win.idl    $win.countl
        grid $win.hotl    $win.hotid  $win.hotcount  -sticky e
        grid $win.coldl   $win.coldid $win.coldcount -sticky e
        
        # Process the initial configuration.
        
        $self configurelist $args
    }
}

##
# @class EVB::outputStatistics
#
#   This snit::widget provides a display of the output statistics for the event
#   orderer.   This display is divorced from the mechanisms of getting the statistics.
#   the only thing we do is provide a widget that presents the statistics.
#
# OPTIONS:
#   - -totalfragments - Total number of output fragments emitted.
#
# METHODS:
#   - addSource           - adds an event source to the widget.
#   - removeSource        - removes an event source from the widget.
#   - setSourceFragments  - Provides the number of fragments a source has emitted.
#   - clear               - Clears the fragment counters on all sources.
#   - listSources         - Returns the list of sources.
#
# LAYOUT:
#
# \verbatim
#   +-----------------------------------+
#   | Total Fragments <frag-count>      |
#   +-----------------------------------+
#   | Source            Fragments       |
#   |  id1              count1          |
#   \                                   \
#   /                                   /
#  +-----------------------------------+
# \endveratim
snit::widget ::EVB::outputStatistics {
    option -totalfragments -default 0 -configuremethod configTotals

    delegate option * to innerhull
    delegate method  * to innerhull

    variable sourceIds [list];		# ordered list of ids.

    ##
    # constructor
    #    creates the initial widgets 

    constructor args {

	#  Create the widgets.

	install innerhull using ttk::frame $win.hull; # ttk frame for a backdrop.
	
	set top [ttk::frame $innerhull.top -relief groove -borderwidth 3]
	ttk::label $top.totall -text {Total Fragments}
	ttk::label $top.total  -textvariable ${selfns}::options(-totalfragments)

	set bottom [ttk::frame $innerhull.bottom]
	ttk::label $bottom.sourcel -text {Source Id} 
	ttk::label $bottom.countl  -text {Fragments}


	#  Layout the widgets.

	grid $top.totall $top.total
	grid $top -sticky new

	grid $bottom.sourcel $bottom.countl
	grid $bottom -sticky nswe

	pack  $innerhull -expand 1 -fill both

	# Process the options
	
	$self configurelist $args
    }
    #------------------------------------------------------------------------
    # Public methods.
    #
    
    ##
    # addSource
    #
    #  Add a new data source to the widget.  The data sources are maintained
    #  ordered by source id low to high from top to bottom.
    #  If necessary sources are pushed down to make room for the new source.
    #
    # @param sourceId - Id of new source to add.
    #
    # @note it is an error to add a duplicate source.
    #
    method addSource id {
	if {$id in $sourceIds} {
	    error "$id is already managed"
	}

	# figure out where this goes.. unmanage the old widgets.
	# Create the new ones, 
	# add the id to the list and remanage it and all those below it.
	#
	
	ttk::label $innerhull.bottom.label$id   -text $id
	ttk::label $innerhull.bottom.counter$id -text 0 -justify right -anchor e

	set newIndex [$self _getInsertIndex $id]; # Determine where this goes.
	set sourceIds [linsert $sourceIds $newIndex $id]

	$self _unManage $newIndex
	$self _manage   $newIndex
    }
    ##
    # removeSource
    #
    #   Removes an event source from the widget.  It is an error to remove a source
    #   that is not in the widget.
    #
    # @param id - The source id of the data source being removed.
    #
    method removeSource id {
	set removeIndex [lsearch -exact  $sourceIds $id]
	if {$removeIndex == -1} {
	    error "$id is not managed"
	}

	#
	# Unmanage the widgets and all those below,
	# remove from list, destroy the widgets and remanage the remaining widget:

	$self _unManage $removeIndex

	destroy $innerhull.bottom.label$id
	destroy $innerhull.bottom.counter$id
	
	set sourceIds [lreplace $sourceIds $removeIndex $removeIndex]
	$self _manage $removeIndex
	
    }
    ##
    # setSourceFragments
    #
    #  Sets the number of fragments in a source.
    #  - it is an error for the source id to not be in the managed list.
    #  - it is an error for the counter not to be an integer.
    #
    # @param id      - Source id
    # @param counts - Number of fragments 
    # 
    method setSourceFragments {id counts} {
	if {$id ni $sourceIds} {
	    error "$id is not managed"
	}
	if {![string is integer -strict $counts]} {
	    error "$counts must be an integer"
	}

	$innerhull.bottom.counter$id configure -text [format "% 6d" $counts]
    }
    ##
    # clear
    #   Clears the counters in all of the source elements.
    #
    method clear {} {
	foreach id $sourceIds {
	    $self setSourceFragments $id 0
	}
    }
    ##
    # Return the list of current source Ids:
    #
    # @return list - sourceIds
    #
    method listSources {} {
	return $sourceIds
    }

    #----------------------------------------------------------------
    # Private methods.

    ##
    # _getInsertIndex
    #
    # Return the correct insertion index in sourceIds for a new
    # id such that this list can remain ordered.
    #
    # @param id - new id to add.
    #
    method _getInsertIndex sourceId {

	# this is really brute force with the idea that 
	# there won't be a huge number of sourcdes:

	set newSources [lsort -integer [concat $sourceIds $sourceId]]
	return [lsearch -exact $newSources $sourceId]
    }
    ##
    # _unManage
    #
    # grid  all widgets created for elements in the sourceIds list
    # at and past index
    #
    # @param index - index of the first iem in sourceIds to unmanage.
    #
    #    
    method _unManage index  {
	foreach id [lrange $sourceIds $index end] {
	    grid  $innerhull.bottom.label$id
	    grid  $innerhull.bottom.counter$id
	}
    }
    ##
    # _manage
    #
    #  Grid all widget in the list of source ids at and after index
    #  in that list.  The -row to use is just index+1.  Usually
    #  this is done after _unManage has removed these widgets and the
    #  list has been adjusted.
    #
    # @param index - which item to start re-gridding from
    method _manage   index {
	set row [expr {$index + 1}]

	foreach id [lrange $sourceIds $index end] {
	    grid $innerhull.bottom.label$id   -row $row -column 0
	    grid $innerhull.bottom.counter$id -row $row -column 1 -sticky w
	    
	    incr row
	}
    }


}

#----------------------------------------------------------------------------
#
# internal self test methods.

namespace eval EVB {
    namespace eval test {}
}
##
# test the outputSummary widget:
#
proc ::EVB::test::outputSummary {} {
    ::EVB::outputSummary .target
    pack .target
    
    # now the control panel:
    
    toplevel .control
    
    label .control.fragl -text Fragments
    entry .control.frag  -textvariable -fragments
    
    label .control.hotidl -text {Hot id}
    entry .control.hotid  -textvariable -hottestid
    
    label .control.hotcountl -text {Hot count}
    entry .control.hotcount  -textvariable -hottestcount
    
    label .control.coldidl  -text {Cold id}
    entry .control.coldid   -textvariable -coldestid
    
    label .control.coldcountl -text {Cold Count}
    entry .control.coldcount -textvariable -coldestcount
    
    grid .control.fragl .control.frag
    grid .control.hotidl .control.hotid
    
    grid .control.hotcountl .control.hotcount
    
    grid .control.coldidl .control.coldid
    grid .control.coldcountl .control.coldcount

    # Add variable traces to propagate values -> the .target.
    
    foreach var [list -fragments -hottestid -hottestcount -coldestid -coldestcount] {
        trace add variable ::$var write [list ::EVB::test::updateWidgetOption .target]
    }
}
##
# Testing utility to update a widget option from a trace on a variable
# that has a name matching the option:
#
# @param widget - the widget to modify.
# @param name1  - name of the variable (and option)
# @param name2  - index of the variable if an array.
# @param op     - operation performed.
#

if {[info proc ::EVB::test::updateWidgetOption] eq ""} {
    proc ::EVB::test::updateWidgetOption {widget name1 name2 op} {
        upvar #0 $name1 value
        $widget configure $name1 $value
        return ""
    }
}