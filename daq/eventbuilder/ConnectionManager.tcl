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


#--------------------------------------------------------------------------------
#
#  Event builder connection manager:
# 
#  This must be run in an event loop such as in a vwait or in 
#  tk
#
#  Two snit objects are defined here:
# 
#   Connection - The state machine that makes up an actual connection.
#                What a connection does with input depends on
#                the state a connection is in.
#                Connections are created in the 'FORMING' state and have the following 
#                additional states:
#                - 'ACTIVE'  - The connection is in a state where it can handle data'
#                              from a data source
#                - 'CLOSED'  - The connection closed normally (as per protocol).
#                - 'LOST'    - The connection was lost without normal close protocol.
#                - 'ERROR'   - The connection saw a protocol error and was dropped from
#                              our end.
#
#  ConnectionManager - Handles TCP/IP level connection requests creating connections
#                      as needed and killing them as well.
#

package require snit
package provide EVB::ConnectionManager 1.0

namespace eval EVB {}

##
# Connection object
#
# OPTIONS
#   -state - Current socket state (see above).  This is readonly.
#   -clientaddr - Client TCP/IP address (this is readonly).
#   -description - Client description as provided in the the connection negotiation.
#   -socket    - Socket connected to client.
#   -clientaddr - Client TCP/IP address.
#   -disconnectcommand - Script  to call on disconnect.
#
# METHODS
#   Only construction and destrution are are public.
#
snit::type EVB::Connection {
    option -state -default FORMING -readonly yes
    option -description -default "" -readonly yes

    option -socket;		#  The socket connected to client.
    option -clientaddr -default "not-set"
    option -disconnectcommand [list]

    constructor args {
	$self configurelist $args

	fconfigure $options(-socket) -blocking 0 -buffering none -translation {binary lf}

	$self _Expecting _Connect FORMING; # We are now expecting a CONNECT command.
    }
    
    #------------------------------------------------------------------------------
    # Private methods:


    ##
    # Reads a counted string from the socket turning it into a 
    # Tcl string.
    #
    # @param socket
    # @return string
    # @note If a read on the socket fails an error is thrown.
    #
    method _ReadCountedString socket {
	#
	# Note that [string length] is documented to
	# return the length of a byte array object if that's what it's
	# applied to so it should work below:
	#
	set count [read $socket 4]; # uint32_t.
	if {[string length $headerCount] != 4} {
	    error "read of string length failed"
	}
	set stringBytes [read $socket $count]
	if {[string length $stringBytes] != 4} {
	    error "read of string itself failed"
	}
	#
	# Decode the string as a tcl string via binary scan
	#
	binary scan $stringBytes a$count result

	return $result

    }
    ##
    # Read/Decode a message whose header and body payloads are ASCII Strings.
    #
    # @param socket - The socket open on the client.
    #
    # @return a 2 element list.  [lindex .. 0] is the header text.
    #         [lindex .... 1] is the body text.
    #
    method _ReadTextMessage socket {

	set header [$self _ReadCountedString]
	set body   [$self _ReadCountedString]

	return [list $header $body]

    }

    ##
    #   Indicate which method shouild be associated with socket readability as part
    #   of a state transition.
    #
    # @param method - the new method that shouild handle readability.
    # @param newState - The new state value.
    #
    method _Expecting {method newState} {
	fileevent $options(-socket) readable [mymethod $method $options(-socket)] 
	set options(-state) $newState
    }
    ##
    # Called to close the connection
    #  - The new state is set.
    #  - The socket is closed.
    #  - The callback, if set, is called.
    #
    # @param newState - New state to set.
    method _Close {newState} {
	set options(-state) $newState
	close $options(-socket)

	if {$options(-disconnectcommand) ne [list]} {
	    uplevel #0 $options(-disconnectcommand)
	}
    }


    ##
    #  Input handler for when we are expecting a CONNECT message.
    #  when one is properly handled, we transition to the ACTIVE state.
    #
    # @param socket - the socket on which we are going to get our
    #                 message.
    # @note:  If the message given is not a valid CONNECT,
    #         We just transition to ERROR, and close out
    #
    method _Connect socket {
	
	$

	if {[catch {$self _ReadTextMessage $socket} msg ]} {
	    $self _Close ERROR
	}

	set header [lindex $msg 0]
	set body   [lindex $msg 1]

	# Must be a CONNECT message with a non-zero body.
	#

	if {$header ne "CONNECT"} {
	    puts $socket "ERROR {Expected CONNECT}"
	    $self _Close ERROR
	}

	if {[string length $body] == 0} {
	    puts $socket "ERROR {Empty Body}"
	    $self _Close ERROR
	}

	# Save the description and transition to the active state:
        # Register ourself with the event builder core

	set options(-description) $body
	# RegisterStub   TODO:
	$self _Expecting _Fragments ACTIVE

    }
    #
    # Stub for fragments handler.
    #
    method _Fragments socket {}
}


##
#  Connection manager object.  Creating one of these
#  creates a connection manager on the specified port
#  which will create Connection objects each time a connection arrives.
#  and will destroy them as they die.
#
# OPTIONS
#   -port - port on which we are listening for connections.
#
# METHODS:
#
snit::type EVB::ConnectionManager {
    option -port;		# Port on which we listen for connections.

    variable serverSocket;	# Socket run by us.
    variable connections [list]; # List of Connection objects.

    constructor args {
	$self configurelist $args; # To get the port.

	socket -server [mymethod _NewConnection] $options(-port)
    }

    #-----------------------------------------------------------------
    #  Private methods.

    ##
    # Client disconnect is just removing it from the list of connections:
    #
    # @param object - connection object name.
    # @note  Since it should not be possible for an object to disconnect that
    #        is not in our list, we toss an error if we are handed one.
    #
    method _DisconnectClient object {
	set objectIndex [lsearch -exact $connections $object]
	if {$objectIndex != -1} {
	    lreplace connections $objectIndex $objectIndex
	} else {
	    error "BUG - $object not in ConnectionManager connection list (Disconnect)"
	}
    }


    ##
    #  React to client connections:
    #  - A new Connection object is created to interact witht the client.
    #  - The object is aded to the connections list.
    #  - The object's -disconnectcommand is configured to invoke
    #    _DisconnectClient
    #
    #  @param sock - socket connected to the client
    #  @param client - Client IP addrerss.
    #  @param cport - client port (we could care less).
    #
    method _NewConnnection {sock client cport} {
	set connection [EVB::Connection %AUTO% -socket $sock  -clientaddr $client]
	lappend connections $connection
	$connection configure -disconnectcommand [mymethod _DisconnectClient $connection]
    }
}