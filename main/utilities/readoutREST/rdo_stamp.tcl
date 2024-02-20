##
#   rdo_stamp.tcl
#     This program should be added to the end of the BEGIN sequence.
#     it will set the value of the KVStore variable:
#     run_start_time to the current [clock seconds]
#
#     This provides a timebase for the ReadouControl script to
#     provide a running time for active runs.
#
# Usage:
#   rdo_stamp.tcl -host manager_host -user manager_user
#
#   where
#      -host is the host the experiment manager is running in and
#      -user is the name of the user who started the manager.
#

#  Pull in the kvclient package we need:

if {[array names env DAQTCLLIBS] ne ""} {
    lappend auto_path $::env(DAQTCLLIBS)
}

package require kvclient

set varname run_start_time

##
#   Print he program usage to stderr and exit:
#
proc usage {} {
    puts stderr "Usage:"
    puts stderr "    rdo_stamp -host manager_host -user manager_user"
    puts stderr "Where:"
    puts stderr "   manager_host is the host (IP or DNS name) where the manager"
    puts stderr "                is running"
    puts stderr "   manager_user is the name of the user that started the manager"
    exit -1
}



#   Main entry point:

if {[llength $argv] != 2} {
    usage
}

set host [lindex $argv 0]
set user [lindex $argv 1]

#  Set the variable to the appropriate value

KvClient c -host $host -user $user

c setValue $varname [clock seconds]

c destroy
