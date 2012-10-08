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

package provide barriers 1.0

package require Tk
package require snit

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
snit::widget EVB::BarrierStats::Summary {
    option -completecount     -default 0
    option -incompletecount   -default 0
    option -heterogenouscount -default 0
    
    constructor args {
        #
        #  Create the widgets
        
        label $win.title         -text {Barriers}
        label $win.completel     -text {Complete}
        label $win.incompletel   -text {Incomplete}
        label $win.heterogenousl -text {Heterogenous}
        
        label $win.complete     -textvariable ${selfns}::options(-completecount)
        label $win.incomplete   -textvariable ${selfns}::options(-incompletecount)
        label $win.heterogenous -textvariable ${selfns}::options(-heterogenouscount)
        
        # Lay them out.
        
        grid x                $win.title        x
        grid $win.completel   $win.incompletel  $win.heterogenousl
        grid $win.complete    $win.incomplete   $win.heterogenous -sticky e
        
        $self configurelist $args
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


if {[info proc ::EVB::test::updateWidgetOption] eq ""} {
    proc ::EVB::test::updateWidgetOption {widget name1 name2 op} {
        upvar #0 $name1 value
        $widget configure $name1 $value
        return ""
    }
}