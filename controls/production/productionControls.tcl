#!/bin/bash
#
# Start Wish. \
exec wish ${0} ${@}

package require vhq          ;# We're going to control the hv devices!!



#  Where the master database file is.

set MasterDbFile "~see/saved_settings/database.tcl"
set Configdir    $env(CONFIGDIR)
set Distdir      $env(DISTRIBROOT)
set Logfile      ~/experiment/Controls.log
# 
#  SpeciesCompare compares two particle species in a way that they
#  can be sorted.  A species is a string of the form nntt
#  where nn is an integer butted up against a string.
#  The physically meaningful order sorts tt's alphabetically with ties
#  broken by the numerical comparison of the mass.
#
proc SpeciesCompare {s1 s2} {
    #  Break the species apart into mass and element:

    scan $s1 "%d%s" m1 e1
    scan $s2 "%d%s" m2 e2

    #  If e1 != e2 we know the answer:
    #
    if {$e1 < $e2} { return -1}
    if {$e1 > $e2} { return  1}

    #   Same elements, break the tie based on the mass #

    if {$m1 < $m2 } {return -1}
    if {$m1 > $m2 } {return  1}
    return 0

}

#
#  ECompare compares the energies of a list (suitable for use in 
#  sorting settings lists for a single isotope.
#
#
proc ECompare {e1 e2} {


    set e1 [lindex $e1 0]
    set e2 [lindex $e2 0]

    if {$e1 < $e2}   {return  -1}
    if {$e1 > $e2} {return   1}
    

    return 0
}
#
#  Select  - called when the energy is actually selected.
#
proc Select {isotope list} {
    global Configdir


    # Make the top level menu entry reflect the seleted beam:

    set e  [format "%sMeV" [lindex $list 0]]
    set label "$isotope $e"

    .select.beamselect configure -text $label


    # Extract the names of the configuration files we need to copy in.
    #

    set shaperfile [lindex $list 1]
    set cfdfile    [lindex $list 2]
    set lrhvfile   [lindex $list 3]
    set udhvfile   [lindex $list 4]
    set ppachvfile [lindex $list 5]

    # Copy these files into the configuration directory

    file copy -force $shaperfile $Configdir/shaper_defaults.shaper_values
    file copy -force $cfdfile    $Configdir/seecfd_default.cfd_settings
    file copy -force $lrhvfile   $Configdir/scintlr_hv_defaults.hv_settings
    file copy -force $udhvfile   $Configdir/scintud_hv_defaults.hv_settings
    file copy -force $ppachvfile $Configdir/ppac_hv_defaults.hv_settings


    Log "Configuration installed for $isotope [lindex $list 0]MeV"

    # Setup the CFD...

    SetupCfd
    Log "Constant fraction setup for $isotope [lindex $list 0]MeV"

    # Setup tyhe shaper...

    SetupShaper
    Log "Shaping amplifier setup for $isotope [lindex $list 0]MeV"

    SetupHv 

    Log "HV Initialized for $isotope [lindex $list 0]MeV"
    Log "HV Ready to ramp"

    # Setup emergency off on exit:

    bind . <Destroy> "Exiting %W"

}
#
#   Return the state of a check widget.
#
proc GetCheckState {widget} {
    set varname [$widget cget -variable]
    global $varname
    return [set $varname]
}
#
#  Turn off a power supply controller set.
#  This turns the associated button into an on button.
#
proc TurnOff {devices widget prefixtext} {
    global HVInfo

    # A maximum speed downward ramp is initiated to scram the HV:
   
    foreach device $devices {
	set controller $HVInfo($device.module)
	Scram $controller
    }

    # Turn tyhe button into an on button

    append label $prefixtext " On"
    $widget configure -text $label \
	    -command "Ramp [list $devices] $widget [list $prefixtext]"

}
#
#  Initiate a ramp on a set of  power supply controllers.
#  The ramp also turns the button into an off button for the devices.
#
#
proc Ramp {devices widget prefixtext} {
    global HVInfo

    puts "Ram: $devices"

    # Devices is a set of devices.  Each is started independently although
    # they are slaved to the same switch.
    #
    foreach device $devices {
	puts "initiating ramp for $device"
	set controller $HVInfo($device.module)
	set setpta     $HVInfo($device.setpoint.a)
	set setptb     $HVInfo($device.setpoint.b)
	set rampa      $HVInfo($device.rampspeed.a)
	set rampb      $HVInfo($device.rampspeed.b)
	set ilima      $HVInfo($device.ilimit.a)
	set ilimb      $HVInfo($device.ilimit.b)
	set ires       $HVInfo($device.resi)

	#  Convert the  current limits into register units

	set ilima [expr $ilima / $ires]
	set ilimb [expr $ilimb / $ires]
	
	# setup and start ramp on channel a:
	
	vhq::limit       $controller i a $ilima
	vhq::rampspeed   $controller a   $rampa
	vhq::setv        $controller a   $setpta

	# setup and start rampon channel b:

	vhq::limit       $controller i b $ilimb
	vhq::rampspeed   $controller b   $rampb
	vhq::setv        $controller b   $setptb
	

    }


    #  Turn the button into an off button.

    append label $prefixtext " Off"
    $widget configure -text $label
    $widget configure \
	    -command "TurnOff [list $devices] $widget [list $prefixtext]"
    
}
#
#  Scram both channels of a power supply back to zero.
#
proc Scram {module} {
    vhq::rampspeed $module a 255
    vhq::setv      $module a 0

    vhq::rampspeed $module b 255
    vhq::setv      $module b 0
}
#
#  Exiting is called when a widget is destroyed.  If this is the top level
#  Widget, we turn off all hv's.
#
proc Exiting {widget} {
    global HVInfo

    if {$widget == "."} {
	set controllers [array names HVInfo *.module]
	foreach controller $controllers {
	    set module $HVInfo($controller)
	    Scram $module
	}
    }
}
#
#   Called when a trip occurs.
#     Calls TurnOff for the appropriate device or pair of devices.   
#
proc Trip {device} {
    global ppaconoff
    global scintonoff

    if {$device == "ppac"} {
	TurnOff ppac $ppaconoff {PPAC Hv}
    } else {
	TurnOff {scintlr scintud} $scintonoff {Scint Hv}
    }
}
#
#  Monitor the trips for a channel
#    widget - the frame that contains the status widgets.
#    device - The vhq device.
#    channel- a or b the channel id.
#
#
proc MonitorStatus {widget device channel} {
    global HVInfo

    set dev $HVInfo($device.module)

    set stat1 [vhq::stat1 $dev]
    set stat2 [vhq::stat2 $dev]

    # Extract the apropriate channel statuses:

    if {$channel == "a"} {
	set stat1 [lindex $stat1 0]
	set stat2 [lindex $stat2 1]
    } elseif {$channel == "b"}  {                     
	set stat1 [lindex $stat1 1]
	set stat2 [lindex $stat2 2]
    } else {
	error "Invalid channel in MonitorStatus $channel"
    }
    set ramping [lindex [lindex $stat1 [lsearch -regex $stat1 ^stable]] 1]
    set ilimit  [lindex [lindex $stat2 [lsearch -regex $stat2 ^ilimit]] 1]
    set vlimit  [lindex [lindex $stat2 [lsearch -regex $stat2 ^Voverset]] 1]

    #
    #  Derive the widget names for the checkboxes.
    #
    set overv $widget.overv
    set overi $widget.overi
    set ramp  $widget.ramp
    #
    #  Get the current state of each widget:
    #
    set isoverv [GetCheckState $overv]
    set isoveri [GetCheckState $overi]
    set isramp  [GetCheckState $ramp]

    # process  ramp start/stop: 
    

    set trip 0

    if {$isramp != $ramping} {
	if {$ramping} {
	    $ramp select
	    Log "Ramp started for $device $channel"
	} else {
	    Log "Ramp finished for $device $channel"
	    $ramp deselect
	}
    }
    #   process over voltage trips:

    if {$isoverv != $vlimit} {
	if {$vlimit} {
	    set trip 1
	    $overv select
	    Log "Over voltage trip for $device $channel !!"
	} else {
	    $overv deselect
	    Log "Over voltage trip reset for $device $channel"
	}
    }
    # Process over current trips:

    if {$isoveri != $ilimit} {
	if {$ilimit} {
	    set trip 1
	    $overi select
	    Log "Over current trip for $device !!"
	} else {
	    $overi deselect
	    Log "Over current trip reset for $device $channel"
	}
    }

    # If the device tripped, turn it off.

    if {$trip} {
	Trip $device
    }
    

    # reschedule the monitor.

    after 500 "MonitorStatus $widget $device $channel"
}
#
#
#   Monitor the HV actual value for a channel.
#
#    Parameters:
#        device    - the device to monitor (name).
#        variable  - Name of variable tied to the actual reading.
#        channel   - Channel of hv unit tied to that var.
#
proc MonitorVoltage {device variable channel} {
    global HVInfo
    global $variable

    set controller $HVInfo($device.module)

    set vandi [vhq::actual $controller $channel]
    set $variable [lindex $vandi 0]

    after 500 "MonitorVoltage $device $variable $channel"
}
#
#  Reads the HV configuration and settings files and
#  sets up the interface so that the ramp buttons will work appropriately.
#
proc SetupHv {} {
    global HVInfo
    global Configdir
    global status
    global ppac-t
    global left-t
    global right-t
    global up-t
    global down-t
    global scintonoff ppaconoff

    #  Destroy the existing HVInfo elements and the devices associated with
    #  them.

    set devices [array names HVInfo *.module]
    if {$devices != ""} {
	foreach device $devices {
	    Scram $HVInfo($device)    ;# Scram the supplies to 0.
	    vhq::delete $HVInfo($device)
	}
    }
    set elements [array names HVInfo]
    if {$elements != ""} {
	foreach element $elements {
	    unset HVInfo($element)
	}
    }

    #  Setup the HV.


    foreach device {ppac scintlr scintud} {
	set filename $Configdir/
	append filename $device "_hv.cfg"
	source $filename

	set HVInfo($device.module) [vhq::create $base]   ;# Controls the module
	set HVInfo($device.setpoint.a)   $SetPoint(a)
	set HVInfo($device.setpoint.b)   $SetPoint(b)
	set HVInfo($device.rampspeed.a)  $RampSpeed(a)
	set HVInfo($device.rampspeed.b)  $RampSpeed(b)
	set HVInfo($device.ilimit.a)      $ILimit(a)
	set HVInfo($device.ilimit.b)     $ILimit(b)
	set HVInfo($device.resi)         $resi

    }
    #
    #  The ppac only monitors one channel:
    #
    MonitorStatus $status.ppac ppac a 
    MonitorVoltage ppac ppac-a a
    set ppac-t $HVInfo(ppac.setpoint.a)

    #
    #  left and right monitor both channels:
    #

    foreach item {{scintlr left right} {scintud up down} } {
	set device [lindex $item 0]
	set chan0  [lindex $item 1]
	set chan1  [lindex $item 2]

	set $chan0-t $HVInfo($device.setpoint.a)
	set $chan1-t $HVInfo($device.setpoint.b)
	MonitorStatus $status.$chan0 $device a
	MonitorVoltage $device $chan0-a a
	MonitorStatus $status.$chan1 $device b
	MonitorVoltage $device $chan1-a b
    }
    

    #  Enable the buttons to ramp:

    $scintonoff configure -state normal -text {Scint Hv On} \
	    -command "Ramp {scintlr scintud} $scintonoff {Scint Hv}"
    $ppaconoff  configure -state normal -text {PPAC Hv On} \
	    -command "Ramp ppac $ppaconoff {PPAC Hv}"
}
#
#   Setup the cfd:
#
proc SetupCfd {} {
    global Configdir
    global Distdir

    set cfddir $Distdir/controls/cfd
    catch "exec $cfddir/loadcfd.tcl $Configdir/seecfd.cfd \
	                            $Configdir/seecfd_default.cfd_settings" \
				    mymsg
    Log $mymsg
}
#
#  Setup the shapers:
#
proc SetupShaper {} {
    global Configdir
    global Distdir

    set shaperdir $Distdir/controls/shaper

    catch "exec $shaperdir/loadshaper.tcl $Configdir/shaper.cfg \
                            $Configdir/shaper_defaults.shaper_values" mymsg
    Log $mymsg
} 
#
#  Log  takes a log entry, prepends a timestamp and appends it to 
#  $logwindow.
#
proc Log {text} {
    global logwindow
    global Logfile

    append  entry [exec date] ":" $text "\n"
    $logwindow configure -state normal
    $logwindow insert end $entry
    $logwindow see    end
    $logwindow configure -state disabled

    set fd [open $Logfile a+]
    puts -nonewline $fd $entry
    close $fd

    update idletasks
    update idletasks
    update idletasks
}
#---------------------------------------------------------------
#   The index below is used to generate unique submenu names.
#

