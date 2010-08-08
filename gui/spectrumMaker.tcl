package require Tk
package require csv
package require Plotchart

set scriptdir [file dirname [info script]]

source  [file join $scriptdir ledcontrol.tcl]

# we have two leds control panels that will just be used to show us stepping
# through the LED values.

proc createLedControl {} {
    toplevel .led
    ledControl .led.led1 -name led1
    ledControl .led.led2 -name led2
    pack .led.led1 .led.led2
    wm withdraw .led;		# We only need it for its functions not for control.
}


createLedControl

# Main Gui is in the main toplevel and will have
#
# +------------------------------------+
# |  Dwell time [  ] seconds           |
# |            [ start/stop]           |
# |  Threshold  nn                     |
# +------------------------------------+

set currentThreshold 0;                # holds current threshold value.
set nextThreshold    0;                # used to throw away the first data point in each setting.
set runState         0;                # 0 stopped, 1 active.
set dwellTime        3;                # Seconds for each step.

set afterId         -1


proc createControlGui {} {
    label     .dwellLabel     -text {Dwell Time}
    spinbox   .dwell          -from 3 -to 10 -increment 1 -width 3 \
                              -textvariable  dwellTime
    label     .secondsLabel   -text {seconds}

    button   .startstop      -text {Start} -command startStop

    label     .thresholdLabel -text {Threshold}
    label     .threshold      -textvariable currentThreshold

    # Layout the widgets

    grid .dwellLabel     .dwell      .secondsLabel
    grid  x              .startstop
    grid .thresholdLabel .threshold

}

createControlGui

bind . <Destroy> {shutdown %W}

# This is needed to prevent an infinite loop/recursion when exiting.
# normal exit will eventually destroy widgets which will call exit etc.
# Replace our exit to remove the <Destroy> event binding and then do the
# real exit.

rename exit _exit
proc exit {} {
    bind  . <Destroy> {}
    _exit

}

#
#  If . is being destroyed, then 
#  1. end any active run.
#  2. exit the program.
#

proc shutdown {widget} {
    catch {
    global runState

    if {$widget ne "."} {
	return
    }
    # End the run so we know the CC-USB is out of DAQ mode:
    #
    if {$runState ne "Halted"} {
	catch {end};		# Could be ended but state var not set.
    }
    exit
    } msg
    puts $msg

}


#--------------------------------------------------------------------------------
#
#  Code here is responsible for the data taking itself.
#

#  Data for the run will be summed as follows:
#
#   - samplesPerStep - array of 1024 elements that will have the number of
#                      samples we got at each step...for each scaler.
#   - sumsPerStep   - array of 1024 channel sums.
#
#  When the spectrum is made, we're going to normalize the counts before doing
#  the subtraction in order to ensure that we deal with the effect of differing
#  samples per timestep.  The number of samples/sec is hopefully large enough that the
#  'incomplete' first sample will not be a big factor.
#


proc clearData {} {
    global samplesPerStep
    global sumsPerStep


    for {set i 0} {$i < 32} {incr i} {
	lappend initial 0
    }

    for {set i 0} {$i < 1024} {incr i} {
	set samplesPerStep($i) 0
	set sumsPerStep($i)    $initial
    }
}
clearData
#
#  Called when the time for a step has completed.. We just end the run.
#  onEnd will take care of making the next step happen.
#

proc stepDone {} {
    end
}

#  Called to start a step
#
proc startStep {} {
    global currentThreshold
    global dwellTime

    .led.led1 setThresholdValue $currentThreshold
    .led.led2 setThresholdValue $currentThreshold

    begin
    set afterId [after [expr 1000*$dwellTime] stepDone]
}

#
#  Depending on runState either starts or stops the run.
#  if runState is 0, the run must be started.  The thresholds are set to zero,
#  the run is started and the dwell time clock is started so that
#  thresholds can be incremented.
#  We're going to write the configuration so that the rate is always 100Hz for now.
#

proc startStop {} {
    global runState
    global afterId
    global dwellTime
    global currentThreshold
    global nextThreshold
    global Filename

    if {$runState} {
	.startstop configure -text {Start}
	set runState 0
	set currentThreshold 1023;                 # Ensure we don't step anymore.. next step will stop.
    } else {

	set gdgbPeriod [expr int(1.0e8/50.0)];  # 50hz rate.
	set fd [open [file join ~ config runconfig.tcl] w]
	puts $fd "ccusb config ccusb -gdgbdelay $gdgbPeriod"
	close $fd

	clearData
	.startstop configure -text {Stop}

	set runState 1
	set currentThreshold 0
	set nextThreshold    0
	startStep


    }
}

# Add an event to the current data:
#

proc addEvent event {
    global currentThreshold
    global nextThreshold
    global samplesPerStep
    global sumsPerStep

    #  Toss the first event in a step as it can be partial:

    if {$currentThreshold == $nextThreshold} {
	incr nextThreshold
	return
    }

    #  Given how we test this, only process the first 32 scaler values.

    set sums $sumsPerStep($currentThreshold)
    set event [lrange $event 0 31]

    foreach sum $sums counts $event {
	if {$event ne ""} {
	    incr sum $counts
	}
	    lappend finalSums $sum
    }
    # Count the sample.

    set  sumsPerStep($currentThreshold) $finalSums
    incr samplesPerStep($currentThreshold);   # One more sample.
    
}



