#
#  Gui to control the module in calibration mode.
#  This software allows readout data to be corelated with some position information.
#

package require Tk
package require csv

set scriptdir [file dirname [info script]]
set scalerWin [toplevel .scalerDisplay]

set coordinates 0

source [file join $scriptdir runsetup.tcl]
source [file join $scriptdir ledcontrol.tcl]

set numScalers  32

#position input 'frame'.

label .positionLabel -text {Position}
checkbutton .cartesian -text {Cartesian} -variable coordinates -command flipcoords



label .zlabel        -text {Z position}
entry .z             -textvariable zposition -width 6
.z insert 0 0.0

label .rlabel        -text {radius}
entry .r             -textvariable r         -width 6
.r insert 0 0.0

label .thetalabel    -text {theta}
entry .theta         -textvariable theta     -width 6
.theta insert 0 0.0

#  Run control

label   .runcontrolLabel -text {Run Control}
label   .freqLabel -text {Readout Frequency}
spinbox .frequency -values {6 10 20 50 100 200 500} -width 4
button .newpos   -text {New position} -command newPosition
button .start    -text {Start}         -command startStop

#  Layout the widgets:

grid     .cartesian  .positionLabel
grid     .zlabel     .z
grid     .rlabel     .r
grid     .thetalabel .theta
grid     .freqLabel  .frequency
grid     .newpos     -
grid     .start      -

#
#  Called to set the labels for the coordinates depending on the system.
#
proc flipcoords {} {
    global coordinates

    if {$coordinates} {
	.zlabel configure -text {X position}
	.rlabel configure -text {Y position}
	.thetalabel configure -text {Z position}
    } else {
	.zlabel configure -text {Z position}
	.rlabel configure -text {radius}
	.thetalabel configure -text {theta}
    }
}

puts "Gridded GUI"
proc createLedControl {} {
    toplevel .led
    ledControl .led.led1 -name led1
    ledControl .led.led2 -name led2
    pack .led.led1 .led.led2
}

proc destroyLedControl {} {
    destroy .led
}
puts "Scheduling ledcontrol"

after 5000 createLedControl

puts "Scheduled"
#-------------------------------------------------------------------------
# 
#  Scaler display:


puts "Sourcing scaler display"


source /usr/opt/daq/8.2/TclLibs/ScalerDisplay/scaler.tcl

#  Setup for numScalers channels
puts "Creating scaler defs"

page scalers {Count rates}

for {set i 0} {$i < $numScalers} {incr i} {
    set name [format Scaler.%02d $i]
    channel $name $i
    display_single scalers $name


}

proc clearScalers {} {
    global numScalers
    global Scaler_Increments
    global Scaler_Totals

    for {set i 0} {$i < $numScalers} {incr i} {
	set Scaler_Increments($i) 0
	set Scaler_Totals($i)     0
    }
}

clearScalers
 
#  Create other globals expected by scaler display:

set ElapsedRunTime        0
set ScalerDeltaTime       1.0
set RunTitle              {Fluid Tracking diagnostics}
set RunNumber             0
set RunState              Halted

set frequency             0
set updatesToGo           0


# Update scaler information from an event.
# If the updates to go goes to zero.. then trigger a display update.
# @param event - a list of the scaler increments for this readout.
proc scalerUpdate event {
    global Scaler_Increments
    global Scaler_Totals
    global frequency
    global updatesToGo
    global ElapsedRunTime
    global numScalers


    # on the first update of a group zero the increments
    # so the rate is right.

    if {$updatesToGo ==  $frequency} {
	for {set i 0} {$i < $numScalers} {incr i} {
	    set Scaler_Increments($i) 0
	}
    }
    # Update increments and totals from the event:

    set i 0
    foreach count $event {
	if {$i < $numScalers} {
	    set Scaler_Increments($i) [expr $Scaler_Increments($i) + $count]
	    set Scaler_Totals($i)     [expr $Scaler_Totals($i)     + $count]
	}
	incr i
    }
    incr updatesToGo -1
    
    # If appropriate, trigger a scaler update (one second worth of run data:

    if {$updatesToGo == 0} {
	incr ElapsedRunTime
	Update
	set updatesToGo $frequency
	update idletasks

    }

}

#
#  If . is being destroyed, then 
#  1. end any active run.
#  2. exit the program.
#


bind . <Destroy> {shutdown %W}
set   exiting 0
rename exit _exit
proc exit {} {
    bind . <Destroy> {}
    set exiting 1
    _exit
}

proc shutdown {widget} {
    global exiting
    global RunState
    if {$exiting} return

    if {$widget ne "."} {
	return
    }
    # End the run so we know the CC-USB is out of DAQ mode:
    #
    if {$RunState ne "Halted"} {
	catch {end};		# Could be ended but state var not set.
    }
    exit

}
#------------------------------------------------------------------------------
#  Run control actions
#

set  position1 [.z get]
set  position2 [.r get]
set  position3 [.theta get]

set entering 0

#
#  Process a request to put in a new set of positions.
#  - THis is a no-op if the run is halted.
#  - If entering is 0, we pause the run and flip the label to "Continue".. set entering 1and do nothing.
#  - If entering is 1 we latch the positions into position* variables, flip the label to 
#    New Postion resume the run and set entering 0..
#  

proc updatePositions {} {
   global position1 position2 position3

    set position1 [.z get]
    set position2 [.r get]
    set position3 [.theta get]
    
}