set sindex 0



source $MasterDbFile

#
#   Create a frame for the top level menu and it's label.
#
frame .select -relief ridge -borderwidth 3
label .select.l -text "Select Beam: "

#
#   Generate the top level menu button and
#   it's menu.
#
menubutton .select.beamselect -text "Select Beam" \
	                      -relief raised -borderwidth 3
set topmenu [menu .select.beamselect.species -tearoff 0]
.select.beamselect configure -menu $topmenu


set submenu $topmenu.erange	;#  the base menu entry name for energy ranges.

#
#   Now generate the cascading menu hierarchy from the database we read in:
#


set species [lsort -command SpeciesCompare [array names ConfigurationDatabase]]


#
#  species is now a 'sensibly' sorted list of indices into the 
#  ConfigurationDatabase array so that the menu will be built nicely. 

# Each isotope creates a cascade in $topmenu, and a $topmenu.erangenn menu.
# Each energy range creates a command widget in the $topmenu.erangenn menu.

foreach isotope $species {
    set erangemenu $submenu
    append erangemenu $sindex
    incr sindex

    menu $erangemenu -tearoff 0         ;# Menu for energy ranges.
    $topmenu add cascade -label $isotope -menu $erangemenu
  
    #  For each energy range, (sorted), put up a command in $erangemenu

    set elist $ConfigurationDatabase($isotope)
    set elist [lsort -command ECompare $elist]

    foreach energy $elist {

	set e     [lindex $energy 0]
	set estring [format "%sMeV" $e]

	$erangemenu add command -label $estring \
		-command "Select $isotope \"$energy\""
    }

}


