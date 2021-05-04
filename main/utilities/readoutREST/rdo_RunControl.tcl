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
#             Giordano Cerriza
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


##
# @file   rdo_RunControl.tcl
# @brief  Full RunControl UI with manager.
# @author Ron Fox <fox@nscl.msu.edu>
#

if {[array names ::env DAQTCLLIBS] ne ""} {
    lappend auto_path $::env(DAQTCLLIBS)
}

package require ReadoutRESTUI;           # Widgets we'll use.
package require ReadoutRESTClient;       # To get readout states.
package require programstatusclient;     # To query programs from manager.
package require stateclient;             # To query/control manager state.
package require kvclient;                # Title and run# are in kv store.
package require Tk

#-------------------------------------------------------------------------------
#   User interface megawidgets.

##
# @class ReadoutStateTable
#    Provides a table of Readouts and their run/manager states
#
#  This is a ttk::treeview (with vertical scrollbar)
#  run as a table with the following
#  columns:
#     *   name   - program name.
#     *   host   - where it runs.
#     *   state  - internal state.
#     *   running- X if the program is actually running.
# This is a  view.  The model comes both from the manager and,  for
# running programs, the programs themselves
#
# OPTIONS
#    none
# METHODS
#    add    - Adds a name/host
#    delete - Remove a name/host\
#    exists - Query the existence of a program.
#    list   - Return the contents of the table as  a list of dicts.
#    setActive - Set active flag of a name/host.
#    setState  - Set the runstate of a name/host.
#
snit::widgetadaptor ReadoutStateTable {
    variable ids -array [list]
    constructor {args} {
        installhull using ttk::frame
        
        set columns [list name host state running]
        ttk::treeview $win.table -yscrollcommand [list $win.vscroll set] \
            -columns $columns -show headings \
            -displaycolumns $columns         \
            -selectmode none
        
        foreach c $columns h [list Name Host State Running] {
            $win.table heading $c -text $h
        }
        
        ttk::scrollbar $win.vscroll -command [list $win.table yview] \
            -orient vertical
        
        grid $win.table $win.vscroll -sticky nsew
        grid columnconfigure $win 0 -weight 1;          # Only tree expands.
          
        #  There are no options and configureList gets unhappy if
        #  we call it an none are defined.  The code below ensure we
        #  don't forget to configure if options are later added:
        
        if {[array names options] ne [list]} {
            $self configurelist $args
        }
    }
    #--------------------------------------------------------------------------
    # Public methods:
    #
    
    ##
    # add - adds a new item to the end of the table.
    #
    # @param name - program name (contents of name column).
    # @param host - contents of host column.
    # @param state - (optional) value to put in state column.
    # @param active - (optional) value to put in the  Running columns.
    #
    method add {name host {state -} {active -}} {
        set index $name@$host
        if {![$self exists $name $host]} {
            set active [expr {$active ? "X" : ""}]
            set element \
                [$win.table insert {} end -values [list $name $host $state $active]]
            set ids($index) $element
        } else {
            error "This program ($name@$host) already exists."
        }
        
    }
    ##
    # delete
    #    Removes a name/host combination from the table.
    #
    # @param name - name of the program.
    # @param host - host in which it runs.
    #
    method delete {name host} {
        set index $name@$host
        if {[$self exists $name $host]} {
            $win.table delete $ids($index)
            array unset ids $index
        } else {
            error "$index is not in the table."
        }
    }
    ##
    # exists
    #   Determines if a name/host combination is in the table.
    #   The assumption is that the ids array is properly maintained.
    #
    # @param name - name of the program.
    # @param host - host it runs in.
    # @return boolean - true if name@host is in the table.
    #
    method exists {name host} {
        set index $name@$host
        return [expr {[array names ids $index] ne ""}]
    }
    ##
    # list
    #   Lists the contents of the table.  The table contents are list of
    #   dicts...See below.
    # @return list of dicts  - Each dict represents one line of the table and
    #              contains the following key/value pairs:
    #              name  - program name.
    #              host  - host the program runs in.
    #              state - State of the program.
    #              active- Boolean true if the program is active according
    #                      to the table.
    # @note that state can be '-' indicating it's not been actually set.
    #       if that's the case with active, active is assumed to be false.
    # @note that the list should not be assumed to be correct as the underlying
    #       model can change at any time.  It's just the list as of the
    #       last time this view was updated.
    #
    method list {} {
        set result [list]
        foreach index [array names ids] {
            set data [$win.table item $ids($index) -values]
            set name [lindex $data 0]
            set host [lindex $data 1]
            set state [lindex $data 2]
            set active [expr {[lindex $data 3] eq "X"}]
            lappend result [dict create                                     \
                name $name host $host state $state active $active           \
            ]
        }
        
        return $result
    }
    ##
    # setActive
    #    Sets the activity state of a program.
    #
    # @param name   - program name.
    # @param host   - host the program runs on.
    # @param active - boolean flag with new value of the active state of name@host
    #
    method setActive {name host active} {
        set index $name@$host
        if {[$self exists $name $host]}  {
            set entry $ids($index)
            set data [$win.table item $entry -values]
            set active [expr {$active ? "X" : ""}]
            $win.table item $entry -values [lreplace $data 3 3 $active]
        } else {
            error "$index does not exist"
        }
    }
    ##
    # setState
    #   Sets a program's state field.
    #
    # @param name - name of the program.
    # @param host - host of the program.
    # @param state - String to put in the state field.
    #
    method setState {name host state} {
        set index $name@$host
        if {[$self exists $name $host]} {
            set data [$win.table item $ids($index) -values]
            $win.table item $ids($index) -value [                        \
                lreplace $data 2 2 $state                                \
            ]
        } else {
            error "$index does not exist."
        }
    }
}
##
# @class controls
#    Widget to control more than one Readout.  Consists of
#    - ParametersUI  (from ReadoutRESTUI)
#    - Control/State (from ReadouttRESTUI)
#    - Boot/Shutdown control section of the manager itself.
#  Note this is a view, it relies on controller and model logic.
#  Model comes from the manager and the various readout programs.
#
# OPTIONS:
#   -state -  manager state.
#   -readoutstate - List of states from the readouts.
#   -bootcommand  - command to call when we should boot.
#   -shutdowncommand - command to call when we should shutdown.
#
#  Delegated to ReadoutParameters subwidget
#
#   -title        - Current title.
#   -nexttitle    - Next title (being entered).
#   -titlecommand - Called when title should be changed.
#   -run          - Current run number value.
#   -nextrun      - Next run number (being entered).
#   -runcommand   - Command called when run number should be changed.
#   -parameterstate -> -state in ReadoutParameters subwidget.
#
#    Delegated to ReadoutState
#
#    -statecommand  - Called when state change button has been clicked.
#                     (-> -command.)
#
snit::widgetadaptor ReadoutManagerControl {
    component parameters
    component control
    
    # 'native' options:
    
    option -state -default *Unknown* -configuremethod _cfgState
    option -readoutstate -configuremethod _cfgRdoState 
    option -bootcommand [list]
    option -shutdowncommand [list]
    
    # Options delegated ton the parameters widget:
    
    delegate option -title        to parameters
    delegate option -nexttitle    to parameters
    delegate option -run          to parameters
    delegate option -nextrun      to parameters
    delegate option -titlecommand to parameters
    delegate option -runcommand   to parameters
    delegate option -parameterstate to parameters as -state
    
    # Delegated to ReadoutState:
    
    delegate option -statecommand  to control as -command
    
    ## constructor
    
    constructor args {
        installhull using ttk::frame
        
        # Generate the components.
        
        install parameters using ReadoutParameters $win.pars
        install control    using ReadoutState      $win.control
        
        #  Generate widgets that control/reflect the state machine in the manager
        
        ttk::labelframe $win.manager -borderwidth 3 -relief groove -text "Manager"
        ttk::label $win.manager.statelabel -text "State: "
        ttk::label $win.manager.state -textvariable [myvar options(-state)]
        ttk::button $win.manager.boot -text Boot -command _dispatchBoot \
            -state disabled
        ttk::button $win.manager.shutdown -text Shutdown -command _dispatchShutdown \
            -state disabled
        
        grid $win.manager.statelabel $win.manager.state -sticky w
        grid $win.manager.boot $win.manager.shutdown    -sticky w -padx 3
        
        grid $parameters -columnspan 3
        grid $win.manager $control
        
        $self configurelist $args
        
        
    }
    
    #--------------------------------------------------------------------------
    #  Configuration management.
    
    ##
    # _cfgState
    #   Handle the -state option configuration:
    #      If the state is SHUTDOWN - Boot is enabled and  Shutdown not.
    #      otherwise Shutdowns is enabled and Boot is not.
    #
    # @param optname - name of the option being modified.
    # @param value   - new value.
    method _cfgState {optname value} {
        set options($optname) value;     # This updates the state label.
        if {$value eq "SHUTDOWN"} {
            $win.manager.shutdown configure -state disabled
            $win.manager.boot     configure -state normal
        } else {
            $win.manager.shutdown configure -state normal
            $win.manager.boot     configure -state disabled
        }
    }
    
    ##
    # _cfgRdoState
    #    Called when -readoutstate is configured.  We're handed a list of
    #    states, one for each reaodut program being controlled.  If all the
    #    states are the same, we use that state as the resulting state.
    #    if not, we use the state 'inconsistent'
    #
    #    Regardless, options(-readoutstate) is updated so cget is trivial.
    #
    # @param optname - name of the option being configured (-readoutstate).
    # @param value   - new value.
    #
    method _cfgRdoState {optname value} {
        set state [lindex $value 0]
        foreach s [lrange $value 1 end] {
            if {$s ne $state} {
                set state inconsistent
                break
            }
        }
        set options($optname) $state
        $control configure -state $state
    }
        
    
    
    #---------------------------------------------------------------------------
    # Event handling.
    
    ##
    # _dispatch
    #   Dispatch a script.
    #
    # @param opt - option holding the script.
    #
    method _dispatch {opt} {
        set script $options($opt)
        if {$script ne ""} {
            uplevel #0 $script
        }
    }
    
    ##
    # _dispatchBoot
    #  Dispacth the boot button's script:
    #
    method _dispatchBoot {} {
        $self _dispatch -bootcommand
    }
    
    ##
    # _dispatchShutdown
    #
    # Dispatch the shutdown button's script.#
    method _dispatchShutdown {} {
        $self _dispatch -shutdowncommand
    }
}
##
# @class RunControlGUI
#   This is the view for the run control.  It consists of a top part that is a
#    ReadoutManagerControl and a bottom part that is a ttk::notebook.
#    The notebook has one page that is a ReadoutStateTable and then one page
#    for each readout program that contains a ReadoutStatistics entry for
#    each program.
# OPTIONS:
#   All ReadoutManagerControl options - delegated to that component.
# Methods:
#   All ReadoutStateTable methods delegate to that component.
#   updateStatistics - updates statistics page for a specific program/node
#                   if necessary generating a new page for that program/node.
#
snit::widgetadaptor RunControlGUI {
    component controls
    component notebook
    component summary
    
    ## ReadoutManagerControls options:
    
    delegate option -state to controls
    delegate option -readoutstate to controls
    delegate option -bootcommand to controls
    delegate option -shutdowncommand to controls
    delegate option -title to controls
    delegate option -nexttitle to controls
    delegate option -titlecommand to controls
    delegate option -run  to controls
    delegate option -nextrun to controls
    delegate option -runcommand to controls
    delegate option -parameterstate to controls
    delegate option -statecommand to controls
    
    # Summary methods:
    
    delegate method add to summary
    delegate method delete to summary
    delegate method exists to summary
    delegate method list to summary
    delegate method setActive to summary
    delegate method setState to summary
    
    # Manages assigning, keeping track of and 
    
    variable statsIndex 0;     # USed to generate index commands.
    variable statsWidgets -array [list];   # Widgets indexed by program@host
    

    ## Constructor
    
    constructor {args} {
        installhull using ttk::frame
        
        install controls using ReadoutManagerControl $win.controls
        install notebook using ttk::notebook $win.notebook
        install summary using ReadoutStateTable $notebook.summary
        $notebook add $summary -text summary
        
        grid $controls -sticky nsew
        grid $notebook -sticky nsew
        
        
        
        $self configurelist $args
    }
    
    #--------------------------------------------------------------------------
    #  Public methods.
    
    ##
    # updateStatistics
    #    - If necessary a new statistics tab/widget is created.
    #    - The statistics are then passed to that widget to be displayed.
    #
    # @param program - name of the program.
    # @param host    - Host the program runs in.
    # @param stats   - Statistics dict of the program@host.
    #
    method updateStatistics {program host stats} {
        set index $program@$host
        if {[array names statsWidgets $index] eq ""} {
            #
            #   Need to make a new tab:
            #
            set widget  $notebook.stats[incr statsIndex]
            ReadoutStatistics $widget
            $notebook add $widget -text $index
            set statsWidgets($index) $widget
        } else {
            set widget $statsWidgets($index)
        }
        $widget configure -data $stats
    } 
}
#------------------------------------------------------------------------------
# Controller classes.
#

