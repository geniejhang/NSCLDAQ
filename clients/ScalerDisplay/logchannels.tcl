#
#  Scaler display add on that logs all scaler channels to file.
#  the file is a CSV ascii file. The file starts with a header
#  that defines the fields and then contains an entry for each scaler
#  update for each of the field values. One of the field values
#  is time  which is the number of seconds into the run.
#

#  To use this source it into your scaler configuration file
#  e.g.
#   source /path/to/logstripchart.tcl
#

package require csv


set userOutputFile  {};		# File descriptor when file open.
set userChannelList [list];     # list of channels to write.


#
#  Called when a run begins.
#  Figures out userChannelList, userChannelIds
#  and, if they are not empty, opens the output file.
#  If the output file is already open, UserEndRun is called
#  under the assumption the run ended badly (e.g. Readout or spectrodaq
#  failed.
#
proc UserBeginRun {} {
    set ::userChannelList [list]

    if {$::userOutputFile ne ""} {
	UserEndRun

    }
    set ::userChannelList [array names ::ScalerMap]

    # Get rid of the special channel named >>><<<:

    set specialIndex [lsearch $::userChannelList ">>><<<"]
    if {$specialIndex != -1} {
	set ::userChannelList [lreplace  $::userChannelList \
				   $specialIndex $specialIndex]
    }
    #
    # open the file and write the header if 
    # there are channels being strip charted.
    #
    if {[llength $::userChannelList] != 0} {
	set ::userOutputFile [open run${::RunNumber}-scaler.log w]
	set headerfields [concat time $::userChannelList]
	puts $::userOutputFile [::csv::join $headerfields ,]

	
    }

    
}
#
#  At the end of the run, if the file is open, 
#  - Close it.
#  - Set the global file descriptor to empty.
#
proc UserEndRun {} {
    if {$::userOutputFile ne ""} {
	close $::userOutputFile
	set ::userOutputFile {}
    }
}
#
#  If the file is open, write a line of scaler counts for that time period.
#
proc UserUpdate {} {
    if {$::userOutputFile ne ""} {
	set fields $::ElapsedRunTime
	foreach scaler $::userChannelList {
	    lappend fields [scalerRate $scaler]
	}
	puts $::userOutputFile [::csv::join $fields]
    }
}