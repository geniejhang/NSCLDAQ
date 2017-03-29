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
# @file stateProgramSerializer.tcl
# @brief Serialize state programs to the database.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide stateProgramSerializer 1.0
package require stateclient
package require stateProgram

##
# @note procs whose names (after the namespace) begin with _ are considered
#       private.
#


#  Establish the Serialize namespace (if needed);

namespace eval ::Serialize {
    namespace export serializeStatePrograms deserializeStatePrograms
}

##
# ::Serialize::_destroySmPrograms
#    Destroys all state manager programs currently in a database.
#
# @param api - api command.
#
proc ::Serialize::_destroySmPrograms api {
    foreach p [$api listPrograms] {
        $api deleteProgram $p
    }
}

##
# ::Serialize::_saveSmProgram
#    Save a state sensitive program to the database.
#
# @param api  - Api base command.
# @param program - Program object.
#
proc ::Serialize::_saveSmProgram {api program} {
    set properties [$program getProperties]
    set pos        [$program getPosition]
    
    set name [[$properties find name] cget -value]
    
    
    # Build the dict:
    
    set d [dict create]
    dict set d host       [[$properties find host]          cget -value]
    dict set d path       [[$properties find path]          cget -value]
    dict set d enabled    [[$properties find enable]        cget -value]
    dict set d standalone [[$properties find standalone]    cget -value]
    dict set d outring    [[$properties find "Output Ring"]  cget -value]
    dict set d inring     [[$properties find "Input Ring"]   cget -value]
    
    $api addProgram $name $d
    
    # Command line parameters:
    
    $api setProperty $name args [[$properties find "Program Parameters"] cget -value]
    $api setProperty $name type [[$properties find "type"] cget -value]
    #  Save canvas position:
    
    $api setEditorPosition $name [lindex $pos 0] [lindex $pos 1]
    
}

##
#  ::Serialize::serializeStatePrograms
#
#    Save the list of state programs to the database.
#
# @param uri - Database URI.
# @param sps - List of state programs.
#
proc ::Serialize::serializeStatePrograms {uri sps} {
    ::nscldaq::statemanager _smApi $uri
    
    ::Serialize::_destroySmPrograms _smApi
    
    foreach program $sps {
        ::Serialize::_saveSmProgram _smApi $program
    }
    
    ::nscldaq::statemanager -delete _smApi
}
##
# ::Serialize::deserializeStatePrograms
#
#   Recover the list of state programs that are in a database.
#
# @param dbURI - uri that specifies how to connect with the database.
# @return list of dicts - each dict contains the following items:
#          * object  - the object that will be cloned onto the cavnas.
#          * x,y     - desired editor canvas coordinates.
#
proc ::Serialize::deserializeStatePrograms dburi {
    ::nscldaq::statemanager _smApi $dburi
    set result [list]
    foreach program [_smApi listPrograms] {
        set info [_smApi getProgram $program]
        set x    [_smApi getEditorXPosition $program]
        set y    [_smApi getEditorYPosition $program]
        
        #  Create the object and fill its properties in from the info dict:
        
        set obj [StateProgram %AUTO%]
        set p   [$obj getProperties]
        
        [$p find name]  configure -value $program
        foreach pname [list enable  standalone path host "Input Ring" "Output Ring"] \
                dname [list enabled standalone path host inring        outring] {
            [$p find $pname] configure -value [dict get $info $dname]    
        }
        # There are two properties args -> "Program Arguments" and type -> "type"
        #  The catches provide for the case of pre-property databases:
        
        catch {[$p find {Program Arguments}] configure -value [_smApi getProperty $program args]}
        if {[catch {[$p find type] configure -value [_smApi getProperty $program type]}]} {
            [$p find type] configure -value StateProgram
        }
       
        
        #  Add a new dict to the result:
        
        lappend result [dict create object $obj x $x y $y]
    }
    
    ::nscldaq::statemanager -delete _smApi
    return $result
}