##
# @class SystemStateTracker
#    This class interacts with a manager (model) to maintain the state
#    of the system in a view that supports a -state option.
#    Note that so we don't have a plethora of after's in the event qeueu
#    update is externally called at whatever rate or demand required by the
#    larger application:
#
#  OPTIONS
#    -view    - Widget with a -state option that we'll maintain.
#    -model   - StateClient object that can interact with the manager.
#  METHODS
#    update   - Fetches the state from the model (if possible) and
#               updates the view.
#
snit::type SystemStateTracker {
    option -view
    option -model
    
    constructor {args} {
        $self configurelist $args
    }
    
    #--------------------------------------------------------------------------
    # public methods.
    #
    
    ##
    # update
    #    Attempts to update the view from the model
    #    - currentState is called in the model.
    #    - The returned value is passed into the view.
    #
    # @return integer - 0 on success and 1 if currentState failed.
    #
    method update {} {
        set model $options(-model)
        set view  $options(-view)
        
        if {[catch {$model currentState} state]} {
            return 1;                       # state fetch failed.
        } else {
            $view configure -state $state
        }
    }
}
##
# @class MultiReadoutStateTracker
#    Controller that maintains the readout state for multiple Readout models.
#
# OPTIONS:
#    -view - an object that acepts the -readoutstate option.
#    -models - List of ReadoutRESTClient objects that we'll get our
#              data from.
# METHODS:
#    update - Updates the view from the models.
#
snit::type MultiReadoutStateTracker {
    option -view
    option -models
    
    constructor {args} {
        $self configurelist $args
    }
    
    #--------------------------------------------------------------------------
    # Public methods.
    
    ##
    # update
    #    Attempts to fetch the run state from the models we have.
    #    If any model cannot give us a state (e.g. the Readout is not running)
    #    We'll put in the pseudo state 'inconsistent'  This will mean the
    #    view will represent the state as inconsistent because:
    #    - Either all Readouts are done which results in inconsistent state
    #      due to unanimous consent.
    #    - Some Readouts are down which will cause state disagreement
    #      resulting in a display of inconsistent as the state.
    #
    #
    method update {} {
        set statelist [list]
        foreach model $options(-models) {
            if {[catch {$model getState} state]} {
                lappend statelist inconsistent
            } else {
                lappend statelist $state
            }
        }
        $options(-view) configure -readoutstate $statelist
    }
        
    
}
##
# @class MultiReadoutStatisticsTracker
#    Controller that maintains statistics in a view with multiple readout
#    programs.
# OPTIONS:
#    -view  - A view object that must support the updateStatistics method.
#    -models - list of ReadoutRESTCLient model objects from which the statistics
#              are fetched.
#    -programs - List of program names - corresponding to the models.
# METHODS:
#   udpate  - update the view from the models.
snit::type MultiReadoutStatisticsTracker {
    option -view
    option -models
    option -programs
    
    constructor {args} {
        $self configurelist $args
    }
    #---------------------------------------------------------------------------
    # public methods
    
    ##
    # update
    #    Update the view object from the models.  Note that if a model
    #    errors out (e.g. the Readout isn't running) we ignore that error
    #    at this time.
    #
    method update {} {
        foreach model $options(-models) name $options(-programs) {
            if {![catch {$model getStatistics} stats]} {
                $options(-view) updateStatistics $name [$model cget -host] $stats
            }
        }
    }
}
##
# @class ReadoutParameterTracker
#
#  Manages updates from the title and run key value store items and
#  the uneditable parmaters of a readout.
#
# OPTIONS
#   -view   - View that must support the -title, -run options.
#   -model  - Model which must support a getValue method to retrieve a
#             key value store item.
#
snit::type ReadoutParameterTracker {
    option -view
    option -model
    
    constructor {args} {
        $self configurelist $args
    }
    #---------------------------------------------------------------------------
    # Public methods.
    
    ##
    # update
    #   Updates the view from the model.  Note that if the
    #   model fails (e.g. the manager isn't running we don't update).
    #
    # @return integer - 0 success 1 failure in update.
    #
    method update {} {
        return [catch {
            set model $options(-model)
            set view  $options(-view)
            $view configure -title [$model getValue title]
            $view configure -run   [$model getValue run]
        }]
    }
}
##
# @class SummaryTracker
#    Updates the readout summary view from the program states and models we
#    dynamically generate for those programs.
#
# OPTIONS:
#    -view   - an object that supports add, delete, setActive and setState
#             methods in the same calling convention that ReadoutStateTable does.
#    -model  - an object with a status method that returns lists of dicts that
#              are like the ones a ProgramClient object returns.
#    -readouts - List of program names for programs that are Readouts which also
#              must run their REST interface plugin as we'll be fetching their states.
#
# METHODS:
#    update   - Updates the view from data gotten from the model.
#
snit::type SummaryTracker {
    option -view
    option -model
    option -readouts
    
    constructor {args} {
        $self configurelist $args
    }
    #---------------------------------------------------------------------------
    # Utility methods
    
    ##
    # _filterReadouts
    #   Given the list o dicts containing program states, filters out those
    #   that are not readouts, returning only those that are.
    #   A readout program just needs to match a name in options(-readouts).
    #
    # @param states - list of program status dicts.
    # @return list of program status dicts - the name of each program in the
    #                 returned list is gauranteed to be in options(-readouts)
    #
    method _filterReadouts {states} {
        set result [list]
        foreach status $states {
            if {[dict get $status name] in $options(-readouts)} {
                lappend result $status
            }
        }
        return $result
    }
    ##
    # _getReadoutState
    #   Get the state of the readout running in the specified host/user pair
    #
    # @param host  - host running the program.
    # @param user  - user running the program.
    # @return string - state name.
    # @retval *unresponsive* indicates we could not fetch the state from the
    #                REST server.
    #
    method _getReadoutState {host user} {
        set client [ReadoutRESTClient %AUTO% -user $user -host $host]
        if {[catch {$client getState} state]} {
            set state *unresponsive*;    # Deal with e.g. exit after state get.
        }
        $client destroy
        return $state
    }
    ##
    # _updateReadoutStatus
    #   Updates the status of a program:
    #   - Generate a ReadoutRESTClient and, if possible, fetch the state.
    #   - If the program already exists, just update the state and status.
    #   - If the program does not exist add it.
    #
    # @param program - dict describing the program.
    #
    method _updateReadoutStatus {program} {
        set host [dict get $program host]
        set name [dict get $program name]
        set active [dict get $program active]
        
        set user [$options(-model) cget -user]
        
        if {$active} {
            set state [$self _getReadoutState $host $user]
        } else {
            set state *unresponsive*
        }
        
        set view $options(-view)
        
        if {[$view exists $name $host]} {
            $view setActive $name $host $active
            $view setState  $name $host $state
        } else {
            $view add $name $host $state $active
        }
        
    }
    ##
    # _markDeleted
    #   If a program is not in the list of status items it gets marked deleted
    #   Locate any program in the listed names and remove it.
    #
    # @param name - program name.
    #
    method _markDeleted {name} {
        set model $options(-view)
        set programs [$model list]
        foreach program $programs {
            if {[dict get $program name] eq $name} {
                $model delete [dict get $program name] [dict get $program host]
                break;     # Presumably there's only one.
            }
        }
    }
    ##
    # _deleteMovedHosts
    #    If any programs have moved to a new host, delete the old entry for
    #   them.  It's up to some other bit of code to make the new one.
    #
    # @param names  - Names of programs.
    # @param nodes  - Nodes in which each program is running.
    #
    method _deleteMovedHosts {names nodes} {
        set view $options(-view)
        
        # Note we query the model each time as the outer loop can change the
        #  list iterated over in the inner loop.
        
        foreach name $names node $nodes {
            foreach existing [$view list] {
                set oldName [dict get $existing name]
                set oldHost [dict get $existing host]
                puts "$name@$node : $oldName@$oldHost"
                if {($name eq $oldName) && ($node ne $oldHost)} {
                    $view delete $oldName $oldHost
                    break;                 #Presumably only one match is allowed.
                }
            }
        }
    }

    #--------------------------------------------------------------------------
    # Public methods
    
    ##
    # update
    #   This is actually quite complex considering some of the edge cases:
    #    - The manager does not respond to 'status' no update can occur.
    #    - Some programs have been removed from the manager (vs. the -readouts
    #      list); Those get marked with a state *deleted* and inactive.
    #    - Some program supposedly active does not respond to its state request
    #      That gets marked as '*unresponsive*'
    #    - If a program moves to a different host, we'll make a new one and
    #      delete the old one.
    #
    # @return integer - 0 success -1 failure  because status didn't work.
    #                   all other problems are caught and handled.
    #
    method update {} {
        catch {
            set view $options(-view)
            set programStates [dict get [$options(-model) status] programs]
            set programStates [$self _filterReadouts $programStates]
            
            set existingReadouts [list]
            set readoutHosts     [list]
            foreach program $programStates {
                lappend existingReadouts [dict get $program name]
                lappend readoutHosts     [dict get $program host]
                $self _updateReadoutStatus $program
            }
            #  Mark the programs that disappeared:
            
            foreach program $options(-readouts) {
                if {$program ni $existingReadouts} {
                    $self _markDeleted $program
                }
            }
            #  If hosts changed delete the old ones:
            
            $self _deleteMovedHosts $existingReadouts $readoutHosts
            
        }
    }
}
    

    

#    This is a bit complex bec

#------------------------------------------------------------------------------
#  Utility procs:

##
# usage
#   Output an error message and program usage.
#
# @param msg - the error message.
#
proc usage {msg} {
    puts stderr $msg
    puts stderr "Usage:"
    puts stderr "   \$DAQBIN/rdo_RunControl host user program..."
    puts stderr "Provides a graphical interface to control run state"
    puts stderr "Where:"
    puts stderr "   host  - is the host running the program manager"
    puts stderr "   user  - is the user the program manager is running under"
    puts stderr "   program... is a list of program names that are readouts."
    
    exit -1
    
}

#------------------------------------------------------------------------------
#  Entry
    
