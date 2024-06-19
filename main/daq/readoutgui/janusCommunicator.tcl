#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2024.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Genie Jhang
#            FRIB
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file janusCommunicator.tcl
# @brief Runs a server and JanusC with socket connection
# @author Genie Jhang <changj@frib.msu.edu>

# Initialize the server socket
set port [lindex $argv 0]
socket -server Connection $port

# Running JanusC
if {$argc > 2} {
	exec [lindex $argv 1] -g -d -c[lindex $argv 2] -p$port &
} else {
	exec [lindex $argv 1] -g -d -p$port &
}

set clients {}

proc Connection {newsock ipaddr port} {
    global clients
    lappend clients $newsock
    fconfigure $newsock -blocking 0 -buffering line
    fileevent $newsock readable [list Receiver $newsock]
}

proc Receiver {sock} {
    if {[gets $sock line] >= 0} {
      if {[string match "* ENDMSG" $line]} {
        close $sock
        set idx [lsearch $::clients $sock]
        set ::clients [lreplace $::clients $idx $idx]

        set line [string map [list " ENDMSG" ""] $line]
      }

      foreach client $::clients {
        if {$client ne $sock} {
          puts $client $line
        }
      }

			if {[string match "q" $line]} {
				foreach client $::clients {
					if {$client ne $sock} {
						close $client
						exit
					}
				}
			}
    }
}

vwait forever
