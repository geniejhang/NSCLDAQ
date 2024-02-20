##
#  This package provides a simple interface for determining the
#  elapsed time. Prerequisites:
#
#   The configuration must have a kvstore element named 'run_start_time'
#   The BEGIN sequence must run rdo_stamp to define when the run starts.
#


package provide rdo_ElapsedTime 1.0


if {[array names env DAQTCLLIBS] ne ""} {
    lappend auto_path $::env(DAQTCLLIBS)
}

package require kvclient


namespace eval ElapsedTime  {
    variable start_time "Unknown"
    variable client
}

#
#  Set the KVStore client
#
proc ElapsedTime::setKvClient {c} {
    set ::ElapsedTime::client $c
}

#
#  Get the begin run timestamp (if possible).
#  We assume 0 means unknwon too.
#
proc ElapsedTime::begin {} {
    if {[catch {$::ElapsedTime::client  getValue run_start_time} time]} {
	set ElapsedTime::start_time "Unknown"
    } else {
	set ElapsedTime::start_time $time
    }
}
#
#  Returns a nice string to put in the elapsed time widget.
#
proc ElapsedTime::get {} {
    set start $ElapsedTime::start_time
    if {![string is integer $start]} {
	return $start
    } else {
	set now [clock seconds]
	set elapsed_secs [expr {$now -$start}]
	set secs [expr {$elapsed_secs % 60}]
	set min  [expr {int($elapsed_secs / 60) % 60}]
	set hrs  [expr {int($elapsed_secs/ 3600) %3600}]

	return [format "%d:%02d:%02d" $hrs $min $secs]
    }
}

