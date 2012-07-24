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
package require EVB::CallbackManager
package require EvbOrderer;	# C/C++ orderer.

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
	fconfigure $options(-socket) -blocking 1 -buffering none -translation {binary lf}

	$self _Expecting _Connect FORMING; # We are now expecting a CONNECT command.
    }
    destructor {
	if {$options(-socket) != -1} {
	    catch {$self _Close 'CLOSED'}
	}

    }
    
    #------------------------------------------------------------------------------
    # Private methods:

    ##
    # Read binary data from the socket with a leading uint32_t that contains
    # the number of bytes in the data:
    #
    # @param socket - The socket to read from.
    #
    # @return byte array containing the data read from the socket.
    #
    method _ReadBinaryData socket {
	set count [read $socket 4]; # Read the count.

	if {[string length $count] != 4} {
	    error "Read of byte array length failed"
	}
	binary scan $count i1 size

	if {$size > 0} {
	    return [read $socket $size]
	} else {
	    return ""
	}
    }

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
	if {[string length $count] != 4} {
	    error "read of string length failed"
	}
	binary scan $count i1 size
	set stringBytes [read $socket $size]
	if {[string length $stringBytes] != $size} {
	    error "read of string itself failed"
	}
	#
	# Decode the string as a tcl string via binary scan
	#
	binary scan $stringBytes a$size result
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

	set header [$self _ReadCountedString $socket]
	set body   [$self _ReadCountedString $socket]

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
	fileevent $options(-socket) readable [list]
	close $options(-socket)

	set options(-socket) -1

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

	if {[catch {$self _ReadTextMessage $socket} msg ]} {
	    $self _Close ERROR
	    return
	}

	set header [lindex $msg 0]
	set body   [lindex $msg 1]

	# Must be a CONNECT message with a non-zero body.
	#

	if {$header ne "CONNECT"} {
	    puts $socket "ERROR {Expected CONNECT}"
	    $self _Close ERROR
	    return
	}

	if {[string length $body] == 0} {
	    puts $socket "ERROR {Empty Body}"
	    $self _Close ERROR
	    return
	}
	puts $socket "OK"

	# Save the description and transition to the active state:
        # Register ourself with the event builder core

	set options(-description) $body
	$self _Expecting _Fragments ACTIVE

    }
    #
    # Expecting fragments if the next message is
    # DISCONNECT, close the socket after responding.
    # If the next message has a FRAGMENTS header
    # pass the socket down to the C/C++ code
    # for further processing.
    #
    # @param socket - socket that is readable.
    #
    method _Fragments socket {
	set status [catch {
	    set header [$self _ReadCountedString $socket]
	    set body   [$self _ReadBinaryData    $socket]
	} msg]
	if {[eof $options(-socket)]} {
	    $self _Close LOST
	    return;		# Nothing else to do.
	}
	# Protocol allows a DISCONNECT here:

	if {$header eq "DISCONNECT"} {
	    puts $socket "OK"
	    $self _Close

	} elseif {$header eq "FRAGMENTS"} {

	    # protocol allows FRAGMENTS here:
	    # TODO: Handle errors as a close

	    EVB::handleFragment $socket
	    puts $socket "OK"

	} else {
	    # Anything else is a crime against The Protocol:

	    puts $socket "ERROR {Unexpected header: $header}"

	}
    }
}

##
#  Connection manager object.  Creating one of these
#  creates a connection manager on the specified port
#  which will create Connection objects each time a connection arrives.
#  and will destroy them as they die.
#
# OPTIONS
#   -port - port on which we are listening for connections.
#   -connectcommand - script to call when a connection has been added.
#                    Substitutions:
#                     %H - Host that is connecting with us.
#                     %O - Connection object created to manage this connection.
#   -disconnectcommand - Script to call when a connection has been lost.
#                     %O - Connection object created to manage this connection.
#                     %H - Host from which the client came.
#                     %D - Connection descdription.
#                     
# METHODS:
#  getConnections - List the connections and their states.
#
snit::type EVB::ConnectionManager {
    option -port;		# Port on which we listen for connections.

    option -connectcommand   -default [list]  -configuremethod _SetCallback
    option -disconnectcommand -default [list] -configuremethod _SetCallback

    variable serverSocket;	# Socket run by us.
    variable connections [list]; # List of Connection objects.
    variable callbacks;		 # Callback manager.

    constructor args {
	$self configurelist $args; # To get the port.

	#
	# Set up the callback manager.
	#
	set callbacks [EVB::CallbackManager %AUTO%]
	$callbacks define -connectcommand
	$callbacks define -disconnectcommand



	set serverSocket [socket -server [mymethod _NewConnection] $options(-port)]
    }
    destructor {
	foreach object $connections {
	    $object destroy
	}
	close $serverSocket
    }
    #-----------------------------------------------------------------
    # Public methods:
    #

    ##
    # Get the list of connections and their properties/status:
    #
    # @return list of three element sublists (possibly empty).
    # @retval Each list element contains in order:
    #  - Client's host.
    #  - description of client.
    #  - Connection state string.
    #
    method getConnections {} {
	set result [list];	# possibly empty return list.
	foreach connection $connections {
	    set host  [$connection cget -clientaddr]
	    set desc  [$connection cget -description]
	    set state [$connection cget -state]

	    lappend result [list $host $desc $state]
	}
	return $result
    }
    #-----------------------------------------------------------------
    #  Private methods.

    ##
    # Client disconnect is just removing it from the list of connections
    # and invoking the disconnect callback.  Once all that dust has settled,
    # the connection object is destroyed, and its socket closed.
    #
    #
    # @param object - connection object name.
    # @note  Since it should not be possible for an object to disconnect that
    #        is not in our list, we toss an error if we are handed one.
    #
    method _DisconnectClient object {
	set objectIndex [lsearch -exact $connections $object]
	if {$objectIndex != -1} {

	    $callbacks invoke -disconnectcommand [list %O %H %D] \
		[list $object [$object cget -clientaddr] [$object cget -description]]
	    set connections [lreplace $connections $objectIndex $objectIndex]
	    $object destroy
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
    #  - We invoke the -connectcommand callback.
    #
    #
    #  @param sock - socket connected to the client
    #  @param client - Client IP addrerss.
    #  @param cport - client port (we could care less).
    #
    method _NewConnection {sock client cport} {

	set connection [EVB::Connection %AUTO% -socket $sock  -clientaddr $client]
	lappend connections $connection
	$connection configure -disconnectcommand [mymethod _DisconnectClient $connection]
	$callbacks invoke -connectcommand [list %H %O] [list $client $connection]
    }
    ## 
    # We very carefully ensured the callbacks registered with the callback manager
    # are the same as the options that set/get them.
    # this ensures that we can have a single configuremethod take care of both of them.
    # 
    # @param option - Option being set (callback name).
    # @param value  - New value for the option (the callback).
    #
    method _SetCallback {option value} {
	$callbacks register $option $value
	set options($option) $value
    }
}