pack .select
pack .select.l .select.beamselect -side left

#
#   Set up the HV  controls.  
#   These look like:
#   +---------------------------------- ---------------------------+
#   |  Scint L    Scint R   Scint U   Scint D  |       PPAC        |
#   |[] over V   [] Over V [] Over V [] Over V |   [] Over V       |
#   |[] Over I   [] Over I [] Over I [] Over I |   [] Over I       |
#   |[] Ramp     [] Ramp   [] Ramp   [] Ramp   |   [] Ramp         |
#   +--------------------------------------------------------------+
#   |Target V     Target V  Target V Target V  |   Target V        |
#   |[        ]   [       ] [      ] [      ]  |   [         ]     |
#   |Actual V     Actual V  Actual V Actual V  |   Actual V        |
#   |[        ]   [       ] [      ] [      ]  |   [         ]     |
#   +--------------------------------------------------------------+
#   |                   [  HV ON ]                   [ HV ON]      |
#   +--------------------------------------------------------------+
#   | Mon Oct 20 11:13:52 EDT 2003 Scint L Over I trip            ^|
#   | Mon Oct 20 11:30:00 EDT 2003 Scint Trip reset               ||
#   | Mon Oct 20 11:30:05 EDT 2003 Scint Ramp initiated           V|
#   +--------------------------------------------------------------+
#   
frame .pscontrols                      ;# Overall outside frame.
set status [frame .pscontrols.status]  ;# Frame for status info.
set volts  [frame .pscontrols.volts]   ;# Frame for target/actual.
set hvctl  [frame .pscontrols.control] ;# Frame with buttons.
set log    [frame .pscontrols.log]     ;# Frame with log.



