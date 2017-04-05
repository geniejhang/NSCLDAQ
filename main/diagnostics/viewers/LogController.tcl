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
# @file LogController.tcl
# @brief Tie together a Log model and a Log View.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide LogController 1.0
package require Tk
package require snit
package require dialogWrapper

##
# @class LogController
#    links together a log model with a view providing:
#    - Automated updates
#    - Filtering
#    - Support methods for building menus that can give a UI for filtering.
#
snit::type LogController {
    option -model -default [list]
    option -view  -default [list]
    option -filter -default [dict create] -configuremethod _newFilter
    option -period 2
    
    variable lastId -1
    variable afterId -1
    
    
    
    ##
    # constructor
    #   Process the configuration options and start updating.
    constructor args {
        $self configurelist $args
        
        $self _update
    }
    destructor {
        catch {after cancel $afterId}
    }
    
    #------------------------------------------------------------------------
    #  Configuration handlers
    
    ##
    # _newFilter
    #    Clear the view and restart updates immediately.
    #
    method _newFilter {optname value} {
        set options($optname) $value
        set lastId -1
        $options(-view) clear
        after cancel $afterId
        $self _update
    }
    
    ##
    # _update
    #   Update the view by adding new messages to the end of it..
    #   the filters are used to set query filtering in the model:
    #
    method _update {} {
        set idFilter [list id > $lastId]
        set filter   [list $idFilter]
        set filterDict $options(-filter)
        
        if {[dict exists $filterDict severities]} {
            lappend filter [dict get $filterDict severities]
        }
        
        foreach field [list application source] {
            if {[dict exist $filterDict $field]} {
                lappend filter [dict get $filterDict $field]
            }
        }

        set newMessages [$options(-model) get $filter]
        if {[llength $newMessages] > 0} {
            $options(-view) load $newMessages
            set lastId [dict get [lindex $newMessages end] id]
        }    
        
        set afterId [after [expr {$options(-period) * 1000}] [mymethod _update]]
    }
    

    #--------------------------------------------------------------------------
    #  Menu support:
    #
    
    variable DEBUG 1
    variable INFO  1
    variable WARNING 1
    variable SEVERE 1
    variable DEFECT 1
    variable severityList [list DEBUG INFO WARNING SEVERE DEFECT]
    
    
    ##
    # _updateSeverityFilter
    #    Called when the severity filter has changed:
    #
    method _updateSeverityFilter {} {
        set sevSet [list]
        foreach sev $severityList {
            puts "$sev [set $sev]"
            if {[set $sev]} {
                puts "Appending $sev"
                lappend sevSet '$sev'
            }
        }
        puts "set: $sevSet"
        set inClause "severity IN ([join $sevSet ,])"
        puts "Clause: $inClause"
        
        dict set options(-filter) severities $inClause
        puts "$options(-filter)"
        $self configure -filter $options(-filter);  # force repopulation.
    }
    ##
    # _setTextFilter
    #    Prompts for a filter for a textual field.
    #    Saves it in the -filter options.
    #
    # @param which - field for which we're prompting a filter value:
    #
    method _setTextFilter which {
        #
        #  Create a dialog that prompts for the field value:
        #
        toplevel .prompt
        set d [DialogWrapper .prompt.dialog]
        set carea [$d controlarea]
        set f [ttk::frame $carea.frame]
        ttk::label $f.label -text [string totitle "$which "]
        set value [ttk::entry $f.entry]
        grid $f.label $value
        $d configure -form $f
        pack $d
        
        set result [$d modal]
        
        if {$result eq "Ok"} {
            set requested [$value get]
            
            
            #  Note that an empty string removes the filter:
            
            if {$requested eq ""} {
                dict unset options(-filter) $which
            } else {
                dict set options(-filter) $which [list $which = '$requested']
            }
            $self configure -filter $options(-filter)
        }
        destroy .prompt;              # Don't need this any more.
    }
    ##
    # _clearFilters
    #    Clears the set of filters applied to the log display:
    #
    method _clearFilters {} {
        $self configure -filter [dict create]
    }
    ##
    # Fill a menu with filtering stuff.
    #
    # @param menu - the menu being stocked.
    #
    method fillFilterMenu menu {
        menu $menu.filter
        $menu add cascade -label Filter -menu $menu.filter
        $menu.filter add command -label {Severities:}
        foreach sev $severityList {
            $menu.filter add checkbutton \
                -onvalue 1 -offvalue 0 -variable [myvar $sev] -label $sev    \
                -command [mymethod _updateSeverityFilter]
            
        }
        $menu.filter add separator
        $menu.filter add command  \
            -label application... -command [mymethod _setTextFilter application]
        $menu.filter add command  \
            -label Source... -command [mymethod _setTextFilter source]
        $menu.filter add separator
        $menu.filter add command -label Clear -command [mymethod _clearFilters]
    }
}