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
# @file RingStatController.tcl
# @brief  Controller (as in MVC) for ring statistics.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide RingStatController 1.0
package require snit
package require SqlWhere

##
# @class RingStatController
#    A controller responsible for maintaining a ring statistics view given a
#    model.
#
# OPTIONS:
#    -view   - View object.
#    -model  - Model object.
#    -period - seconds per update.
#
snit::type RingStatController {
    option -view -default "" -readonly 1
    option -model -default "" -readonly 1
    option -period 2
    
    variable lastId 0;                 # ID of last record we've seen in statistics.
    variable afterid -1;               # Update scheduling id.
    
    ##
    # constructor
    #   Process the configuration options and start updating.
    #
    constructor args {
        $self configurelist $args
        
        $self _update
    }
    ##
    # destructor
    #   Kill off the after:
    #
    destructor {
        after cancel $afterid
    }
    
    #--------------------------------------------------------------------------
    # Private methods
    #
    
    ##
    # _update
    #    Update the view from the model and schedule the next update.
    #
    method _update {} {
        # Reschedule:
        
        after [expr {$options(-period) * 1000}] [mymethod _update]
        
        # Get the new information:
        
        set filter [RelationToNonStringFilter  %AUTO% s.id > $lastId]
        set data [$options(-model) queryStatistics $filter]
        $filter destroy
        
        $options(-view) newStatistics $data
        
        $self _updateLastid $data
    }
    ##
    # _updateLastid
    #    Updates the lastId variable given a set of new data.
    #
    # @param data - query results
    #
    method _updateLastid data {
        dict for {fqnam ringData} $data {
            set clients [lindex $ringData 1]
            foreach client $clients {
                set stats [lindex $client  1]
                foreach stat $stats {
                    set id [dict get $stat id]
                    if {$id > $lastId} {
                        set lastId $id
                    }
                }
            }
        }
    }
}