#  Called when a batch of events comes from the data.

proc onEvent events {
    foreach event $events {
	if {[catch {addEvent $event} msg]} {
	    puts "onEvent failed: $msg"
	}
    }
}
#
#  We don't process begin run buffers.

proc onBegin {runNumber title time} {

}
#
#  On end moves to the next step if appropriate.
#  Else processes the accumulated data.
#
proc onEnd {runNumber title time} {
    global dwellTime
    global currentThreshold
    global runState

    incr currentThreshold
    if {$currentThreshold > 1023} {
	puts "Scan completed, creating spectra"
	set runState 0
	.startstop configure -text {Start}
	processData
    } else {
	startStep
    }
}

#----------------------------------------------------------------
#
#  Data processing.
#

set plotChannel 0;		# Which channel to plot.

# Turn the sumsPerStep array into a spectrum.
# to do this; each channel is floating point divided by the number of
# triggers it received to normalize for variable trigger numbers.
# The spectrum is then created by taking the
# each channel and subtracting from it the next channel.
#
proc createSpectra {} {
    global sumsPerStep
    global samplesPerStep

    # normalize...

    for {set i 0} {$i < 1024} {incr i} {
	if {$samplesPerStep($i) != 0} {
	    set normalized [list]
	    foreach sum $sumsPerStep($i) {
		set normal [expr {double($sum)/double($samplesPerStep($i))}]
		lappend normalized $normal
	    }
	    set sumsPerStep($i) $normalized
	}
    }
    # Now take the derivative to get the spectrum:

    for {set i 0} {$i < 1023} {incr i} {
	set this $sumsPerStep($i)
	set next $sumsPerStep([expr $i+1])

	set result [list]
	foreach t $this n $next {
	    lappend result [expr ($t - $n)]
	}
	set sumsPerStep($i) $result
    }

}

# 
#  Create data spectra and write them out to file as CSV 
proc processData {} {
    global sumsPerStep
    global samplesPerStep
    global Filename
    puts "Creating spectra"

    if {[catch createSpectra msg]} {
	puts "Create spectrum failed: $msg"
    }

    set Filename [tk_getSaveFile -defaultextension .csv \
		      -filetypes {
			  {{Comma separated fields} .csv }
			  {{All Files}  * }
		      } -initialdir ~]
    
    
    puts "Writing output file: $Filename"



    set fd [open $Filename  w]
    for {set i 0} {$i < 1023} {incr i} {
	puts $fd [csv::join $sumsPerStep($i)]
    }
    close $fd

    puts {Done}

    if {![winfo exists .plotgui]} {
	plotGui
    }
}

# Plot a spectrum.
#
proc Plot {} {
    global plotChannel;		# Channel to plot.
    global sumsPerStep
    global cursorPosition

    set cursorPosition 0

    # Create the plot series:

    set max 0
    for {set i 0} {$i < 1024} {incr i} {
	lappend x $i
	set counts [lindex $sumsPerStep($i) $plotChannel]
	if {$counts > $max} {
	    set max $counts
	}

	lappend y $counts
    }
    set max [expr $max*1.1 + 1.0]
    
    #  Get max to a decade:

    set maxScale [expr (pow(10,(int(log10($max)) +1)))]

    #  Now we can try .5, .25 of full scale to get a bit more detail:

    if {($maxScale/2.0) > $max} { # .5
	set maxScale [expr $maxScale/2.0]
    }
    if {($maxScale/2.0)  > $max} { #  .25
	set maxScale [expr $maxScale/2.0]
    }
    puts $maxScale

    if {[winfo exists .plot]} {
	destroy .plot
    }

    toplevel .plot
    canvas   .plot.graph -width 800 -height 500
    pack     .plot.graph -fill both
    label    .plot.cursorx -textvariable cursorPosition
    pack     .plot.cursorx

    set chart [::Plotchart::createHistogram .plot.graph {0 1023 100}  [list 0.0 $maxScale [expr $maxScale/10.0]]]

    $chart title "Channel $plotChannel"

    foreach channel $x counts $y {
	$chart plot spectrum $channel $counts
    }

    # Motion event so that we can update cursor position:

    bind .plot.graph <Motion> [list updatePosition %W %x %y]
}

proc updatePosition {widget x y} {
    global cursorPosition
    set coords [::Plotchart::pixelToCoords $widget $x $y]

    set channel [lindex $coords 0]
    set cursorPosition [expr round($channel)]

}

#
#  Provides a GUI for plotting spectra we created.
#
#  Just looks like an array of 32 radio buttions and
#  a Plot button.. no big shakes.
#
proc plotGui {} {
    toplevel .plotgui

  
    label .plotgui.chooselabel -text {Select channel}
    for {set i 0} {$i  < 32} {incr i} {
	radiobutton .plotgui.channel$i -variable plotChannel -text $i -value $i
    }

    button .plotgui.plot -text {Plot} -command Plot

    # Layout the interface:
    # four rows of radiobuttons.
    
    grid .plotgui.chooselabel -column 0 -row 0 -columnspan 5
    set channel 0

    for {set row 1} {$row < 5} {incr row} {
	for {set col 0} {$col < 8} {incr col} {
	    grid .plotgui.channel$channel -row $row -column $col

	    incr channel
	}
    }

    # and the button:

    grid .plotgui.plot -row 6 -column 2 -columnspan 3
    
  
}

