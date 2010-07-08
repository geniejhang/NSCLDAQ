#
#   This file contains the run setup gui.
#   This consists of controls that set the
#   readout frequency, the set of scaler channels to use
#   the run duration and to start/stop the run.
#
#   The entire GUI is implemented as a self contained snit widget.
#
package require Tk
package require snit

snit::widget runsetup {

    # Channels is the array of variables bound to the channel enable
    # bitmask.

    variable channels -array {1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 
	1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1}
    variable state      halted
    variable timerId    -1
    variable elapsedSeconds 0
    variable runDuration

    #
    # Construction ignores args as this is not configurable.
    constructor {args} {
	variable channels

	# Elapsed timein a run.

	label $win.elapsedl -text {Run elapsed time: }
	label $win.elapsed  -text {0-00:00:00}

	# Spinbox for the period:

	label   $win.periodl -text {Period in Hz}
	spinbox $win.period -values [list 6 10 20 50 100 200 500] -width 4

	# Duration as a set of h:m:s entries:

	label $win.durationl -text {Duration  d-h:m:s}
	spinbox $win.durationd -from 0 -to 100 -width 3
	label   $win.durationdl -text {-}
	spinbox $win.durationh -from 0 -to 24 -width 2
	label   $win.durationhl -text {:}
	spinbox $win.durationm -from 0 -to 60 -width 2
	label   $win.durationml -text {:}
	spinbox $win.durations -from 0 -to 60 -width 2

	# Channel selectors:

	label $win.channelsl -text {Channel enables}
	for {set i 0} {$i < 32} {incr i} {
	    set channels($i) 1
	    checkbutton $win.ch$i -text $i -variable [myvar channels($i)]

	}
	# Run control.. one button. which is labeled start to begin with
	# and re-labels itself stop when the run is active.
	

	button $win.startstop -text Start -command [mymethod startStop]

	# Set up the layout (various additional layout labels are
	# defined here as well:

	grid $win.elapsedl -row 0 -column 0 -columnspan 4
	grid $win.elapsed  -row 0 -column 4 -columnspan 3
	
	grid $win.durationl -row 5 -column 0 -columnspan  7
	grid $win.durationd $win.durationdl $win.durationh $win.durationhl \
             $win.durationm $win.durationml $win.durations

	grid $win.periodl -column 3 -columnspan 3
	grid $win.period  -column 3 -columnspan 2

	grid $win.startstop -row 10 -column 3 -columnspan 2

	#  The channel enables are two vertical columns on the right side:

	grid $win.channelsl -row 0 -column 10 -columnspan 2
	for {set i 0} {$i < 16} {incr i} {
	    set row [expr $i+1]
	    grid $win.ch$i -row $row -column 10
	    set ii [expr $i + 16]
	    grid $win.ch$ii -row $row  -column 11
	}

    }

    #Methods:

    # Provide the frequency of the readout:

    method getFrequency {} {
	return [$win.period get]
    }

    #  Get the current value of the run state variable

    method getState {} {
	variable state 
	return $state
    }
    # Get a list of channel enables:
    # form is as in array get {chann# value chan# value ...}

    method getEnables {} {
	variable channels

	return [array get $channels]
    }
    #  Handle the start/stop button:

    method startStop {} {
	variable state

	if {$state eq "halted"} {
	    $self start
	} else {
	    $self stop
	}
    }
    # Stop a run.. For now this is just sending the end command.
    # Cancel the run timer.
    method stop {} {
	variable timerId
	variable state
	end
	if {$timerId != -1} {
	    after cancel $timerId
	    set timerId -1
	}
	set state halted
	$win.startstop configure -text Start
    }
    # Start a run: 
    #  Calculate the period in 10n multiples and write ~/config/runconfig.tcl
    #  so that's the period of gdgb.
    #  Start the run timer to tick every second.
    #  begin the run.
    method start {} {
	variable timerId
	variable elapsedSeconds
	variable state
	variable runDuration

	set gdgdelay   [$self getPeriod]
	set runSeconds [$self getRunDuration]
	$self writeConfigFile $gdgdelay

	#
	# Initialize the appropriate state variables:
	#
	set elapsedSeconds 0
	set runDuration $runSeconds
	set timerId [after 1000 [mymethod tick]]

	begin
	set state active
	$win.startstop configure -text Stop
    }

    #  Figure out the gate generator delay parameter from the value of the
    # period spin box.
    
    method getPeriod {} {
	set hz [$win.period get]
	
	# GDG delay in 10ns units that is 1.0e8/$hz

	set delay [expr int(1.0e8/$hz)]
	return $delay
    }
    # Figure out how long the run is in seconds.
    # given the duration boxes:

    method getRunDuration {} {
	set days    [$win.durationd get]
	set hours   [$win.durationh get]
	set minutes [$win.durationm get]
	set seconds [$win.durations get]

	set duration [expr $seconds + $minutes * 60 + ($hours  + 24*$days)*3600]

	return $duration
    }
    #
    #  Write a ccusb configuration line for the gate generator delay.
    #

    method writeConfigFile {delay} {
	set fname [file join  ~ config runconfig.tcl]
	set fd    [open  $fname w]

	puts $fd "ccusb config ccusb -gdgbdelay $delay"
	close $fd


    }
    #  Count a second of elapsed run time.
    
    method tick {} {
	variable elapsedSeconds
	variable runDuration
	variable timerId

	incr elapsedSeconds
	$self updateElapsedTime

	if {$elapsedSeconds >= $runDuration} {
	    $self stop
	    return
	}
	set timerId [after 1000 [mymethod tick]]
    }
    #  Update elapsed time display:

    method updateElapsedTime {} {
	variable elapsedSeconds

	set seconds [expr $elapsedSeconds % 60]
	set minutes [expr ($elapsedSeconds / 60) % 60 ]
	set hours   [expr ($elapsedSeconds / 3600) % 60]
	set days    [expr ($elapsedSeconds/(3600*24))]

	set elapsed [format %03d-%02d:%02d:%02d $days $hours $minutes $seconds]

	$win.elapsed configure -text $elapsed
    }
}