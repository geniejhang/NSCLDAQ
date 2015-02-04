#!/bin/sh

# start wish \
exec wish ${0} ${@}
#
#  SEE application for shaper control.
#  Source in configuration files build the appropriate set of GUI's.
#
proc ResetDefaults {} {
    global outconfig outpolarity shapetime coarsegain finegain pole0 offset

    for {set i 0} {$i < 8} {incr i } {
	set outconfig($i)     0
	set outpolarity($i)   1
	set shapetime($i)     1
	set coarsegain($i)    0
	set finegain($i)      0
	set pole0($i)         0
        set offset            0
    }
}


set me $argv0                       ;# Full path to this script.
set instdir [file dirname $me]      ;# Where we are installed.

package require caennet
source $instdir/n568b.tcl
package require n568b

wm withdraw .



source $instdir/shaper.ui.tcl


set vme0   [caennet::create 0x100000]
caennet::reset $vme0

puts "argv - $argv"

foreach file $argv {

    ResetDefaults
    source $file
    toplevel .$name
    shaper_ui .$name
    
    CreateModule $name $controller $nodeid

}

