##
#  This file can be sourced into a scaler configuration script.
#  It can then be used to create detailed textual log files for scalers.
#  The log files are csv files that contain a header of the form:
#
#  run, number
#  time,scalername1,scalername2....
#
#  The body contains the CVS data implied by the second header.
#
#
package require csv
set ::logFd   -1;		# File descriptor of open logfile
set ::scalerLogDir [file join ~ scalerfiles]
file mkdir $::scalerLogDir;           # Ensure the scaler log file directory is created.

##
# UserBeginRun 
#
#  Called at the beginning of the run:
#  - Opens the log file.
#  - Writes the header using the ScalerMap and RunNumber globals.
# 
proc UserBeginRun {} {
    set fileName [file join $::scalerLogDir [format run%05d.log $::RunNumber]]
    set ::logFd  [open $fileName w]
    fconfigure $::logFd -buffering line

    set startTime [clock format [clock seconds]]

    puts $::logFd [::csv::join [list "Run:" $::RunNumber "Start Time:" $startTime]]
    set names [array names ::ScalerMap] 
    set channels [list]
    foreach name $names {
	set c $::ScalerMap($name)
	if {$c ne ""} {
	    lappend channels $name
	}
    }

    puts $::logFd [::csv::join [concat "time" [lsort $channels]]]


    
}
##
#  If the logfile is open, write the current scaler data.
#  Scaler increments are written.
#
proc UserUpdate {} {
    if {($::logFd != -1) && ($::ScalerDeltaTime != 0)} {
	set outlist $::ElapsedRunTime; # Seconds into the run.
	foreach name [lsort [array names ::ScalerMap]] {
	    set c    $::ScalerMap($name); # channel number.
	    if {[array names ::Scaler_Increments $c] ne ""} {
		set inc  $::Scaler_Increments($c); # Increments this period.
		set rate [expr {($inc*1.0)/$::ScalerDeltaTime}]
		lappend outlist $rate
	    }
	}
	puts $::logFd [csv::join $outlist]
    }
}
##
# UserEndRun
#
#  If the run ends, and the file is open, close it..
#
proc UserEndRun {} {
    if {$::logFd != -1} {
	close $::logFd
	set ::logFd -1
    }
}
