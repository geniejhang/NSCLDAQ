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
# @file ReadoutStatModel.tcl
# @brief Model for readout statistics.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide ReadoutStatModel 1.0
package require snit

##
# @class ReadoutStatModel
#   Provides a class that can encapsulate queries of the readout statistics
#   database.   This database includes information about readout applications,
#   data taking runs and readout statistics from those runs.
#
#  OPTIONS:
#     -dbcommand - database access command (statusdb object).
#
snit::type ReadoutStatModel {
    option -dbcommand -default [list] -readonly 1
    
    constructor args {
        $self configurelist $args
    }
    
    ##
    #  The main entry point performs the queryReadoutStatistics.
    #
    # @param filter - filter predicate whose toString method will result in an
    #                 sqlite WHERE clause.
    method queryReadoutStatistics filter {
        return [$options(-dbcommand) queryReadoutStatistics $filter]
    }
    ##
    # lastStatisticId
    #    Given output from queyrReadoutStatistics, determines the highest valued
    #    id for a statistics entry.
    #
    # @param data - data from queryReadoutStatistics
    # @return int - New highest value.
    # @retval -1  - indicates there are no statistics values.
    #
    method lastStatisticId data {
        set result -1
        dict for {rdoid info} {
            set programs [lindex $info 1]
            foreach $p $programs {
                set runs [lindex $p 1]
                foreach run $runs {
                    set counters [lindex $run 1]
                    foreach counter $counters {
                        set id [dict get $counter id]
                        if {$id > $result} {
                            set result $id
                        }
                    }
                }
            }
        }
        
        return $result
    }
}