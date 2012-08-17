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
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321



##
# This is the event orderer main object.
# The event orderer is a Tcl package that the user incorporates into their own script
# via:
#   package require EventBuilder
#
# The package contains the top level event builder snit::type and
# code to properly construct and start it.
# Also provided are EVB namespace procs that allow the user
# to establish an appropriate set of callbacks.
#

package provide EventBuilder 1.0
package require Tk
package require snit
package require EVB::ConnectionManager
package require portAllocator

namespace eval EVB {
    variable eventBuilder
}
##
#  The event builder basically ties everything together in to 
#  a single type that represents the event building application.
#
#
snit::type EVB::EventBuilder {
    component connectionManager

    delegate option -connectcommand to connectionManager
    delegate option -disconnectcommand to connectionManager

    delegate method getConnections to connectionManager

    constructor args {
	$self configurelist $args
	install  connectionManager using  EVB::ConnectionManager %AUTO% -port [$self _GetServerPort]
    }

    #--------------------------------------------------------------------
    # Private methods:
    #

    ##
    # Return the port manager's server port.  This is gotten from the
    # nscldaq port manager.
    #
    # @return int - port number.
    method _GetServerPort {} {
	set pa [portAllocator create %AUTO%]
	set port [$pa allocatePort ORDERER]
	$pa destroy

	return $port
    }

}

#------------------------------------------------------------------------
#
#  Unbound procs that provide a proecedural interface to the event builder.
#
#

##
#  Creates and starts the event builder.
#
proc EVB::Start {} {
    set EVB::eventBuilder [EVB::EventBuilder %AUTO%]
}
##
# Set the connection callback.  The
# connection callback is invoked every time a new client connects to the
# event builder.  The command passed in has the host and description
# string appended to it when called.
#
# @param script - The script to invoke.
#
proc EVB::setConnectionCallback script {
    $EVB::eventBuilder configure -connectcommand [list $script %H %D]
}
##
# Set the disconnect callback.  This is called whenever a data source
# client drops the connection with us.
# The script will have the host and description appended to it.
#
# @param script - the script to invoke.
#
proc EVB::setDisconnectCallback script {
    $EVB::eventBuilder configure -disconnectcommand [list $script %H %D]
}
##
# Get the set of connections. The connection is returned as a list of three
# element sublists.  The elements of each sublist are in-turn,
# - The host of the client.
# - The client description
# - The state of the client connection.
#
#
proc EVB::getConnections {} {
    $EVB::eventBuilder getConnections
}

##
#  EVB::getInputStageStatistics
#
# Get the statistics about the input stage from the event builder
# C++ code.
# This returns directly the output of EVB::inputStats:
#
# \verbatim
#    {oldestTimestamp newestTimestamp totalFragcount queue-statistics}
#  \endverbatim
#     Where:
#     - oldestTimestamp is the timestamp of the oldest queued fragment and
#     - newestTimestamp is similarly the timestamp of the newest queued fragment.
#     - queue-statistics is itself a list of detailed queue statistics. Each element
#       is a sublist containing in order:
#       # id    - the source id that is putting fragments in this queue.
#       # depth - the number of fragments queued in this queue.
#       # oldest- the timestamp of the fragment at the front of the queue.
#                 by the specifications of data sources, this is the
#                oldest queued fragment from that data source.
#
proc EVB::getInputStageStatistics {} {
    return [::EVB::inputStats]
}
