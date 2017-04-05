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
# @file ReadoutStatController.tcl
# @brief Controller for readout statistics in the MVC sense of the word.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide ReadoutStatController 1.0
package require snit
package require SqlWhere


##
# @class ReadoutStatController
#     Ties together a readout  statistics model and a readout statistics
#     view periodically updating the contents of the view from the model
#     and reacting to any events we care abot.
#
# OPTIONS:
#   - model - the model object.
#   - view  - The view object.
#   - period  - Seconds between updates.
#
snit::type ReadoutStatController {
    option -model -default [list]
    option -view  -default [list]
    option -period -default 5
    option -cleanup -default 5
    
    
    variable lastId 0;                # Ask for data with s.id > than this.
    variable afterId -1;              # cancel this on exit.
    
    ##
    # constructor
    #    Process the options and start the updater.
    constructor args {
        $self configurelist $args
        
        $self _updateView
    }
    ##
    # destructor - cancel the timer.
    #
    destructor  {
        after cancel $afterId
    }
    
    #--------------------------------------------------------------------------
    # Private methods.
    
    ##
    # update
    #   - Schedule the next update.
    #   - Get data from the model.
    #   - Update the view.
    method _updateView {} {
        set afterId [after [expr {$options(-period)*1000}] [mymethod _updateView] ]
        
        set filter [RelationToNonStringFilter %AUTO% s.id > $lastId]
        set stats  [$options(-model) queryReadoutStatistics $filter]
        $filter destroy
        set lastId [$options(-model) lastStatisticId $stats]
        
        $options(-view) addStatistics $stats
        $options(-view) cleanupOld $options(-cleanup)
    }
}