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
# @file svcValidator.tcl
# @brief Validate all services.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide serviceValidator 1.0
package require Service
package require objectInstaller
package require connectorInstaller

namespace eval ::Validation {
    variable requiredServiceDescriptions
    array set requiredServiceDescriptions [list                                \
        vardb-service     {The variable database server daemon}                \
        boot-service      {The DAQ experiment boot service}                    \
        statusinjector    {The status message to database injection service}   \
    ]
    
    variable requiredServiceDefaultPaths
    array set requiredServiceDefaultPaths [list                                \
        vardb-service  [file join $::env(DAQBIN) vardb-service]                \
        boot-service   [file join $::env(DAQBIN) boot-service]                 \
        statusinjector [file join $::env(DAQBIN) statusinjector]              \
    ]
}
##
# ::Validation::validateServices
#   For each service provided, checks that:
#   - The service has a name.
#   - The service has a path.
#   - The service has been allocated to a host.
#
#  Check that there are services whose filename are:
#    - vardb-service  - The variable database server service wrapper shell.
#    - boot-service   - The boot service wrapper shell
#    - statusinjector - The status database status injection program.
#
#
#  With the exception of nameless services each problem results in an error
#  added to the result list. If there are several services without a name, however,
#  these messages get collapsed to a single error.
#
# @param svcs  - list of service objects.
# @return list - possibily empty list of validation failure messages.
#
proc ::Validation::validateServices svcs {
    variable requiredServiceDescriptions   

    set namelessCount 0
    set result [list]
    array set names [list]
    set svcPrograms [list]
    foreach svc $svcs {
        set p [$svc getProperties]
        set name [[$p find name] cget -value]

        if {$name eq ""} {
            incr namelessCount
            set name -no-name-
        } elseif {[array names names $name] ne ""} {
            lappend result "There is more than one service named $name"
        } else {
            set names($name) $name
        }
        if {[[$p find host] cget -value] eq ""} {
            lappend result "Service $name has not been assigned to a host"
        }
        set path [[$p find path] cget -value]
        if {$path eq ""} {
            lappend result "Service $name has not been given a path."
        } else {
            lappend svcPrograms [file tail $path];     # Don't care about the dirs.
        }
    }

    if {$namelessCount } {
        lappend svcPrograms "There are services that have not been named"
    }
    #
    #  Check for required services:
    #

    foreach requiredService [array names requiredServiceDescriptions] {
            
        
        if {$requiredService ni $svcPrograms} {
            lappend result \
                "The required service $requiredService: \
                $requiredServiceDescriptions($requiredService) has not been \
                defined."
        }
    }
    
    return $result
}

##
# Validation::createMandatoryServices
#   Create/install the mandatory services if they're not
#   already there.  We configure the service name and the service path from
#   requiredServiceDefaultPaths but the user will have to configure a few other
#   items.
#
# @param c   - Canvas on which the objects will be installed.
# @param oi  - Installer object (ObjectInstaller).
# @param ci  - Connector installer.
#
#
proc Validation::createMandatoryServices {c oi ci} {
    variable requiredServiceDefaultPaths
    set svcs [$ci listObjects .c service]
    
    #  Get names of existing services (those that have nonempty names that is).
    
    set existingNames [list]
    foreach svc $svcs {
       set p [$svc getProperties]
       set name [[$p find name] cget -value]
       if {$name ne ""} {
           lappend existingNames $name
       }
    }
    #  Now iterate over required services making those that are not yet
    #  in existence and installing them.
     
    set x 10
    set y 10
    
    foreach svcname [array names requiredServiceDefaultPaths] {
        if {$svcname ni $existingNames} {
            set path $requiredServiceDefaultPaths($svcname)
            set svcObj [Service %AUTO% -canvas $c]
            set p [$svcObj getProperties]
            set nprop [$p find name]
            set pprop [$p find path]
            
            $nprop configure -value $svcname
            $pprop configure -value $path
            
            $oi installAt $svcObj  $c $x $y
            
            incr x 20
            incr y 20
        }
    }
}
