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
# @file LogModel.tcl
# @brief Model for log messages.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide LogModel 1.0


##
# This package provides a model for {set log messages.
# It maintains a database of LOG messages.  The log message database is
# created as follows:
#   *  If -file is a file then the database is created in the file -- which
#      can exit in order to support a persistent database of log messages.
#   *  If -file is blank, the database is created in a temporary file
#      deleted on destruction of this type
#   *  IF -file is :memory: the datagse is in memory.
#
#

package require snit
package require statusMessage
package require SqlWhere
##
# @class LogModel
#    Snit class that encapsulates the log file.
#

snit::type LogModel {
    option -file -default [list]  -configuremethod _configureFile
    variable dbCommand ""
    
    constructor args {
        $self configurelist $args
    }
    
    destructor {
        if {$dbCommand ne ""} {
            statusdb destroy $dbCommand
        }
    }
    
    ##
    # Configuration handlers:
    
    ##
    # _configureFile
    #    Called when -file is reconfigured.  If dbCommand is defined we need
    #    to destroy it and then make a new one.
    #
    # @param name - option name (-file)
    # @param value - New filename.
    #
    method _configureFile {name value} {
        if {$dbCommand ne ""} {
            statusdb destroy $dbCommand
            set dbCommand ""
        }
        set dbCommand [statusdb create $value readonly]
        set options($name) $value
    }
    
    ##
    #  API:
    #
    

    ##
    # get
    #   Get log records from the database in accordance with the filter
    #   criteria.
    #
    # @param filter - describes any filter criteria as a list of sublists;  An empty
    #                 filter implies no filtering.  Each sublist contains a
    #                 fieldname followed by a relational operator followed by a value.
    #                 
    #                 
    # @return list of dicts with the key value pairs (note these are field names):
    #         *  severity - message severity
    #         *  application - Application name
    #         *  source      - data source.
    #         *  timestamp   - Timestamp.
    #         *  message     - Message string.
    #
    method get {{filter {}} } {
        
        # Construct the query filter:  constructedFilters keeps track of the
        # filters that must be destroyed.
        
        set queryFilter  [RelationToNonStringFilter %AUTO% 1 = 1]
        set constructedFilters [list $queryFilter]
        
        if {[llength $filter] > 0} {
            $queryFilter destroy
            set queryFilter [AndFilter %AUTO%]
            set constructedFilters [list $queryFilter]
            
            foreach condition $filter {
                set c [RelationToNonStringFilter %AUTO%             \
                    [lindex $condition 0] [lindex $condition 1]       \
                    [lindex $condition 2]                            \
                ]
                lappend constructedFilters $c
                $queryFilter addClause $c
            }
        }

        #  Get the raw data and destroy the filters we made:
        
        set rawResult [$dbCommand queryLogMessages $queryFilter]
        foreach f $constructedFilters {
            $f destroy
        }
        
        #  The raw data is a superset of what we need to return so:
        
        return $rawResult
        
    }
    ##
    #  Return the number of messages that are in the database.
    #
    # @return integer - number of messages in the database.
    #
    method count {} {
        return [llength [$dbCommand queryLogMessages]]    
    }
 
}   

