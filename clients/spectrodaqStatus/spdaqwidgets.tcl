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


package provide spdaqwidgets  1.0

#
#  This package provides widgets that can be used to
#  display the status of spectrodaq based data acquisition systems.
#  Specifically, these widgets are intended to supply diagnostics
#  that even researchers can use to determine which systems/process ids
#  are eating up spectrodaq buffers because some idiot left SpecTcl in
#  a 'stopped' state.
#
#  All widgets live in the namespace: spdaqstatwidgets
#  We export:
#   spdaqwidgets::freepagegraph   - Shows a graph of the free pages
#                                   in a selected server.
#   spdaqwidgets::bufferusage     - Shows, for a given base system (usually
#               The data source), free page graphs for all systems that are
#               clients of the daq, and usage bargraphs by pid in each
#               client so that in the worst case someone will just kill
#               the guy that has all the buffers.  Too much to hope
#               someone will actually bother to figure out what the problem
#               is and fix it.
#
#  Note all widgets require manual updates (via their 'update' methods).
#  - Because updates are expensive to the DAQ system.
#  - Because experience has shown that Tcl/Tk bogs down rather badly if
#    too many timers are created...so this allows you to share timers
#    with some other application specific thingy.
#

#-------------------------------------------------------------------------------
#
#  Initialization
#

package require snit
package require BWidget
package require spdaqstat
package require Tktable

#-------------------------------------------------------------------------------
#
#   The freepagegraph widget uses a BWidget ProgressBar to
#   maintain a graph of the free pages in a spectrodaq server.
# Options:
#   -host    - Which host to monitor, defaults to localhost.
#   -warning - level at which the bar is displayed in yellow.
#              defaults to 33% of total buffers.
#   -alarm   - level at which the bar is displayed in red
#              defaults to 10% of total buffers.
# Methods:
#   update   - Updates the bar graph display position and colors.
#

snit::widget spdaqwidgets::freepagegraph {
    
    option -host    localhost
    option -warning 33.0
    option -alarm   10.0
    
    component bargraph
    delegate option * to bargraph
    
    variable barGraphValue
    
    constructor args {
        install bargraph using ProgressBar $win.progressbar -orient horizontal \
                                                            -type   normal     \
                                                            -variable ${selfns}::barGraphValue
        grid $win.progressbar -sticky nsew
        
        $self configurelist $args
        $self update
    }
    
    # update the widget appearance.
    
    method update {} {
        set max  [spdaqstat::totalpages $options(-host)]
        set free [spdaqstat::freepages  $options(-host)]
        
        
        
        $bargraph configure -maximum $max
        set barGraphValue $free
        
        if {$free < [expr {$options(-alarm)*$max/100.0}]} {
            $bargraph configure -foreground red
            return
        }
        if {$free < [expr {$options(-warning)*$max/100.0}]} {
            $bargraph configure -foreground yellow
            return
        }
        $bargraph configure -foreground green
    }
}
#-------------------------------------------------------------------------------
#
#   The bufferusage widget provides a roughly hierarchical display of the
#   over all buffer usage for a spectrodaq based DAQ system given the data
#   taking host as a starting point.
#   We use the TkTable widget to display all the stuff that needs displaying.
#   The table will look something like this:
#
#   +---------------------+----------------+----------------+
#   |spdaq22.nscl.msu.edu | [***********]  |                |
#   +---------------------+----------------+----------------+
#   |                     |   1234         | [*   ]         |
#   +---------------------+----------------+----------------+
#   |                     |   5432         | [**  ]         |
#   +---------------------+----------------+----------------+
#   |                     |                |                |
#   +---------------------+----------------+----------------+
#   | bad.nscl.msu.edu    | [*          ]  |                |
#   +---------------------+----------------+----------------+
#   |                     | 666            | [******]       |
#   +---------------------+----------------+----------------+
#    ...
#  From the table above, we can see that there's a back presssure
#  issue due to the fact that pid 666 in bad.nscl.msu.edu
#  is holding all the available spectrodaq pages.
#
#
# OPTIONS:
#   -host    - Host that has the data source.  Defaults to localhost.
#              This is used to determine the set of other hosts that are
#              involved by asking it for its links.
#  -warning  - The warning level to be used for the various freepagegraph
#              widgets (one for each node).
#  -alarm    - The alarm level to be used for the various freepagegraph
#              widgets for each node.
#
# METHODS:
#    update - updates the table.  The update is done is a way to
#             minimize changes in the display as pids vanish and are added,
#             and as links are added and vanish as well.
#

