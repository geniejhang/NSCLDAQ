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
# @file late.tcl
# @brief Provide data late GUI information.
#


package provide Late 1.0
package require Tk
package require snit

namespace eval EVB {}

##
# @class EVB::sourceLate 
#
# Provides a widget that gives information about the
# data late conditions for a specific data source.
# 
# OPTIONS
#    - -id  - Source id of the widget.
#    - -count - Number of data late conditions.
#    - -worst - Delta t of worst case late.
#
# FORMAT
#
#  +---------------------------------------+
#  | Sourceid    #lates       #worstDt     |
#  +---------------------------------------+

snit::widget EVB::late {
    component innerHull

    option -id
    option -count
    option -worst

    delegate option * to innerHull

    ##
    # constructor.
    #  - Create/layout widgets.
    #  - process options.
    #
    # @param args Argument value parirs.
    #
    constructor args {
    
	install innerHull using ttk::frame $win.innerHull 

	ttk::label $win.innerHull.sourceid \
	    -textvariable ${selfns}::options(-id) \
	    -width 5
	ttk::label $win.innerHull.count \
	    -textvariable ${selfns}::options(-count)  \
	    -width 6
	ttk::label $win.innerHull.worst \
	    -textvariable ${selfns}::options(-worst) \
	    -width 11

	# layout the widgets in the inner hull:

	grid $win.innerHull.sourceid \
	     $win.innerHull.count    \
	     $win.innerHull.worst -sticky w

	# Make the inner hull fill the actual hull:

	grid $win.innerHull -sticky nsew


	$self configurelist $args
    }
   
}

##
# @class EVB::lateSummary
#
# Widget that summarizes the data late packets.
# Specific information includes the total number of late
# fragments and the worst case time difference.
#
# OPTIONS
#
#   - -count - total number of late fragments.
#   - -worst - Worst case lateness.
#
# LAYOUT
#
#   +-----------------------------------------------+
#   | Late:  <late frags>  Worst: <worst time diff> |
#   +-----------------------------------------------+
#
snit::widget EVB::lateSummary {
    component innerHull
    
    option -count
    option -worst

    delegate option * to innerHull

    ##
    # constructor
    #  - Create/layout widgets.
    #  - process options.
    #
    constructor args {
	
	install innerHull using ttk::frame $win.innerHull

	ttk::label $win.innerHull.latel -text "Late: "
	ttk::label $win.innerHull.late  \
	    -textvariable ${selfns}::options(-count) \
	    -width 6

	ttk::label $win.innerHull.worstl -text "Worst: "
	ttk::label $win.innerHull.worst \
	    -textvariable ${selfns}::options(-worst) \
	    -width 11

	# layout widgets in the innerHull

	grid $win.innerHull.latel -column 0 -row 0 -sticky w
	grid $win.innerHull.late  -column 1 -row 0 -sticky e
	grid $win.innerHull.worstl -column 2 -row 0 -sticky w
	grid $win.innerHull.worst -column 3 -row 0 -sticky e

	# paste the innerHull into the hull 

	grid $win.innerHull -sticky nsew

	# process construction line options:

	$self configurelist $args
    }
}
##
# EVB::lateFragments
#
#  Displays the late fragment statistics.
#  this includes a summary widget and a 
#  set of widgets that show the detailed per source
#  statistics for all sources that have been late
#  since the last clear.
#
#
# OPTIONS
#   - -count -- Total number of late fragments.
#   - -worst -- Worst case late time difference.
#
# METHODS
#   - source id count late - provide per source statistics.
#   - clear -- clear statistics and destroy the set of
#              per source statistics.
#
# LAYOUT
#
#   +--------------------------------------+
#   | Late: -count   Worst: -worst         |
#   +--------------------------------------+
#   | Source    Lates    Worst             |
#   | id1       lates1   worst1            |
#   \                                     /
#   /                                     \
#   +--------------------------------------+
#
# NOTES:
#   # The individual source data includes a 
#   scroll bar if needed.
#
snit::widget EVB::lateFragments {
    component innerHull
    component summary

    delegate option -count to summary
    delegate option -worst to summary
    delegate option * to innerHull



}

