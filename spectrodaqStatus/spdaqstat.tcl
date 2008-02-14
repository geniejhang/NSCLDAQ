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
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321


package provide spdaqstat 1.0

# This package (namespace also spdaqstat)
# provides mechanisms for getting diagnostic information regarding
# spectrodaq buffer/link status.
# The following names are exported:
#
#   spdaqstat::freepages - Free pages  in a host.
#   spdaqstat::usedpages - Used pages in a host.
#   spdaqstat::usedbypid - Used pages for each pid in a host.
#   spdaqstat::links     - Nodes with links to the host.
#
#  spdaqstat::spdaqhome  - Location of the spdaq home directory (variable)
#

namespace eval ::spdaqstat {
    variable spdaqhome /usr/opt/spectrodaq
    variable Domain
}
#-------------------------------------------------------------------------------
#
#  Initialization:

# Dns:

package require dns
set nameserver [lindex [dns::nameservers] 0]

dns::configure -nameserver $nameserver

# Now resolve the nameserver and extract the last parts of the
# name to set the domain search list

set token [dns::resolve $nameserver]
dns::wait $token
set nameserver [lindex [dns::name $token] 0]
dns::cleanup $token

set period [string first "." $nameserver ]
incr period
set domain [string range $nameserver $period end]

dns::configure -search $domain
set ::spdaqstat::Domain $domain;     # tcllib dns does not yet use -search >sigh<


#-------------------------------------------------------------------------------
# Private methods:

#
#   ProgramPath returns path to a spectrodaq utility:

proc ::spdaqstat::ProgramPath utility {
    return [file join $spdaqstat::spdaqhome bin $utility]
}

#
# StatPort returns the status port for spectrodaq by analyzing
# /etc/services, looking for the service sdaq-stat.
#
proc ::spdaqstat::StatPort {} {
    set f [open "/etc/services" r]
    set services [read $f]
    close $f
    set serviceList [split $services "\n"]
    foreach service $serviceList {
        set name     [lindex $service 0]
        set portInfo [lindex $service 1]
        if {$name eq "sdaq-stat"} {
            set portInfoList [split $portInfo "/"]
            return [lindex $portInfoList 0]
        }
    }
    error "Service not defined"
}

#
#  Constructs a status url to specific host.
#
proc ::spdaqstat::StatusUrl host {
    return "tcp://$host:[spdaqstat::StatPort]"
}
#
#  Resolve a host id (name or IP to a name) localhost is resolved to
#  the resolution of hostname.
#
proc ::spdaqstat::DNSResolve host {
    

    # If the host has no periods in it and is not localhost,
    # we need to append the domain name:

    if {([string first "." $host] == -1) && ($host ne "localhost")} {
	append host .  $spdaqstat::Domain
    }

    set token [dns::resolve $host]
    dns::wait $token
    set name  [dns::name    $token]
    set name [lindex $name 0]
    dns::cleanup $token
    if {$name eq "localhost"} {
        set uqname [exec hostname]
        append uqname . $spdaqstat::Domain
        set token  [dns::resolve $uqname]
        dns::wait $token
        set name   [dns::name $token]
        set name [lindex $name 0]
        dns::cleanup $token
    }
    return $name
    
}
#-------------------------------------------------------------------------------
#
#  Public methods for the package:
#


#++
#
#   ::spdaqstat::freepages - Returns the number of free pages in a spectrodaq running host.
#               It is an error to ask the status of a host not running spectrodaq.
# Parameters:
#   host      - host of which to ask this information, defaults to localhost
#
#--
proc ::spdaqstat::freepages {{host localhost}} {
    set url [spdaqstat::StatusUrl $host]
    set program [spdaqstat::ProgramPath freepages]
    
    set result [exec $program -u $url |& tail -1]
    return [lindex $result 1]

}

#++
# ::spdaqstat::usedpages - Returns the number of pages in use in a spectrodaq
#              running host.  It is an error to ask the status of a host that is
#              not running spectrodaq.
# Parameters:
#   host   -  Host of which to ask this information.  Defaults to localhost.
#
#--
proc ::spdaqstat::usedpages {{host localhost}} {
    set url     [spdaqstat::StatusUrl $host]
    set program [spdaqstat::ProgramPath usedpages]
    set result  [exec $program -u $url |& tail -1]
    return [lindex $result 1]
}


#++
# ::spdaqstat::totalpages - Returns a good estimate of the total  number of
#             pages available to a host's spectrodaq.  This is done by
#             totalling used and free pages for that host and, since the
#             number of used/free pages can change between calls, this can
#             be incorrect.
# Parameters:
#  Host    - The host from which to get these statistics. Defaults to localhost
#
#--
proc ::spdaqstat::totalpages {{host localhost}} {
    set used [spdaqstat::usedpages $host]
    set free [spdaqstat::freepages $host]
    
    return [expr ${free} + ${used}]
}


#++
# ::spdaqstat::usagebypid - Returns a list that describes the usage
#             of spectrodaq pages by pid in a host.
#             We do this by looking for the Owner=PID expression in each
#             line of the output of usedpages.
#             NOte that pid 0 is special; pages waiting to be reclaimed, so we
#            won't return it.
#
# Parameters:
#   host   - Host for which to inquire this. Defaults to localhost.
#
#--
proc ::spdaqstat::usagebypid {{host localhost}} {
    set url     [spdaqstat::StatusUrl $host]
    set program [spdaqstat::ProgramPath usedpages]
    set usage   [exec $program -u $url]
    set usageLines [split $usage "\n"]
    foreach line $usageLines {
        if {[regexp {Owner=\d+} $line itemstring]} {
            set pid [lindex [split $itemstring =] 1]
            if {$pid != 0} {
                if {[array names pages $pid] eq ""} {
                    set pages($pid) 0
                }
                incr pages($pid)
            }
        }
    }
    # pages(i) is number of pages used by pid i.
    # Let's sort this by pid so that the order is stable.
    #
    set result [list]
    foreach pid [lsort -integer [array names pages]] {
        lappend result [list $pid $pages($pid)]
    }
    return $result
}

#++
#  ::spdaqstat::links  - Returns a list of the systems to which data are
#                      being sent by the host.
#                      Where possible, host ip addresses are translated to
#                      host name.  This can lead to a peculiarity with
#                      the IP 127.0.0.1 which is localhost regardless
#                      of the system.  That IP is therefore resolved
#                      unconditionally as the host being queried.
#                      If the host being queried is localhost,
#                      127.0.0.1 is mapped to the resolution of
#                      the result of the hostname command.
# Parameters:
#   host   - Name of the host to query.  Defaults to local host.
#
#--
proc ::spdaqstat::links {{host localhost}} {
    set url     [spdaqstat::StatusUrl $host]
    set program [spdaqstat::ProgramPath linkstat]
    set links   [exec $program -u $url]
    set linkList [split $links "\n"]
    set result ""
    foreach link $linkList {
        if {[regexp {TCP://.*$} $link url]} {
            set urlparts [split $url /]
            set hostlist [split [lindex $urlparts 2] :]
            set hostid   [lindex $hostlist 0]
            lappend result [spdaqstat::DNSResolve $hostid]
        }
    }
    # Uniquify the list by throwing elements into array indices and returning
    # the ascii sorted list:
    
    foreach link $result {
        set linkarray($link) ""
    }
    return [lsort -ascii [array names linkarray]]
}

#                      
#-------------------------------------------------------------------------------
#
#   Exports from the ::spdaqstat namespace:
#
namespace eval ::spdaqstat {
    namespace export freepages
    namespace export usedpages
    namespace export usedbypid
    namespace export links
    namespace export spdaqhome

}