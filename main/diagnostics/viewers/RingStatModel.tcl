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
# @file RingStatModel.tcl
# @brief Provide access to the status database specialized to ringbuffers.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide RingStatModel 1.0
package require statusMessage
package require snit
package require SqlWhere

##
# @class RingStatModel
#   Model in the MVC sense of the word for the ring statistics database.
#   The code provides access to raw queries but also some pre-packaged
#   queries that are worth while.
#
snit::type RingStateModel {
    option -dbcommand -default [list] -readonly 1
    
    
    ##
    # constructor
    #   Only purpose in life is to squirrel away the -dbcommand option.
    constructor args {
        $self configurelist $args
    }
    #--------------------------------------------------------------------------
    #  Expose raw queries:
    
    ##
    # listRings
    #    List rings with a filter.
    #
    # @param filter  - filter that describes the desired query results.
    # @param list    - see listRings in the statusdb command of the statusMessage
    #                  package.
    #
    method listRings filter {
        set cmd $options(-dbcommand)
        
        return [$cmd listRings $filter]
    }
    ##
    #  listRingsAndClients
    #     Lists ring buffers and the clients (producers and consumers)
    #     attached to them.
    #
    # @param filter - query filter parameters.
    # @return dict  - See the listRingsAndClients in the statusdb command of the
    #                  statusMessage package.
    #
    method listRingsAndClients filter {
        set cmd $options(-dbcommand)
        return [$cmd listRingsAndClients $filter]
    }
    ##
    # queryRingStatistics
    #    List rings, clients and statistics data.
    #
    #  @param filter  - Query filter.
    #  @return dict   - See queryRingStatstics in the statusdb command of the
    #                   statusMessage package.
    #
    method queryRingStatistics filter {
        set cmd $options(-dbcommand)
        return [$cmd queryRingStatistics $filter]
    }
    
    ##
    # getNewStatistics
    #   Return the statistics that are more recent than the input set.
    #
    # @param prior - a prior set of statistics.
    # @return dict - same form as queryRingStatistics.
    #
    method getNewStatistics prior {
        set lastId [getLastId $prior]
        set filter [RelationToNonStringFilter %AUTO% s.id > $lastId]
    
        set result [$self queryRingStatistics $filter]
        $filter destroy
        
        return $result
    }
    #------------------------------------------------------------------------
    #  Internal procs.
    
    ##
    # getLastId
    #   Given a result set from queryRingStatistics, returns the largest
    #   s.id in the records received.  If there are no statistics entries,
    #   -1 is returned.
    #
    # @return int  - largest id gotten.
    # @retval -1   - No statistics entries in the result.
    #
    proc getLastId result {
        set last -1
        dict for {key ringclients} $result {
            foreach client [lindex $ringclients 1] {
                foreach statdict [lindex $client 1] {
                    set id [dict get $statdict id]
                    if {$id > $last} {
                        set last $id
                    }
                }
            }
        }
            
        
        
        return $last
    }
}

