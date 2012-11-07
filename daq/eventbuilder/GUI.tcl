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
#        

##
# @file GUI.tcl
# @brief Provides the top level GUI widgets and other related stuff.

#  Add here to the package include path if it's not there already:

set here [file dirname [info script]]
if {[lsearch $::auto_path $here] == -1} {
    lappend ::auto_path $here
}

# Package provides and requires:

package provide EVB::GUI 1.0
package require Tk
package require snit

package require EVB::inputStatistics
package require EVB::outputStatistics
package require EVB::barriers
package require EVB::connectionList
package require EVB::Late

# Establish the EVB namespace into which we're going to squeeze:

namespace eval ::EVB {
    
}

#-----------------------------------------------------------------------------
#
#  Widgets for each tab
#

##
# @class ::EVB::summary
#
#   This class is a widget that displays the top level summary counters.
#   Specifically input statistics, timestmpa information, output statistices
#   Barrier summary and the connection list are displayed as shown in LAYOUT
#
#    This is a widgetadaptor in order to allow a ttk::frame to be used as the
#    hull.
#
# LAYOUT:
#
#    Note in most cases the blocks shown below are  themselves megawidgets.
#
#
#   +------------------------------------------------------+
#   |inputstatistics::           | Output summary          |
#   |     summaryDisplay         +-------------------------+
#   |                            |  Barrier summary        |
#   +----------------------------+-------------------------+
#   |     Connection list/status                           |
#   +------------------------------------------------------+
#
# @note the connection list/statis widget is fully autonomous.
# 
# OPTIONS
#
#   Delegated to the input summary:
#
#   - -infragments    - total number of in flight fragments.
#   - -oldest       - Oldest in-flight timestamp.
#   - -newest       - Newest in-flight timestamp.
#   - -deepestid    - If of deepest input queue.
#   - -deepestdepth - Depth of deepest input queue.
#
#   Delegated to the output summary:
#
#   - -outfragments    - Number of fragments that have been output.
#   - -hottestoutid    - Id from which the most fragments have been written.

