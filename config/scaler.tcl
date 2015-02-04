set dataurl tcp://$env(DAQHOST):2602/
set scalerdir /usr/opt/daq/8.0/Scripts
set configdir $env(CONFIGDIR)
set daqroot   $env(DAQROOT)
set exproot   $env(HOME)/experiment
set current   $env(HOME)/experiment/current

source $scalerdir/scaler.tcl
source $configdir/hardware.tcl

page ALL "All of the scalers"

display_ratio  ALL master.live master

display_ratio  ALL  see.ppac.u see.ppac.d
display_ratio  ALL  see.ppac.l see.ppac.r
display_single ALL  see.ppac.a

display_ratio  ALL see.scint.u see.scint.d
display_ratio  ALL see.scint.l see.scint.r

stripparam see.scint.u
stripparam see.scint.d
stripparam see.scint.l
stripparam see.scint.r

stripratio see.scint.u see.scint.d
stripratio see.scint.l see.scint.r



set clientpid [exec $daqroot/bin/sclclient -s $dataurl & ]

proc Cleanup {widget pid} {
    if {$widget != "."} return
    exec kill $pid
}

bind . <Destroy> "Cleanup %W $clientpid"


set open 0
set fd   ""

proc UserBeginRun {} {
    global current
    global open
    global fd
    global RunNumber
    global RunTitle

    set fd [open $current/scint.log w]
    puts $fd "\"$RunTitle\",$RunNumber"
    puts $fd "Time,up,down,left,right"
    flush $fd
    set open 1
}

proc UserEndRun {} {
    global fd
    global open

    if {$open} {
	close $fd
	set open 0
    }
}

proc UserUpdate {} {
    global open
    global fd
    global Scaler_Increments
    global ScalerMap

    if {$open} {
	set up    $Scaler_Increments($ScalerMap(see.scint.u))
	set down  $Scaler_Increments($ScalerMap(see.scint.d))
	set left  $Scaler_Increments($ScalerMap(see.scint.l))
	set right $Scaler_Increments($ScalerMap(see.scint.r))
	set time [clock seconds]
	set time [clock format $time]
	puts $fd "\"$time\",$up,$down,$left,$right"

	flush $fd
    }
}