proc newPosition  {} {
    global entering
    global RunState
    global finalEnd

    if {$RunState ne "Halted"} {
	if {$entering} {

	    # Latch positions.

	    updatePositions
	    
	    # Set new button state.

	    .newpos configure -text {New Position}

	    # resume the run.

	    begin

	    
	} else {
	    #  Set the button state:

	    set entering 1
	    set finalEnd 0
	    .newpos configure -text {Continue}

	    #  Pause the run:

	    end


	}
    }
}
#
#  Start/Stop the calibration, depending on RunState.. note that if we are in the
#  middle of entering data we don't need to stop the run as it's already stopped.
#  .. and we need to reset the button label/entering value.
#
#

set finalEnd 0;			# Indicates the run is really ending not pausing.
proc startStop {} {
    global RunState
    global entering
    global RunNumber
    global finalEnd
    global updatesToGo
    global frequency
    global ElapsedRunTime

    if {$RunState eq "Halted"} {
	set ElapsedRunTime 0
	set RunState "Active"
	set entering 0
	.start  config -text {End}
	.newpos config -text {New Position}
	
	# Write the rdo frequency configuration file.

	set frequency [.frequency get]
	set delay [expr int(1.0e8/$frequency)]
	set updatesToGo $frequency

	set fd [open [file join ~ config runconfig.tcl] w]
	puts $fd "ccusb config ccusb -gdgbdelay $delay"
	close $fd
	destroyLedControl
	begin
    } else {
	# Stop the run.
	set finalEnd 1
	if {!$entering} {	# end data taking if active.
	    end
	} else {
	    onEnd $RunNumber "title is ignored." [clock format [clock clicks]]
	}
	# Regardless set the state variables and the labels.
	set RunState "Halted"
	set entering 0
	.start config -text {Start}
	.newpos config -text {New Position}
    }
}




#----------------------------------------------------------------------------
#
#   Data acquisition...event handlers for data taking events.
#

set tempFile   -1;		# Fd for temporary file.
set records    0;		# Total count of records written to file (for header)

#
#  Called when the readout thread finshes starting the run.
#
#   If just pausing to enter a new position, nothing happens.
#   Otherwise we must open the new temp file 
#   for write 
# 
#  Set run state for a scaler display update too.
#
proc onBegin {runNumber title time} {
    global entering
    global tempFile
    global records
    global RunNumber
    global RunState

    puts "Begin run buffer"
    if {$entering} {
	set entering 0
    } else {
	clearScalers
	set tempFile [open [file join /tmp run$runNumber.csv] w]
	set records 0
	set RunNumber $runNumber
	set RunState "Active"
	Update
    }
}
#
#  End of run.  If this is not a final end of run, nothing gets done.
#  if it is, we must create the final output file,  writing the
#  header and copying the temp file contents into it.
#
#  set run state and force a scaler display update.
#
proc onEnd {runNumber title time} {
    global finalEnd
    global RunNumber
    global tempFile
    global records
    global run;			# We'll keep the run numbers cool too.
    global RunState


    if {$finalEnd} {
	set tempFilename [file join /tmp run$RunNumber.csv] 
	close $tempFile
	set   tempFile -1

	puts "Run $RunNumber ended.  Creating final output file"
	set finalFile  [tk_getSaveFile -defaultextension .csv \
		      -filetypes {
			  {{Comma separated fields} .csv }
			  {{All Files}  * }
		      } -initialdir ~]
	set fdo [open $finalFile w]
	set fdi [open $tempFilename r]
	puts $fdo $records
	while {![eof $fdi]} {
	    puts $fdo [gets $fdi]
	}
	close $fdo
	close $fdi
	puts "Final file written: $finalFile"

	set finalEnd 0
	set records 0
	set run [incr RunNumber]; # Next run is +1 of RunNumber damnit.
	set RunState Halted
	Update
	createLedControl

    } else {

    }
}
#
#  Write events to the file
#  Update the scalers.
#
proc onEvent {events} {
    foreach event $events {
	writeEvent $event
	scalerUpdate $event

    }
}
#
#  Prepend the positions to the event and write the event to the output file.
#  
proc writeEvent event {
    global position1
    global position2
    global position3
    global tempFile
    global records
   
   
    set record  [concat $position1 $position2 $position3 $event]
    puts $tempFile [csv::join $record]
    incr records

}

# Update scaler information from an event.
# If the updates to go goes to zero.. then trigger a display update.
# @param event - a list of the scaler increments for this readout.
proc scalerUpdate event {
    global Scaler_Increments
    global Scaler_Totals
    global frequency
    global updatesToGo
    global ElapsedRunTime
    global numScalers


    # on the first update of a group zero the increments
    # so the rate is right.

    if {$updatesToGo ==  $frequency} {
	for {set i 0} {$i < $numScalers} {incr i} {
	    set Scaler_Increments($i) 0
	}
    }
    # Update increments and totals from the event:

    set i 0
    foreach count $event {
	if {$i < $numScalers} {
	    set Scaler_Increments($i) [expr $Scaler_Increments($i) + $count]
	    set Scaler_Totals($i)     [expr $Scaler_Totals($i)     + $count]
	}
	incr i
    }
    incr updatesToGo -1
    
    # If appropriate, trigger a scaler update (one second worth of run data:

    if {$updatesToGo == 0} {
	incr ElapsedRunTime
	Update
	set updatesToGo $frequency
	update idletasks

    }

}



#
#  Seems like this is needed to keep events flowing:

proc tick {} {
    after 1000 tick
}
tick