#   - -hottestoutcount - Number of fragments written from hottestoutid.
#   - -coldestoutid    - Id from which the fewest fragments have been written.
#   - -coldestoutcount - Number of fragments written from coldestoutid.
#
#   Delegated to the barrier summary:
#
#   - -completebarriers   - Number of complete barriers seen.
#   - -incompletebarriers - Number of incomplete barriers seen
#   - -mixedbarriers      - Number of barriers with heterogeneous counts.
#  
#
snit::widgetadaptor ::EVB::summary {
    component inputSummary
    component outputSummary
    component barrierSummary
    component connectionList
    
    # Delegate the input summary options:
    
    delegate option -infragments  to inputSummary  as -fragments
    delegate option -oldest       to inputSummary
    delegate option -newest       to inputSummary
    delegate option -deepestid    to inputSummary
    delegate option -deepestdepth to inputSummary
    
    # Delegate output summary options:
    
    delegate option -outfragments    to outputSummary as -fragments
    delegate option -hottestoutid    to outputSummary as -hottestid
    delegate option -hottestoutcount to outputSummary as -hottestcount
    delegate option -coldestoutid    to outputSummary as -coldestid
    delegate option -coldestoutcount to outputSummary as -coldestcount
   
    # Delegate barrier summary
    
    delegate option -completebarriers   to barrierSummary as -completecount
    delegate option -incompletebarriers to barrierSummary as -incompletecount
    delegate option -mixedbarriers      to barrierSummary as -heterogenouscount
    
    
    
    ##
    # constructor
    #
    # @param args - configuration option/value pairs.
    #
    constructor args {
        
        # Install the hull and its components.
        
        installhull using ttk::frame
        install     inputSummary using ::EVB::inputStatistics::summaryDisplay \
            $win.insummary -text {Input Statistics}
        
        install     outputSummary using ::EVB::outputSummary \
            $win.outsummary -text {Output Statistics}
        
        install     barrierSummary using EVB::BarrierStats::Summary \
            $win.barriers -text {Barrier Statistics}
        
        install connectionList     using ::EVB::connectionList \
            $win.connections -text {Connections}
        
        # layout the widgets:
        
        grid $inputSummary   -row 0 -column 0 -rowspan 2 -sticky nsew
        grid $outputSummary  -row 0 -column 1 -sticky nsew
        grid $barrierSummary -row 1 -column 1 -sticky nsew
        grid $connectionList -row 2 -column 0 -columnspan 2 -sticky nsew
        
        
        $self configurelist $args
    }
}
##
# @class EVB::sourceStatistics
#
#  Displays detailed source and barrier statistics by source.
#  See LAYOUT below for details.  This is a snit::widgetadaptor to allow
#  ttk::frame to be the hull without any tomfoolery with the snit valid hull
#  list.
#
# LAYOUT:
#   +------------------------------------------------------------------------+
#   | Per queue source statistics      | Per queue barrier statistics        | 
#   | (EVB::inputStatstics::queueStats | EVB::BarrierStats::queueBarriers    |
#   +------------------------------------------------------------------------+
#
# OPTIONS:
#
# METHODS:
#    getQueueStatistics     - Returns the queue source statistics widget
#    getBarrierStatistics   - Returns the barrier statistics widget.
#
snit::widgetadaptor ::EVB::sourceStatistics {
    component queuestats
    component barrierstats
    component connections
    delegate option * to hull
    
    ##
    # constructor
    #   Build the widgets lay them out and configure the hull
    #
    # @param args - configuration option / value pairs.
    #
    constructor args {
        
        # The hull will be a ttk::frame.
        
        installhull using ttk::labelframe
        
        # Create the components:
        
        install queuestats using EVB::inputStatistics::queueStats $win.queue \
            -width 250 -height 300 -title {Queue statistics}
        
        install barrierstats using EVB::BarrierStats::queueBarriers $win.barrier \
            -width 250 -height 300 -title {Barrier statistics}
        
        install connections using EVB::connectionList $win.connections \
            -text "Connected clients"
        
        # Layout the components:
        
        grid $queuestats $barrierstats -sticky nsew
        grid $connections -row 1 -column 0 -rowspan 2 -sticky ew
        
        # process the options:
        
        $self configurelist $args
    }
    
    ##
    # getQueueStatistics
    #
    # Return the queuestats component.  This allows clients to maintain
    # its appearance/value.
    #
    # @return widget - the queuestats component object.
    method getQueueStatistics {} {
        return $queuestats
    }
    ##
    # getBarrierStatistics
    #
    #  Return the barrierstats component object.  This allows
    #  clients to maintain its appearance/values.
    #
    # @return widget - the barrierstats component object.
    #
    method getBarrierStatistics {} {
        return $barrierstats
    }
    
}
##
# @class EVB::barrierStatistics
#
#   Widget that displays barrier statistics.
#
#  LAYOUT:
#   +-------------------------------------------------+
#   |      EVB::BarrierStats::Summary                 |
#   +-------------------------------------------------+
#   |      EVB::BarrierStats::queueBarriers           |
#   +-------------------------------------------------+
#
# OPTIONS:
#     - -incompletecount - Sets the number of incomplete
#                        barriers in the summary widget.
#     - -completecount   - Sets the number of complete barriers in the summary
#                        widget.
#     - -heterogenouscount - Sets the number of complete barriers that were
#                        heterogenous in type.
#
# METHODS:
#    - setStatistic sourceid barriertype count -
#                         sets the barrier statistics for a source id.
#    - clear             - Clears the counters for each source.
#    - reset             - Removes all source statistics.
#
snit::widgetadaptor EVB::barrierStatistics {
    component summary
    component perQueue
    
    delegate option -text to hull
    delegate option * to summary
    delegate method * to perQueue
    
    ##
    # constructor
    #
    #   Create the hull as a lableframe to allow client titling.
    #   Then create the components and lay everything out.
    #
    constructor args {
        installhull using ttk::labelframe
        
        install  summary using EVB::BarrierStats::Summary $win.summary
        install perQueue using EVB::BarrierStats::queueBarriers  $win.perqueue
        
        grid $summary -sticky new
        grid $perQueue -sticky news
            
        $self configurelist $args
    }
}
##
# @class EVB::errorStatistics
#
# Top level page for displaying error statistics (hopefully this page is
# very boring in actual run-time).
#
# LAYOUT:
#    +--------------------------------------------------------+
#    | EVB::lateFragments   | EVB::BarrierStats::incomplete   |
#    +--------------------------------------------------------+
#
# METHODS:
#   - getLateStatistics       - Returns the EVB::lateFragments widget.
#   - getIncompleteStatistics - Returns the EVB::BarrierStats::incomplete widget.
#
#
snit::widgetadaptor EVB::errorStatistics {
    component lateStats
    component incompleteStats
    
    delegate option * to hull
    
    ##
    # constructor
    #
    #   Install the hull, the two components and lay them out.
    #
    constructor args {
        installhull using ttk::labelframe
        
        install lateStats using       EVB::lateFragments            $win.late
        install incompleteStats using EVB::BarrierStats::incomplete $win.inc
        
        grid $win.late $win.inc
        
        $self configurelist $args
    }
    #-----------------------------------------------------------------------
    # Public methods.
    #
    
    ##
    # getLateStatistics
    #
    #  Return the late statistics widget so that it can be updated.
    #
    method getLateStatistics {} {
        return $lateStats
    }
    ##
    # getIncompleteStatistics
    #
    #  Return the incomplete barrier statistics widget.
    #
    method getIncompleteStatistics {} {
        return $incompleteStats
    }
}

