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
package require Iwidgets

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
    #
    #  Array indexed by barrier id of barrier counters.
    #  Note that corresponding widgets are
    #   ....id$barrierid and count$barrierid
    #  Where barrierid isn index to the array.
    #
    variable counters -array [list]
    variable container;                     # Container of the counter widgets.
    
    ##
    # Constructor
    #
    # @param args -option value pairs
    #
    constructor args {
        
        # Create the widgets and lay them out.
        #

        # The titles are above th srolling frame so that
        # they are always visible.

        ttk::label $win.typel  -text {Barrier Type}
        ttk::label $win.countl -text {Count}
        
        iwidgets::scrolledframe $win.frame -hscrollmode none -vscrollmode dynamic \
                                        -width 256 -relief groove -borderwidth 3
        
        set container [$win.frame childsite]
        
        # Layout the widgets:
        
        grid $win.typel $win.countl
        grid $win.frame -columnspan 2 -sticky ews
        
        
        
    }
    #--------------------------------------------------------------------------
    # public methods
    
    ##
    # setCount
    #   Set the number of times a barrier type has been seen.  If necessary,
    #   new widgets are created for the barrier type and added to the counters
    #   array.
    #
    #  @param id[in]    The barrier id to dd.
    #  @param count[in] Number of times the barrier was seen.
    #
    method setCount {id count} {
        if {[array names counters $id] eq ""} {
            $self _addId $id
        }
        set counters($id) $count;          # This is bound to the counter label.
    }
    
    ##
    # clear
    #
    #  Set all counters to zero.
    #
    method clear {} {
        foreach index [array names counters] {
            set counters($index) 0
        }
    }
    ##
    # reinit
    #
    #  Destroy all the existing counter widgets and the counters themselves.
    #
    method reinit {} {
        foreach index [array names  counters] {
            destroy $container.id$index
            destroy $container.count$index
            unset counters($index)
        }
    }
    
    #--------------------------------------------------------------------------
    # private methods
    
    ##
    # _addId
    #
    #  Add a new counter to the set of counters in the scrolled frame.
    #  The counter's value starts as zero.
    #
    # @param id[in] - new id to add.
    #
    method _addId id {
        set sortedIds [lsort -integer -increasing [array names counters]]
        set counters($id) 0
        
        #  Create the new widgets
        
        ttk::label $container.id$id    -text $id -width 15 -anchor e
        ttk::label $container.count$id -textvariable ${selfns}::counters($id) \
                                       -width 20 -anchor e
   
        
        set insertRow 0
        if {$id > [lindex $sortedIds 0]} {     # else it's the first element.
            foreach contents $sortedIds {
                if {$sortedIds <= $id} {
                    break
                }
                incr insertRow
            }
        }
        
        #  Tell grid to forget the items following insertRow in the list:
        
        set afterIds [lrange $sortedIds $insertRow end]
        foreach line $afterIds {
            grid forget $container.id$id
            grid forget $container.count$id
        }
        set afterIds [linsert $afterIds 0 $id]
        foreach line $afterIds {
            grid $container.id$line    -row $insertRow -column 0 -sticky e
            grid $container.count$line -row $insertRow -column 1 -sticky e
            incr insertRow
        }
        
        
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