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
# @file ReadoutStatView.tcl
# @brief Provide a view of readout statistics across experiment runs.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide ReadoutStatView 1.0
package require Tk
package require snit


#  This namespace is used to allow us to locate bits and pieces we need
#  like icons 

namespace eval ::ReadoutStatViewNS {
    variable here [file dirname [info script]]
}

##
#   @class ReadoutStatView
#     A view megawidget for Readout statistics.  Each node (host) is a folder.
#     Beneath each host folder is a set of readout programs.  Beneath the
#     readout programs are runs.  Each run has statistics associated with it.
#     The statistics shown are counters and rates for:
#     - triggers.
#     - Events
#     - bytes.
#
#     This is all done with a treeview with scrollbar and columns for:
#     - Tree text which will be one of host, readout program, run number
#     - Properties of stuff which for readout program will be the pid,
#       for runs will be the start time stamp.
#     - timestamp - the time of the most recent statistics entry for a
#       statistics line
#     - Elapsed time - Number of seconds into the run for the most recent
#       statistics entry.
#     - triggers - trigger count/rate.
#     - events   - event count/rates.
#     - bytes    - bytes produced/rates.
#
snit::widgetadaptor ReadoutStatView {
    component tree
    
    
    #  Indexed by host whose values are item ids for the host folder

    variable hosts -array [list]
    
    #  Indexed by host folder id, contents are a list of dicts containitng
    #  -  name    - name of the program.
    #  -  treeid  - Id of the folder for this program in the tree.
    #
    
    variable programs -array [list]
    
    #  Indexed by program treeid, contents are a list of dicts containing
    #   - id  - primary key of a run entry.
    #   - treeid - tree item id of the run.
    #
    variable statItems -array [list]
    
    ##
    #  typeconstructor
    #     Create images for the folders, the programs and the runs:
    #
    typeconstructor {
        image create photo readoutstat_folder -format png \
            -file [file join $::ReadoutStatViewNS::here folder.png]
        image create photo readoutstat_app    -format png \
            -file [file join $::ReadoutStatViewNS::here program.png]
        image create photo readoutstat_run -format png \
            -file [file join $::ReadoutStatViewNS::here run.png]
        set fd [open [file join $::ReadoutStatViewNS::here iconcredit.txt] r]
        set credit [read $fd]
        close $fd
        puts $credit        
    }
    
    ##
    # Constructor
    #   - install the hull.
    #   - create the treemenu and scrollbar
    #   - link the tree menu and scrollbar.
    #   - configure the look of the tree menu.
    #   - paste the GUI elements on the hull (ttk::frame),.
    #
    # @param args are ignored as we have not configuration parameters.
    #
    constructor args {
        installhull using ttk::frame
        
        set colnames [list properties timestamp elapsed triggers events bytes]
        
        # Install and do basic treeview configuration.
        
        install tree using ttk::treeview $win.tree \
            -show [list tree headings] -selectmode none \
            -columns $colnames
        $tree column  #0 -anchor w -stretch 1
        foreach name $colnames {
            $tree heading $name -text $name
            $tree column $name -anchor w -stretch 1
        }
        # Add a vertical scroll bar and hook it to the treeview:
        
        ttk::scrollbar $win.vscroll -orient vertical \
            -command [list $tree yview]
        $tree configure -yscrollcommand [list $win.vscroll set]
        
        # Layout the tree view and it scroll bar:
        
        grid $tree -row 0 -column 0 -sticky nsew
        grid $win.vscroll -row 0 -column 1 -sticky nse
        grid rowconfigure    $win 0 -weight 1
        grid columnconfigure $win 0 -weight 1
        grid columnconfigure $win 1 -weight 0 
        
    }
    #-------------------------------------------------------------------------
    #  public entries:
    

    ##
    # addStatistics
    #  Given statistics from queryReadoutStatistics (might be partial
    #  stats).  Merges that information into the display:
    #  -  New host, program and runs with their statistics are created.
    #  -  Existing statistics lines that are also in the data will update
    #     counters/rates/timestamps
    #
    # @param stats - output from queryReadoutStatistics.
    #
    method addStatistics data {
        dict for {id info} $data {
            set program [lindex $info 0]
            set runsinfo [lindex $info 1]
            
            set programId [$self _addProgram $program]
            $self _addRuns    $programId $runsinfo
        }
    }
    ##
    # clear
    #   Remove all the stuff from the view and also get rid of the book keeping
    #   information  in the variables
    method clear {} {
        set items [$tree children {}];      # Enough to delete the top level children.
        foreach item $items {
            $tree delete $item
        }
        array unset hosts
        array unset programs
        array unset statItems
    }

    #------------------------------------------------------------------------
    # Private methods/
    
    ##
    # _getHostId
    #   Takes a host name and returns the id of the treeview folder that
    #   has information for that host.  If needed a new folder is generated.
    #
    # @param host - Host we're looking for.
    # @return string - the item id of the host folder.
    #
    method _getHostId host {
        if {[array names hosts $host] eq ""} {
            
            # Need to create a new host folder.
            
            set id [$tree insert {} end  -text $host -image readoutstat_folder]
            set hosts($host) $id
        }
        return $hosts($host)
    }
    ##
    # _getProgramId
    #   Given a host id parent folder and a program name, return the
    #   id of the program folder.  Note that if it does not yet exist,
    #   the program folder will be created.
    #
    # @param parent    - tree item id of the host folder. i
    # @param program   - name of the program running in the host folder $parent.
    # @return string   - Id of the readout program folder (may have just been
    #                    created).
    method _getProgramId {parent program} {
        if {[array names programs $parent] ne ""} {
            set programList $programs($parent)
            foreach p $programList {
                if {[dict get $p name] eq $program} {
                    return [dict get $p treeid]
                }
            }
        }
        # Need to create a new item.
        
        set item [$tree insert $parent end                                 \
            -text $program -image readoutstat_app                          \
        ]
        lappend programs($parent) [dict create treeid $item name $program]
        
        return $item
    }
    
    ##
    # Processs a program entry from the database query.
    #  - If necessary a host folder is created.
    #  - If necessary a program 'folder' is created.
    #  - The id of the program 'folder' is returned.
    #
    # @param program  - dict describing the program.
    # @return string  - treeview id of the program item.
    #
    method _addProgram program {
        set host [dict get $program host]
        set name [dict get $program name]
        set id   [dict get $program id]
        
        set hostId [$self _getHostId $host]
        set programId [$self _getProgramId $hostId $name]
        
        return $programId
    }
    ##
    # _getRunId
    #   Given an application's item id and the definition of a run that's
    #   supposed to have been taken by that application, returns the ID
    #   of the run on the tree.  If necessary a new run item is created.
    #
    # @param parent  - Application item id.
    # @param runDef  - Run definition dict.
    # @return string - Id of the run item.
    #
    method _getRunId {parent runDef} {
        
        set runId [dict get $runDef id]
        if {[array names statItems $parent] ne "" } {
            set runs $statItems($parent)
            foreach run $runs {
                if {[dict get $run id] eq $runId} {
                    return [dict get $run treeId]
                }
            }
        }
        # Need to create and record a new entry.
        
        set newId [$tree insert $parent end                                 \
            -text [dict get $runDef runNumber] -image readoutstat_run       \
            -values [list "started: [clock format [dict get $runDef startTime] -format "%D %T"]"] \
        ]
        lappend statItems($parent)                                          \
            [dict create id [dict get $runDef id] id $runId treeId $newId]
        return $newId
        
    }
    ##
    # _getPriorStats
    #    Figure out what to use as the prior statistics set for rate computations.
    #    see _setRunStats for the cases we need to consider.
    #
    # @param item - id of the item whose statistics we are going to compute.
    # @param stats - Statistics list
    # @return dict - statistics dict.
    # @retval [dict create] (empty dict) if there is no good prior to use.
    #
    method _getPriorStats {item stats} {
        
        #  If there are at least two elements in stats use the nxt to last:
        
        if {[llength $stats] > 1} {
            return [lindex $stats end-1]
        }
        # From here on in there's only one stats element.  If there's data
        # in the item, we can use that as the prior data:
        
        set values [$tree item $item -values]
        set priorTs [lindex $values 1]
        if {$priorTs ne ""} {
            set triggers [lindex [lindex $values 2] 0]
            set events   [lindex [lindex $value 3] 0]
            set bytes    [lindex [lindex $values 4] 0]
            set priorstamp [clock scan $priorTs]
            
            return [dict create                                                \
                timestamp $priorstamp triggers $triggers events $events        \
                bytes $bytes                                                   \
            ]
        }
        #  There is no prior information:
        
        return [dict create]
        
        
    }
    ##
    # _computeStatsValues
    #    Given a prior and most recent (last) statistics dict compute the
    #    values elements that are shown in the statistics.  Note that
    #    it is possible to have an empty dict for prior in which case we can't
    #    compute anything for the rates but we can show the absolutes.
    #
    # @param last - most recent (last) values
    # @param prior - Prior values (might be empty.)
    # @return a list of elements that are timestamp, triggers, events and bytes.
    #         where possible this list has rates.  It always has totals.
    #
    method _computeStatValues {last prior} {
        
        # The timestamp and elapsed time don't depend on the cases.
        
        set timestamp [clock format [dict get $last timestamp] -format "%D %T"]
        set elapsed   [dict get $last elapsedTime]
        
        if {$prior eq [dict create]} {
            
            # No prior information.
            
            set fmt "%6u"
            set triggers [format $fmt [dict get $last triggers]]
            set events   [format $fmt [dict get $last events]]
            set bytes    [format $fmt [dict get $last bytes]]
            
        } else {
            # There's prior information we can do rates too.
            
            set fmt "%6u %6.2f/sec"
            
            set dt [expr                    \
                {[dict get $last timestamp] - [dict get $prior timestamp]}]
            
            
            if {$dt > 0} {
                
                set trg [dict get $last triggers]
                set trgRate [expr {($trg - [dict get $prior triggers])/$dt}]
                set triggers [format $fmt $trg $trgRate]
                
                set evt [dict get $last events]
                set evtRate [expr {($evt - [dict get $prior events])/$dt}]
                set events [format $fmt $evt $evtRate]
                
                set byt [dict get $last bytes]
                set bytrate [expr {($byt - [dict get $prior bytes])/$dt}]
                set bytes [format $fmt $byt $bytrate]
            } else {
                # dt ==0 can't compute rates.
                
                set fmt "%6u"
                set triggers [format $fmt [dict get $last triggers]]
                set events   [format $fmt [dict get $last events]]
                set bytes    [format $fmt [dict get $last bytes]]
            }
        }
        set result  [list "$timestamp" "$elapsed" "$triggers" "$events" "$bytes"]
        puts "$result"
        return $result
    }
    ##
    # _setRunStats
    #    Sets the run statistics for a specific run.
    #
    # @param id - the id of the tree element to set.
    # @param stats - list of statistics dicts for the run.
    #
    # @note There are three cases we need to be concerned with:
    #        - If there are at least two statistics entries, the last two are used
    #          to compute the rates.
    #        - If there is only one statistics entry but there's data in the
    #          item, the data in the item is used as the prior values for the
    #          rate computation.
    #        - If there is only one statistics entry, and no data in the item,
    #          we can't compute a rate.
    # @note  because of how the sql query works, there will never be an empty
    #        statistics list, we'll deal with that by ignoring that case.
    #
    method _setRunStats {item stats} {
        set lastStat [lindex $stats end]
        set prior    [$self _getPriorStats $item $stats]
        set statValues [$self _computeStatValues $lastStat $prior]
        set runstart   [lindex [$tree item $item -values] 0]
        $tree item $item -values [list $runstart {*}$statValues]
        
    }
    
    ##
    # _addRuns
    #    Process a set of runs for a program and the
    #    statistics associated with those runs.
    #
    # @param parent - the tree id of the program folder that should enclose
    #                 these runs.
    # @param runs   - Two element list. The first element describes a run.
    #                 the second element describes a set of statistics for the run.
    #
    #  @note The run information is a dict with the following keys:
    #            
    method _addRuns    {parent runs}    {
        
        foreach run $runs {
            set runDef [lindex $run 0]
            set stats [lindex $run 1]
            
            set runId [$self _getRunId $parent $runDef]
            $self _setRunStats $runId $stats
        }
    }
}