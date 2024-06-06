#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2024.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#    http://www.gnu.org/licenses/gpl.txt
#
#    Original Author:
#    Jeromy Tompkins
#	   NSCL
#	   Michigan State University
#	   East Lansing, MI 48824-1321
#
#    Author:
#    Genie Jhang
#	   FRIB
#	   Michigan State University
#	   East Lansing, MI 48824-1321

package provide mdppproxy 1.0

package require snit
package require usbcontrolclient

snit::type MDPPProxy {
    option -module -default tcl_mymdpp32
    option -server -default localhost
    option -port   -default 27000

    component _comObj

    delegate option * to _comObj

    constructor {args} {
        if {[catch {install _comObj using controlClient %AUTO%}]} {
            return -code error "Unable to connect the slow control server. Is the server running?"
        }

        $self configurelist $args
    }

    destructor {
        catch {$_comObj destroy}
    }

    method Set {param value} {
        return [$_comObj Set [$self cget -module] $param $value]
    }

    method Get {param} {
        return [$_comObj Get [$self cget -module] $param]
    }
}