#-----------------------------------------------------------------------------
#
# @class EVB::statusNotebook
#
#  The overall GUI widget. This is just a ttk::notebook with
#  tabs for each of the top level widgets.'
#
#
# METHODS:
#   - getSummaryStats - Returns the summary widget so that it can be updated.
#   - getSourceStats  - Returns the source statistics widget so that it can be
#                     updated.
#   - getBarrierStats - Returns the barrier statistics widget so that it can be
#                     updated.
#   - getErrorStats   - Get the error statistics widget
#  ..
snit::widgetadaptor EVB::statusNotebook {
    component summaryStats
    component sourceStats
    component barrierStats
    component errorStats
    
    delegate option * to hull
    delegate method * to hull
    
    ##
    # constructor
    #
    #  Install the hull which gets all of the options (to configure
    #  child widgets get the widget identifier of the child [see METHODS] ).
    #
    #  Install the componen widgets as pages in the notebook.
    #  configure.
    #
    constructor args {
        # Install the hull as a tabbed notebook.
        
        installhull using ttk::notebook 
        
        # Install the components as pages:
        
        install summaryStats using EVB::summary $win.summary
        $hull add $summaryStats -text Summary
        
        install sourceStats using EVB::sourceStatistics $win.sources
        $hull add $sourceStats -text {Source Statistics}
        
        install barrierStats using EVB::barrierStatistics $win.barrier
        $hull add $barrierStats -text {Barriers}
        
        install errorStats using EVB::errorStatistics  $win.errors
        $hull add $errorStats -text {Errors}
        
        $self configurelist $args
    }
    #------------------------------------------------------------------------
    # Public methods:
    
    ##
    # getSummaryStats
    #   Return the widget that manages the summary statistics.
    #
    method getSummaryStats {} {
        return $summaryStats
    }
    ##
    # getSourceStats
    #  Return the widget that manages the source statistics.
    #
    method getSourceStats {} {
        return $sourceStats
    }
    ##
    # getBarrierStats
    #   Return the widget that manages the barrier statistics.
    #
    method getBarrierStats {} {
        return $barrierStats
    }
    ##
    # getErrorStats
    #
    #   Get the error Statistics widget.
    #
    method getErrorStats {} {
        return $errorStats
    }
}


#-----------------------------------------------------------------------------
#
# Stuff to maintain the status of the UI.
#



## Testing stubs

proc ::EVB::getConnections {} {
    return [list \
            [list localhost "Dummy connection" ACTIVE No] \
            [list remote.host.here "Second connection" FORMING "Yes"] \
    ]
}