$status configure -relief ridge -borderwidth 3
$volts  configure -relief ridge -borderwidth 3
$hvctl  configure -relief ridge -borderwidth 3
$log    configure -relief ridge -borderwidth 3

#   The status frame has subframes for each HV channel.  In addition,
#   The frame for the PPAC has a ridge relief.

set lstatus [frame $status.left]
set rstatus [frame $status.right]
set ustatus [frame $status.up]
set dstatus [frame $status.down]
set pstatus [frame $status.ppac -relief ridge -borderwidth 3]

#  Each status frame has a title, and an indicicator
#  for over V, Over I and ramping.
#

foreach item {{"Scint L" lstatus left} \
	       {"Scint R" rstatus right} \
	       {"Scint U" ustatus up} \
	       {"Scint D" dstatus down} \
	       {"PPAC"    pstatus ppac} } {
   set title [lindex $item 0]
   set frame [lindex $item 1]
   eval set frame $$frame
   set varprefix [lindex $item 2]

   label $frame.title -text $title
   checkbutton $frame.overv -text "Over V" -variable $varprefix-v \
	   -state disabled
   checkbutton $frame.overi -text "Over I" -variable $varprefix-i \
	   -state disabled
   checkbutton $frame.ramp  -text "Ramp"   -variable $varprefix-r \
	   -state disabled
    
}

