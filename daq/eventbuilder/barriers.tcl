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
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file barriers.tcl
# @brief Widgets to display barrier statistics.

#-------------------------------------------------------------------------------
#
#  Package stuff

# If this location is not in the path, add it.

set here [file dirname [info script]]
if {[lsearch -exact $auto_path $here] == -1} {
    lappend auto_path $here
}

package provide EVB::barriers 1.0

package require Tk
package require snit
package require Iwidgets
package require EVBUtilities

#------------------------------------------------------------------------------
#  Establish namespaces.

namespace eval EVB {
    namespace eval BarrierStats {}
}

#-------------------------------------------------------------------------------
#  The widgets
#


##
#  @class Summary
#
#   Widget that summarizes barrier statistics.
#
# OPTIONS:
#   - -completecount     - Number of complete barriers received.
#   - -incompletecount   - Number of incomplete barriers received.
#   - -heterogenouscount - Number of barriers that have mixed types.
#
# LAYOUT:
#   +--------------------------------------------+
#   |                Barriers                    |
#   |  Complete      Incomplete   Heterogenous   |
#   |  <complete>    <incomplete> <heterogenous> |
#   +--------------------------------------------+
#
snit::widgetadaptor EVB::BarrierStats::Summary {
    option -completecount     -default 0
    option -incompletecount   -default 0
    option -heterogenouscount -default 0
    
    delegate option -text to hull
    constructor args {
        installhull using ttk::labelframe
        #
        #  Create the widgets
        
        ttk::label $win.title         -text {Barriers}
        ttk::label $win.completel     -text {Complete}
        ttk::label $win.incompletel   -text {Incomplete}
        ttk::label $win.heterogenousl -text {Heterogenous}
        
        ttk::label $win.complete     -textvariable ${selfns}::options(-completecount)
        ttk::label $win.incomplete   -textvariable ${selfns}::options(-incompletecount)
        ttk::label $win.heterogenous -textvariable ${selfns}::options(-heterogenouscount)
        
        # Lay them out.
        
        grid x                $win.title        x
        grid $win.completel   $win.incompletel  $win.heterogenousl
        grid $win.complete    $win.incomplete   $win.heterogenous -sticky e
        
        $self configurelist $args
    }
}
##
# @class BarrierTypes
#
#  Displays a set of barrier type counts for a data source.
#
# OPTIONS
#    none
# METHODS
#    setCount barrierId count - Sets the number of barrier fragments
#                               gotten for barrierId to count.
#    clear                    - Sets all barrier counts to zero.
#    reinit                   - Gets rid of all 'table' elements.
#
#
# LAYOUT
#  +---------------------------------------+
#  |   Barrier Type          Count         |
#  |         id                count      ^|
#  |      ...                   ...       V|
#  +---------------------------------------+
#
#  Barrier ids are always maintained in sorted order (ascending).
#
snit::widget EVB::BarrierStats::BarrierTypes {
    component container
    
    delegate method setCount to container as setItem
    delegate method clear    to container
    delegate method reinit   to container
    
    delegate option * to container
    
    ##
    # Constructor
    #
    # @param args -option value pairs
    #
    constructor args {
        
        install container using EVB::utility::sortedPair $win.sp  \
            -title {Source Stats} -lefttitle {id} -righttitle {Bar. frags.}
        grid $win.sp  -sticky nsews
        
        
        
    }

}


#------------------------------------------------------------------------------
#   Testing stuff.

##
# Testing utility to update a widget option from a trace on a variable
# that has a name matching the option:
#
# @param widget - the widget to modify.
# @param name1  - name of the variable (and option)
# @param name2  - index of the variable if an array.
# @param op     - operation performed.
#

namespace eval EVB {
    namespace eval test {}
}

proc EVB::test::BarrierSummary {} {
    
    # Wiget under test.
    
    ::EVB::BarrierStats::Summary .target
    pack .target
    
    # Control panel for test:
    
    toplevel .panel
    label .panel.completel -text {complete}
    entry .panel.complete  -textvariable -completecount
    grid .panel.completel .panel.complete
    
    label .panel.incompletel -text {incomplete}
    entry .panel.incomplete  -textvariable -incompletecount
    grid  .panel.incompletel .panel.incomplete
    
    label .panel.heterogenousl -text {heterogenous}
    entry .panel.heterogenous -textvariable -heterogenouscount
    grid  .panel.heterogenousl .panel.heterogenous
    
    
    foreach var [list -completecount -incompletecount -heterogenouscount] {
        trace add variable ::$var write [list EVB::test::updateWidgetOption .target]
    }
    
}

proc EVB::test::BarrierTypes {} {
    EVB::BarrierStats::BarrierTypes .target
    pack .target
    
    # Control panel widgets and actions
    
    toplevel .panel
    ttk::button .panel.clear -text Clear -command [list .target clear]
    ttk::button .panel.reinit -text Empty -command [list .target reinit]
    
    ttk::label .panel.idl -text Id
    ttk::label .panel.cl  -text Counts
    ttk::entry .panel.id
    ttk::entry .panel.c
    ttk::button .panel.set -text set -command {.target setCount [.panel.id get] [.panel.c get]}
    
    grid .panel.idl .panel.cl
    grid .panel.id  .panel.c .panel.set
    grid .panel.clear .panel.reinit
    
    

}


if {[info proc ::EVB::test::updateWidgetOption] eq ""} {
    proc ::EVB::test::updateWidgetOption {widget name1 name2 op} {
        upvar #0 $name1 value
        $widget configure $name1 $value
        return ""
    }
}