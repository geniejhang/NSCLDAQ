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

#
snit::widgetadaptor ::EVB::sourceStatistics {
    
}
#-----------------------------------------------------------------------------
#
#  The overall GUI widget.
#


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