#
#   The volts frame has subframes for each HV channel.
#

set lvolts [frame $volts.left]
set rvolts [frame $volts.right]
set uvolts [frame $volts.up]
set dvolts [frame $volts.down]
set pvolts [frame $volts.ppac -relief ridge -borderwidth 3]

#  Each of those frames has labels for the titles and values
#  of the target and actual voltages.
#


foreach item { {left lvolts} \
               {right rvolts} \
               {up uvolts}    \
	       {down dvolts}  \
	       {ppac pvolts} }  {
	   
   set frame [lindex $item 1]
   eval set frame $$frame
   set varprefix [lindex $item 0]
   
   label $frame.tlabel -text "Set V"
   label $frame.tvalue -textvariable $varprefix-t
  
   label $frame.alabel -text "Read V"
   label $frame.avalue -textvariable $varprefix-a
	   
}


#
#    The hvctl frame has a pair of subframes to be sure that
#    the buttons get positioned correctly:
#

frame $hvctl.scint -relief ridge -borderwidth 3
frame $hvctl.ppac  -relief ridge -borderwidth 3

set scintonoff [button $hvctl.scint.onoff -text "Scint Hv On" \
	       -state disabled]
set ppaconoff  [button $hvctl.ppac.onoff  -text "PPAC Hv On"  \
	        -state disabled]


#  The bottom frame (log) contains a text widget of a few
#  lines tall and a vertical scroll bar that knows how to 
#  scroll the text up and down.
#


set logwindow [text $log.log \
	-yscrollcommand "$log.scroll  set" \
	-height 10 -state disabled]
scrollbar $log.scroll -command "$log.log yview"

#
#  Now laboriously pack this all to lay it out.
#

pack .pscontrols

#
#   Pack the status indicators:

pack $status -side top -fill x -expand 1

foreach frame "$lstatus $rstatus $ustatus $dstatus $pstatus" {
    pack $frame -side left -fill x -expand 1
    pack $frame.title $frame.overv $frame.overi $frame.ramp \
	    -side top -anchor w 
}


#
#   Pack the voltage frames:
#

pack $volts -side top -fill x -expand 1

foreach frame "$lvolts $rvolts $uvolts $dvolts $pvolts" {
    pack $frame -side left -fill x -expand 1

    pack $frame.tlabel $frame.tvalue $frame.alabel $frame.avalue \
	    -side top -anchor w -fill x -expand 1
}

#
#  Pack the buttons that control the HV ramps.  These are packed
#  Centered in their frames.
#
 
pack $hvctl -side top -fill x -expand 1
pack $hvctl.scint -side left -fill x -expand 1
pack $hvctl.ppac  -side right 


pack $hvctl.scint.onoff -anchor c -side top
pack $hvctl.ppac.onoff  -anchor c -side right




#
#  Pack the log window.
#

pack $log -fill y -expand 1
pack $log.scroll -side right -fill y -expand 1
pack $logwindow -fill y -expand 1

