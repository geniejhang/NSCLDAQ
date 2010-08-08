package require Tk
package require csv

set scriptdir [file dirname [info script]]

source [file join $scriptdir runsetup.tcl]
source [file join $scriptdir ledcontrol.tcl]




set tempfd  0;     # fd for temp file.
set records 0;     # Records written to temp file.
set numScalers 32;  # Number of scaler channels.

#------------------------------------------------------------------------
#
#  Control panel for discriminators.
#





proc createLedControl {} {
    toplevel .led
    ledControl .led.led1 -name led1
    ledControl .led.led2 -name led2
    pack .led.led1 .led.led2
}

proc destroyLedControl {} {
    destroy .led
}



#-------------------------------------------------------------------------
# 
#  Scaler display:

set scalerWin [toplevel .scalers]

source /usr/opt/daq/8.2/TclLibs/ScalerDisplay/scaler.tcl

#  Setup for numScalers channels

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


#--------------------------------------------------------------------------
#  Run control:

runsetup .setup -startcommand onStart -stopcommand onStop
pack .setup

bind . <Destroy> {shutdown %W}

# This is needed to prevent an infinite loop/recursion when exiting.
# normal exit will eventually destroy widgets which will call exit etc.
# Replace our exit to remove the <Destroy> event binding and then do the
# real exit.

rename exit _exit
proc exit {} {
    bind . <Destroy> {}
    _exit
}

#
#  If . is being destroyed, then 
#  1. end any active run.
#  2. exit the program.
#

proc shutdown {widget} {
    global RunState

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


# Start run pre-actions.. kill the .led control panel.
# Set up the scaler dispaly state variables.
#  Save the readout frequency so scaler displays can be handled.
#

proc onStart {} {
    global RunState
    global RunNumber
    global ElapsedRunTime
    global run
    global frequency
    global updatesToGo
    global Scaler_Totals
    global Scaler_Increments
    global numScalers
    global channelEnables

    destroyLedControl
    set RunState Active
    set RunNumber $run
    set ElapsedRunTime 0

    array set channelEnables [.setup getEnables]

    clearScalers
    BeginRun
    Update

    set frequency [.setup getFrequency]
    set updatesToGo $frequency
}

#
#  End run post actions.  Restart the led control panel
#  and setup the scaler display state variables.

proc onStop {} {
    global run
    global RunNumber

    createLedControl
    incr run
    incr RunNumber

    EndRun
    Update

}
#-------------------------------------------------------------------------
#  Data acquisition:
#

set run 0
#
#  Called at the start of the run.
#  The temp output file is opened and the handle 
#  put in the global tempfd
#
proc onBegin {runNumber title time} {
    global tempfd
    global records


    puts "Run $runNumber started '$title' At $time"
    set tempfd [open [file join /tmp run$runNumber.csv] w]
    set records 0

}
#
#  Close the temp file.. create the permanent file.
#  ulink the temp file.
proc onEnd {runNumber title time} {
    global tempfd
    global records

    puts "Run $runNumber ended '$title' At $time"
    
    close $tempfd
    puts "temp file closed"
    set finalFile  [tk_getSaveFile -defaultextension .csv \
		      -filetypes {
			  {{Comma separated fields} .csv }
			  {{All Files}  * }
		      } -initialdir ~]
    set fdo [open $finalFile  w]
    puts "output file: $finalFile  opened"
    set fdi [open [file join /tmp run$runNumber.csv] r]

    puts "File opened"


    puts "Creating final file..."

    puts $fdo $records
    while {![eof $fdi]} {
	puts $fdo [gets $fdi]
    }
    close $fdi
    close $fdo
    file delete [file join /tmp run$runNumber.csv]
    puts "done"

}
#
#  Write the events to the temp file in csv form and 
#  count how many we wrote
#
proc onEvent events {
    global tempfd
    global records
    global channelEnables
    global numScalers
    foreach event $events {
	if {$event ne ""} {
	    #
	    # Zero out the elements of the event that are 'disabled'.
	    #
	    catch {
	    set size [llength $event]
	    if {$numScalers < $size} {
		set size $numScalers
	    }

	    for {set i 0} {$i < $size} {incr i} {
		if {!$channelEnables($i)} {
		    set event [lreplace $event $i $i 0]
		}
	    } } msg
	    set line [::csv::join $event]
	    puts $tempfd $line
	    incr records
	    set result [catch {scalerUpdate $event} msg]
	    if {$result} {
		puts "Error in update: $msg"
	    }
	}
    }
}

after 3000 createLedControl