snit::widget  spdaqwidgets::bufferusage {
    option -host localhost
    option -warning 33.0
    option -alarm   10.0
    
    variable widgetIndex 0;             # Uniquifies widget names.
    variable usages;                    # array of variables for ProgressBars
    variable nodeInfo;                  # Array of node info lists. Each element
                                        # is indexed by node and has three list elements:
                                        # 0:   Row in the table of the node heading.
                                        # 1:  Total number of buffers this node supports.
                                        # 2:  List of pids of connected clients.
    component   scrolledWindow
    component   table
    
    constructor args {
        install scrolledWindow as ScrolledWindow $win.sw -auto both -scrollbar both
        install table          as table $scrolledWindow.table -cols 3       \
                                                              -rows 1       \
                                                              -colstretchmode all \
                                                              -colwidth 22     \
                                                              -rowheight -25
        $scrolledWindow setwidget $table
        grid $scrolledWindow -sticky nsew
        $self configurelist $args
        $self update
    }
    #
    #  Update the entire widget...first, for each node in the set of links,
    #  if this is a new node, call NewNode, otherwise ExistingNode to process
    # that case.  If a node has doropped, out, then delete the link..
    # otherwise, process the pid buffer usage.
    #
    method update {} {
        set currentLinks [spdaqstat::links $options(-host)]

        
        #  One special case is that if $options(-host) has no local clients,
        #  but is the data source node as it should be, it won't appear
        #  in the list of links, but will have used pages assigned to pids.
        #  so we ensure that $options(-host) is in the currentLinks list:
        
        set sourceHost [spdaqstat::DNSResolve $options(-host)]
        if {[lsearch $currentLinks $sourceHost] == -1} {
            lappend currentLinks $sourceHost
        }
        


        set knownNodes   [array names nodeInfo]
        foreach node $currentLinks {
            if {[lsearch -exact $knownNodes $node] == -1} {
                $self NewNode $node
            } else {
                $self ExistingNode $node
            }
        }
        
        #  Now the reverse, if there are nodes in the knownNodes list
        #  not in currentLinks, they've dropped out; otherwise,
        #  update the barcharts for each pid in the node:
        
        foreach node $knownNodes {
            if {[lsearch -exact $currentLinks $node] == -1} {
                   $self DeleteNode $node                    
            } else {
                set usage [spdaqstat::usagebypid $node]
                $self ProcessPids $node $usage
            }
        }
        
    }
    #--------------------------------------------------------------------------
    # 'private' methods:
    #
    
    # A new node has been detected;
    #  - create the table row.
    #  - create the label and graph widgets, and put them in the table row.
    #  - Create a nodeInfo element for the node.
    #
    method NewNode node {
        set existingNodes [array names nodeInfo]
        set row 0
        foreach n $existingNodes {
            set info $nodeInfo($n)
            set r    [lindex $info 0]
            if {$r >= $row} {
                set row $r
                
                #  Row is the top of the last node...skip past the pids in that node
                #  and leave a blank line too:
                
                set pids [lindex $info 2]
                incr row [llength $pids]
                incr row 2
            }
        }
        if {$row != 0} {
            $table insert rows [expr $row-2] 2
        }
        
        set  bufferCount [spdaqstat::totalpages $node]
        
        set nodeInfo($node) [list $row $bufferCount [list]]
        label $table.label$widgetIndex -text $node
        spdaqwidgets::freepagegraph $table.graph$widgetIndex -host $node   \
                                                       -warning $options(-warning) \
                                                       -alarm   $options(-alarm)
        $table window configure $row,0 -window $table.label$widgetIndex
        $table window configure $row,1 -window $table.graph$widgetIndex
        
        incr widgetIndex
        
    }
    #
    #  Update the bargraph for an existing node.  The nodeInfo array
    #  tells us which row this is in and the table will give us the widget to update.
    #
    method ExistingNode node {
        set info $nodeInfo($node)
        set row [lindex $info 0]
        set widget [$table window cget $row,1 -window]
        
        $widget update
    }
    
    #
    #   If a node has dropped off the list, all of its rows must be deleted,
    #   as well as the blank row at the end of its list.
    #   Deleting the rows will take care of deleting the widgets.
    #   We'll also need to unset the nodeInfo element that corresponds to that
    #   node
    #
    method DeleteNode node {
        set info $nodeInfo($node)
        set row       [lindex $info 0]
        set pidList   [lindex $info 2]
        set rowCount  [expr [llength $pidList] + 2]
        
        $table delete rows $row $rowCount
        unset nodeInfo($node)
        $self UpdateNodeRows $row -$rowCount
    }
    # Update the position of rows that are after a specific row by an
    # appropriate amount:
    #
    method UpdateNodeRows {row difference} {
        foreach node [array names nodeInfo] {
            set current [lindex $nodeInfo($node) 0]
            if {$current >= $row} {
                set current [expr $current + $difference]
                set nodeInfo($node) [lreplace $nodeInfo($node) 0 0 $current]
            }
        }
 
    }
    
    #
    #   Following each node, indented by one cell is a list of PIDs
    #   that are using pages in that node, each with a bar graph that
    #   shows how many pages that PID is using.  This allows one to play
    #   find-the-pagehog followed by the game of kill-the-pagehog
    #

    method ProcessPids {node usage} {
        $self DeleteDeadPids  $node $usage
        $self ProcessLivePids $node $usage
    }
    
    #
    #   Delete any pids that are no longer in the node.
    #   after doing this we need to shift the existing nodes too.
    #
    method DeleteDeadPids  {node usage} {
        
        
        set info $nodeInfo($node)
        set firstRow     [lindex $info 0]
        set totalPages   [lindex $info 1]
        set currentPids  [lindex $info 2]
        
        # construct the list of live pids:
        
        set livePids [list]
        foreach item $usage {
            lappend livePids [lindex $item 0]
        }
        
        # Now make a list of the indices in currentPids for dead pids:
        # The list will be sorted by descending index so deletions will not
        # affect the position of other items in the list.
        set rowsToDelete [list]
        set pidrow       0
        set itemCount    0
        
        foreach pid $currentPids {
            if {[lsearch -exact $livePids $pid] == -1} {
                incr itemCount
                set rowsToDelete [linsert $rowsToDelete 0 $pidrow]
            }
            incr pidrow
        }
        
        # Now get rid of the pids in currentPids and the rows of the table
        # that hold their widgets:
        
        foreach item $rowsToDelete {
            
            set currentPids [lreplace $currentPids $item $item]
            $table delete rows [expr $item + $firstRow +1 ] 1
        }
        
        if {$itemCount  > 0 } {
            $self UpdateNodeRows [expr $firstRow+1] -$itemCount
            
            # Update the node information array element.
            
            set nodeInfo($node) [list $firstRow $totalPages $currentPids]
        }
    }
    #
    #  Assuming we've deleted pids that are not live any more,
    #  We process the pids in the usage list.
    #  For pids already in the current list, just update the
    #  value of their variable.
    #  For pids that are new, add a row.
    #
    method ProcessLivePids {node usage} {
        set info        $nodeInfo($node)
        set firstRow    [lindex $info 0]
        set totalPages  [lindex $info 1]
        set currentPids [lindex $info 2]
        set addedPids   0
        
        foreach item $usage {
            set pid    [lindex $item 0]
            set pages  [lindex $item 1]
            
            set currentIndex [lsearch -exact $currentPids $pid]
            if {$currentIndex == -1} {
                incr addedPids
                lappend currentPids $pid
                set row [expr $firstRow + [llength $currentPids]]
                
                
                set usages(usage$widgetIndex) $pages
                
                label $table.label$widgetIndex -text $pid
                ProgressBar $table.usage$widgetIndex -orient horizontal \
                                                         -type   normal     \
                                                         -variable ${selfns}::usages(usage$widgetIndex) \
                                                         -maximum $totalPages
                $table insert rows $row 1
                $table window configure $row,1 -window $table.label$widgetIndex
                $table window configure $row,2 -window $table.usage$widgetIndex
                
                incr widgetIndex
                
            } else {
                set row [expr $firstRow + $currentIndex +1]
                set widget [$table window cget $row,2 -window]
                set pathList [split $widget .]
                set usages([lindex $pathList end]) $pages
            }
        }
        if {$addedPids > 0} {
            $self UpdateNodeRows [expr $firstRow+1] $addedPids
            set nodeInfo($node) [list $firstRow $totalPages $currentPids]
        }
    }
}
