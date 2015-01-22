# interface generated by SpecTcl version 1.1 from C:/Documents and Settings/fox/Desktop/vhqpanel.ui
#   root     is the parent window for this user interface

set answer [tk_messageBox -title Obsolete -icon info -type okcancel \
    -message {
You are using an obsolete version of the VHQ 
control panel. The up-to-date version is
currently located in the Scripts/ControlApps directory of your
system's daq distribution.  If you want to continue, click ok below}]

if {$answer eq "cancel" } exit 

proc vhqpanel_ui {root args} {

	# this treats "." as a special case

	if {$root == "."} {
	    set base ""
	} else {
	    set base $root
	}
    
	frame $base.frame#1

	frame $base.frame#5

	frame $base.frame#6

	label $base.label3 \
		-text SerialNumber

	label $base.serialno

	label $base.label1 \
		-text {VHQ 204x Power supply Controller}

	label $base.label7 \
		-text {Channel A}

	label $base.lockstate

	label $base.label#13 \
		-text {Channel B}

	checkbutton $base.charamp \
		-state disabled \
		-text ramp \
		-variable checkbutton15

	checkbutton $base.chamanual \
		-state disabled \
		-text {man.  } \
		-variable checkbutton11

	checkbutton $base.chaon \
		-state disabled \
		-text on \
		-variable checkbutton13

	checkbutton $base.chakill \
		-state disabled \
		-text {kill en} \
		-variable checkbutton14

	checkbutton $base.chbramp \
		-state disabled \
		-text ramp \
		-variable checkbutton23

	checkbutton $base.chbmanual \
		-state disabled \
		-text {man.  } \
		-variable checkbutton27

	checkbutton $base.chbon \
		-state disabled \
		-text on \
		-variable checkbutton25

	checkbutton $base.chbkill \
		-state disabled \
		-text {kill en} \
		-variable checkbutton24

	checkbutton $base.chailimit \
		-state disabled \
		-text {I limit} \
		-variable checkbutton18

	checkbutton $base.chavlimit \
		-state disabled \
		-text {V Limit} \
		-variable checkbutton19

	checkbutton $base.chastable \
		-state disabled \
		-text {stable } \
		-variable checkbutton16

	checkbutton $base.chazero \
		-state disabled \
		-text 0 \
		-variable checkbutton10

	checkbutton $base.chbilimit \
		-state disabled \
		-text {I limit} \
		-variable checkbutton21

	checkbutton $base.chbvlimit \
		-state disabled \
		-text {V Limit} \
		-variable checkbutton20

	checkbutton $base.chbstable \
		-state disabled \
		-text {stable } \
		-variable checkbutton22

	checkbutton $base.chbzero \
		-state disabled \
		-text 0 \
		-variable checkbutton28

	label $base.label12 \
		-text {V limit (V)}

	label $base.label8 \
		-text {Current uA}

	label $base.label#16 \
		-text {V limit (V)}

	label $base.label#14 \
		-text {Current (uA)}

	entry $base.chavlimitvalue \
		-cursor {} \
		-state disabled

	entry $base.chacurrent \
		-cursor {} \
		-state disabled

	entry $base.chbvlimitvalue \
		-cursor {} \
		-state disabled

	entry $base.chbcurrent \
		-cursor {}

	label $base.label#19 \
		-text {Ramp Speed (V/sec)}

	label $base.label11 \
		-text {I limit set (uA)}

	label $base.label#20 \
		-text {Ramp Speed (V/sec)}

	label $base.label#15 \
		-text {I limit set (uA)}

	entry $base.chaspeed \
		-cursor {}

	entry $base.chailimitvalue \
		-cursor {} \
		-textvariable entry

	entry $base.chbspeed \
		-cursor {}

	entry $base.chbilimitvalue \
		-cursor {}

	button $base.chasetspeed \
		-command "SetRampSpeed  $base.chasetspeed a" \
		-text Set

	button $base.chasetilimit \
		-command "SetIlimit $base.chasetilimit a" \
		-text Set

	button $base.chbsetspeed \
		-command "SetRampSpeed $base.chbsetspeed b" \
		-text Set

	button $base.chbsetilimit \
		-command "SetIlimit $base.chbsetilimit b" \
		-text Set

	label $base.label#17 \
		-text {Set Point}

	label $base.label#22 \
		-text {Actual (V)}

	label $base.label#18 \
		-text {Set Point}

	label $base.label#23 \
		-text {Actual (V)}

	entry $base.chasetpoint \
		-cursor {}

	entry $base.chavactual \
		-cursor {} \
		-state disabled

	entry $base.chbsetpoint \
		-cursor {}

	entry $base.chbvactual \
		-cursor {}

	button $base.chastartramp \
		-command "StartRamp $base.chastartramp a" \
		-text Ramp

	button $base.button#13 \
		-command "TurnOff $base.button#13 a" \
		-text Off

	button $base.chbstartramp \
		-command "StartRamp $base.chbstartramp b" \
		-text Ramp

	button $base.button#14 \
		-command "TurnOff $base.button#14 b" \
		-text Off

	button $base.save \
		-command "SaveSettings $base.save" \
		-text Save...

	button $base.button#15 \
		-command "RestoreSettings $base.button#15" \
		-text Restore...

	button $base.button#11 \
		-command {after 2000} \
		-text Reset

	button $base.exit \
		-command Exit \
		-text Exit

	button $base.lock \
		-command "ToggleLock $base.lock" \
		-text Lock


	# Geometry management

	grid $base.frame#1 -in $root	-row 1 -column 1  \
		-columnspan 2
	grid $base.frame#5 -in $root	-row 3 -column 1  \
		-columnspan 2
	grid $base.frame#6 -in $root	-row 3 -column 4  \
		-columnspan 2 \
		-sticky n
	grid $base.label3 -in $root	-row 1 -column 4 
	grid $base.serialno -in $root	-row 1 -column 5 
	grid $base.label1 -in $base.frame#1	-row 1 -column 1 
	grid $base.label7 -in $root	-row 2 -column 1  \
		-columnspan 2
	grid $base.lockstate -in $root	-row 2 -column 3 
	grid $base.label#13 -in $root	-row 2 -column 4  \
		-columnspan 2
	grid $base.charamp -in $base.frame#5	-row 1 -column 1 
	grid $base.chamanual -in $base.frame#5	-row 1 -column 2 
	grid $base.chaon -in $base.frame#5	-row 1 -column 3  \
		-sticky w
	grid $base.chakill -in $base.frame#5	-row 1 -column 4  \
		-columnspan 2
	grid $base.chbramp -in $base.frame#6	-row 1 -column 2 
	grid $base.chbmanual -in $base.frame#6	-row 1 -column 3 
	grid $base.chbon -in $base.frame#6	-row 1 -column 4  \
		-sticky w
	grid $base.chbkill -in $base.frame#6	-row 1 -column 5  \
		-columnspan 2
	grid $base.chailimit -in $base.frame#5	-row 2 -column 1 
	grid $base.chavlimit -in $base.frame#5	-row 2 -column 2 
	grid $base.chastable -in $base.frame#5	-row 2 -column 3 
	grid $base.chazero -in $base.frame#5	-row 2 -column 4  \
		-sticky w
	grid $base.chbilimit -in $base.frame#6	-row 2 -column 2 
	grid $base.chbvlimit -in $base.frame#6	-row 2 -column 3  \
		-sticky n
	grid $base.chbstable -in $base.frame#6	-row 2 -column 4 
	grid $base.chbzero -in $base.frame#6	-row 2 -column 5  \
		-sticky w
	grid $base.label12 -in $root	-row 4 -column 1 
	grid $base.label8 -in $root	-row 4 -column 2 
	grid $base.label#16 -in $root	-row 4 -column 4 
	grid $base.label#14 -in $root	-row 4 -column 5 
	grid $base.chavlimitvalue -in $root	-row 5 -column 1 
	grid $base.chacurrent -in $root	-row 5 -column 2 
	grid $base.chbvlimitvalue -in $root	-row 5 -column 4 
	grid $base.chbcurrent -in $root	-row 5 -column 5 
	grid $base.label#19 -in $root	-row 6 -column 1 
	grid $base.label11 -in $root	-row 6 -column 2 
	grid $base.label#20 -in $root	-row 6 -column 4 
	grid $base.label#15 -in $root	-row 6 -column 5 
	grid $base.chaspeed -in $root	-row 7 -column 1 
	grid $base.chailimitvalue -in $root	-row 7 -column 2 
	grid $base.chbspeed -in $root	-row 7 -column 4 
	grid $base.chbilimitvalue -in $root	-row 7 -column 5 
	grid $base.chasetspeed -in $root	-row 8 -column 1 
	grid $base.chasetilimit -in $root	-row 8 -column 2 
	grid $base.chbsetspeed -in $root	-row 8 -column 4 
	grid $base.chbsetilimit -in $root	-row 8 -column 5 
	grid $base.label#17 -in $root	-row 9 -column 1 
	grid $base.label#22 -in $root	-row 9 -column 2 
	grid $base.label#18 -in $root	-row 9 -column 4 
	grid $base.label#23 -in $root	-row 9 -column 5 
	grid $base.chasetpoint -in $root	-row 10 -column 1 
	grid $base.chavactual -in $root	-row 10 -column 2 
	grid $base.chbsetpoint -in $root	-row 10 -column 4 
	grid $base.chbvactual -in $root	-row 10 -column 5 
	grid $base.chastartramp -in $root	-row 11 -column 1 
	grid $base.button#13 -in $root	-row 11 -column 2 
	grid $base.chbstartramp -in $root	-row 11 -column 4 
	grid $base.button#14 -in $root	-row 11 -column 5 
	grid $base.save -in $root	-row 12 -column 1 
	grid $base.button#15 -in $root	-row 12 -column 2 
	grid $base.button#11 -in $root	-row 12 -column 3 
	grid $base.exit -in $root	-row 12 -column 4 
	grid $base.lock -in $root	-row 12 -column 5 

	# Resize behavior management

	grid rowconfigure $base.frame#6 1 -weight 0 -minsize 30
	grid rowconfigure $base.frame#6 2 -weight 0 -minsize 30
	grid rowconfigure $base.frame#6 3 -weight 0 -minsize 2
	grid columnconfigure $base.frame#6 1 -weight 0 -minsize 2
	grid columnconfigure $base.frame#6 2 -weight 0 -minsize 30
	grid columnconfigure $base.frame#6 3 -weight 0 -minsize 30
	grid columnconfigure $base.frame#6 4 -weight 0 -minsize 30
	grid columnconfigure $base.frame#6 5 -weight 0 -minsize 30
	grid columnconfigure $base.frame#6 6 -weight 0 -minsize 2
	grid columnconfigure $base.frame#6 7 -weight 0 -minsize 2

	grid rowconfigure $root 1 -weight 0 -minsize 2
	grid rowconfigure $root 2 -weight 0 -minsize 30
	grid rowconfigure $root 3 -weight 1 -minsize 2
	grid rowconfigure $root 4 -weight 0 -minsize 30
	grid rowconfigure $root 5 -weight 0 -minsize 30
	grid rowconfigure $root 6 -weight 0 -minsize 30
	grid rowconfigure $root 7 -weight 0 -minsize 30
	grid rowconfigure $root 8 -weight 0 -minsize 30
	grid rowconfigure $root 9 -weight 0 -minsize 30
	grid rowconfigure $root 10 -weight 0 -minsize 30
	grid rowconfigure $root 11 -weight 0 -minsize 30
	grid rowconfigure $root 12 -weight 0 -minsize 30
	grid columnconfigure $root 1 -weight 0 -minsize 30
	grid columnconfigure $root 2 -weight 0 -minsize 2
	grid columnconfigure $root 3 -weight 0 -minsize 2
	grid columnconfigure $root 4 -weight 0 -minsize 51
	grid columnconfigure $root 5 -weight 0 -minsize 2

	grid rowconfigure $base.frame#5 1 -weight 0 -minsize 30
	grid rowconfigure $base.frame#5 2 -weight 0 -minsize 30
	grid rowconfigure $base.frame#5 3 -weight 0 -minsize 2
	grid columnconfigure $base.frame#5 1 -weight 0 -minsize 30
	grid columnconfigure $base.frame#5 2 -weight 0 -minsize 30
	grid columnconfigure $base.frame#5 3 -weight 0 -minsize 30
	grid columnconfigure $base.frame#5 4 -weight 0 -minsize 30
	grid columnconfigure $base.frame#5 5 -weight 0 -minsize 19
	grid columnconfigure $base.frame#5 6 -weight 0 -minsize 2

	grid rowconfigure $base.frame#1 1 -weight 0 -minsize 30
	grid columnconfigure $base.frame#1 1 -weight 0 -minsize 51
# additional interface code
 package require vhq
 package require vhq

proc min {a b} {
  return [expr $a < $b ? $a : $b]
}
#
#  Procedure to modify the state of a 'disabled' checkbox
#  disabled checkboxes are used to reflect the status of
#  the module.  To modify them they must be enabled,
#  set and disabled.
#  Parameters:
#      widget   - Widget to modify.
#      state    - New value of checkbox 1 - on 0 - off.
#
#
proc SetCheckBox {widget state} {

	
   $widget configure -state normal 
   if {$state} {
	$widget select
   } else {
	$widget deselect
   }
   $widget configure -state disabled
}
#
#   Sets the text of a readonly  entry widget
#
proc SetEntry {widget value} {
   $widget configure -state normal

   $widget delete 0 end
   $widget insert end $value  

   $widget configure -state disabled
}
#
#  Get the description for a module.
#
proc GetDescription {name} {
   global VhqDevices
   return $VhqDevices($name)
}
# 
#   Given  a widget name returns the device name.
#
proc DevNameFromWidget {w} {
   return [lindex [split $w .] 1]
}
#
#  Given a widget name, returns the device 
#
proc DevFromWidget {w} {
   set name [DevNameFromWidget $w]
   set desc [GetDescription $name]
   return [lindex $desc 4]
}
#
#  Set the serialno widget for a named device.
#
proc SetSerial {name} {
   set description [GetDescription $name]
   set device [lindex $description 4]
   set serial [vhq::id $device]

   .$name.serialno configure -text [format %x $serial] ;# Bcd.
}
#
#  Set the module textual label/name.
#
proc SetLabel {name} {
   set description [GetDescription $name]
   set label [lindex $description 3]

    wm title .$name $label
  #  .$name.psname configure -text $label
}

#
#   Update status indicators for a device's
#   status1 list.
#
proc UpdateStat1 {name chan status}  {
   append zero .$name. ch $chan zero
   SetCheckBox $zero [lindex [lindex $status 0] 1]

   append manual .$name. ch $chan manual
   SetCheckBox $manual [lindex [lindex $status 1] 1]

   # append plus .$name. ch $chan plus
   # SetCheckBox $plus [lindex [lindex $status 2] 1]

   append on  .$name. ch $chan on
   SetCheckBox $on [expr !([lindex [lindex $status 3] 1])]

   append kill .$name. ch $chan kill
   SetCheckBox $kill [lindex [lindex $status 4] 1]

   append ramp .$name. ch $chan ramp
   SetCheckBox $ramp [lindex [lindex $status 5] 1]

   append stable .$name. ch $chan stable
   SetCheckBox $stable [expr !([lindex [lindex $status 6] 1])]

}

#
#  Udate the indicators (I limit and V limit) from the
#  stat2 register:
#
proc UpdateStat2 {name chan status} {
  append ilimit .$name. ch $chan ilimit
  SetCheckBox $ilimit [lindex [lindex $status 0] 1]

  append vlimit .$name. ch $chan vlimit
  SetCheckBox $vlimit [lindex [lindex $status 3] 1]
}
#
#   Update the actual voltage/current indicators.
#
proc UpdateActual {name chan values} {
   set Description [GetDescription $name]
   set ires [lindex $Description 5]

   set voltage [lindex $values 0]
   set current [lindex $values 1]

  

   append vwidget .$name. ch $chan vactual
   SetEntry $vwidget $voltage

    # current must be converted to uA

   set current [expr $current*$ires]

   append iwidget .$name. ch $chan current
   SetEntry $iwidget $current
  
}

#
#   Monitor the device status information for a module
#
proc MonitorModule {name} {
   set description [GetDescription $name]
   set device [lindex $description 4]
  
   # Update the status indicators.
  
   set stat1 [vhq::stat1 $device]
   UpdateStat1 $name a [lindex $stat1 0]
   UpdateStat1 $name b [lindex $stat1 1]

   set stat2 [vhq::stat2 $device]
   UpdateStat2 $name a [lindex $stat2 1]
   UpdateStat2 $name b [lindex $stat2 2]

   # Update current/voltage indicators.

   set avi [vhq::actual $device a] 
   set bvi [vhq::actual $device b]
   UpdateActual $name a $avi
   UpdateActual $name b $bvi

   after 300 "MonitorModule $name"
}
#
#   Set the value of the ramp speed widget with
#  The requested value
#
proc RampSpeed {name channel value} {
   append entry .$name. ch$channel speed
   
  $entry delete 0 end
  $entry insert end $value

   SetRampSpeed $entry $channel
}
#
#   Sets the ramp speed entry with the current
#   value of the channel's ramp speed.
#
proc DisplayRampSpeed {name channel} {
   set description [GetDescription $name]
   set device      [lindex $description 4]


   set speed [vhq::rampspeed $device $channel]
   
   append entry .$name. ch$channel speed
   $entry delete 0 end
   $entry insert end $speed
}
#
#   Sets the setpoint widget of a channel as
#  desired by the paramters
#
proc SetPoint {name channel value} {
   append entry .$name.ch$channel setpoint
   $entry delete 0 end
   $entry insert end $value
}
#
#  Sets the set point widget with the value of the
#  current set point as read from the module.
#
proc DisplaySetPoint {name channel} {
   set description [GetDescription $name]
   set device      [lindex $description 4]
   set v        [vhq::setv $device $channel]

   SetPoint $name $channel $v
   
}
#
#   Displays the current and voltage limit
#   set in the specified channel
#   Note that the hard limits are stored in the module
#   in units of 10% of full scale, but are
#   displayed/entered in V and uA
#   The soft limits are stored and displayed in uA.
#

proc DisplayLimits {name channel} {
   set Description [GetDescription $name]
   set maxv        [lindex $Description 1]
   set maxi        [lindex $Description 2]
   set device      [lindex $Description 4]
   set ires        [lindex $Description 5]

   set vlimit [vhq::limit  $device v $channel]
   set ilimit [vhq::limit  $device i $channel]

   # I Limit is the min of the hardware/software limit.
   # unless the soft limit is 0 in which case it's the
   # hard limit.

   set slimit [lindex $ilimit 0]
   set hlimit [lindex $ilimit 1]

   # conver the current limits to  physical units (uA).
 
   set slimit [expr $slimit*$ires]
   set hlimit [expr $hlimit * (0.1*$maxi)]

   if {$slimit != 0} {
      set ilimit [min $slimit $hlimit]
   } else {
      set ilimit $hlimit
   }
 
   # Convert V to physical units.

   set vlimit [expr $vlimit * (0.1*$maxv)]

   # Last figure out which entries to put them in and
   # set the values..

   append ientry .$name. ch$channel ilimitvalue
   append ventry .$name. ch$channel vlimitvalue

   $ientry delete 0 end
   $ientry insert end $ilimit
   
   SetEntry $ventry $vlimit
   
}
#
#  Set the software current limit
#  Since the current limit set by the software
#  can be overridden by the hardware limit
#  we need to do a bit of work.
#   Cases:
#     - I limit is set to 0: the hard limit is unconditionally
#       used.
#     - I limit > hardware limit, the hardware limit is used.
#     - ILimit < hardware limit, the soft limit is used.
#
#  Note as well that he hardware registers are programmed
#  in 10% of full scale units, but we accept the input
#  values as uAmps.
#  Parameters:
#      Widget  - The widget that's requesting the change.
#                (this will give us the power supply name).
#      channel  - a|b the channel to set.
#
proc SetIlimit {widget channel} {
   set name        [DevNameFromWidget $widget]
   set Description [GetDescription $name]
   set maxi        [lindex $Description 2]
   set device      [lindex $Description 4]
   set ires        [lindex $Description 5]

    # get the hardware limit now.

   set now [vhq::limit $device i $channel]
   set ihard [lindex $now 1]  
   set ihard [expr $ihard * (0.1*$maxi)]

    # get the requested current limit:
    # and convert it to register values.



    append ientry .$name. ch$channel ilimitvalue
    set requested [$ientry get]
    if {$requested == ""} {
      set requested 0
    }

    # Inform the user if they're shooting too high:

    if {$requested > $ihard} {
       tk_dialog .ilimit "Ilimit too big" \
                 "Software limit overridden by hardware limit" \
                  info 0 Dismiss
       set requested 0
       
    } 
    set requested [expr int($requested/$ires)]
    
    vhq::limit $device i $channel $requested

    DisplayLimits $name $channel
   
}
#
#   Set the ilimit from a particular value
#
proc Ilimit {name channel value}  {
	
   append entry .$name. ch$channel ilimitvalue
   $entry delete 0 end
   $entry insert end $value
   
   SetIlimit $entry $channel
}
#
#   Set the ramp speed for a channel in the module.
#   Parameters:
#      Widget - name of the widget requesting this.
#               this is required to be able to get the name of the
#               Power supply since the widget is named .$name.yada...
#      channel - a or b the channel name.
#
proc SetRampSpeed {Widget channel} {
   set device [DevFromWidget $Widget]
   set name   [DevNameFromWidget $Widget]

   append entry .$name. ch$channel speed
   set speed [$entry get]
   if {$speed == ""} {
    set speed 0
   }
   vhq::rampspeed $device $channel $speed
   
}
#
#   Start a ramp.
#   Parameters:
#      Widget  - name of the button starting the ramp.  This is used
#                to get the device name, and widget name of the 
#                setpoint value.
#      channel - a/b the name of the channel to ramp.
#
proc StartRamp {Widget channel} {
   set device [DevFromWidget $Widget]
   set name   [DevNameFromWidget $Widget]

   append entry .$name. ch$channel setpoint
   set target [$entry get]
   if {$target == ""} {
     set target 0
   }  

   vhq::setv $device $channel $target
}
set VhqDevices("") ""

#
#  Saves the values for a channel.  These include:
#   - Channel setpoint
#   - Channel Ramp speed
#   - Channel Current limit.
#
proc SaveValues {fd name chan} {
	append rampentry .$name. ch$chan speed
	set speed [$rampentry get]
	
	append limitentry .$name. ch$chan ilimitvalue
	set ilimit [$limitentry get]
	
	append setpointentry .$name. ch$chan setpoint
	set setpoint [$setpointentry get]
	
	puts $fd "set SetPoint($chan)      $setpoint  ;# $chan setpoint (V)"
	puts $fd "set RampSpeed($chan)  $speed     ;# $chan Ramp speed (10V/s)"
	puts $fd "set ILimit($chan)          $ilimit       ;# $chan Current limit (uA)"
}
#
#  Saves the settings for the supplied device name to
#  the file supplied.
#
proc SaveSettingsTo {name  file} {
   if {[catch "open $file w" fd]  != 0 } {
       tk_dialog .fopenerror "File open error" \
            "Unable to open $name : $fd"        \
	    error 0 Dismiss
   } else {
      set d [GetDescription $name]
      
      puts $fd "# Configuration file saved: [exec date] "
      puts $fd "#   VHQ power supply: [lindex $d 3] \n"
      puts $fd "#      Device configuration was\n#"
      puts $fd "#  Short name:    $name "
      puts $fd "#  VME Crate:      [lindex $d 6]"
      puts $fd "#  Base Address:  [lindex $d 0]"
      puts $fd "#  Max Voltage:   [lindex $d 1]  V"
      puts $fd "#  Max Current    [lindex $d 2] uA"
      puts $fd "#  Description     [lindex $d 3]"
      puts $fd "#  Current Res.   [lindex $d 5] uA"
      puts $fd ""
      
      puts $fd "set name $name"
      puts $fd "set crate [lindex $d 6]"
      puts $fd "set base [lindex $d 0]"
      puts $fd "set maxv [lindex $d 1]"
      puts $fd "set maxi [lindex $d 2]"
      puts $fd "set description \"[lindex $d 3]\""
      puts $fd "set resi [lindex $d 5]"
      
      SaveValues $fd $name a          ;# Save channel A values
      SaveValues $fd $name b          ;# Save channel B values.
      
      close $fd
   }

}

#  Prompts for a filename and saved the settings for this control panel
#  into this file.
#  Note that the actual save is done with a different function
#  in order to support automated timed saves.
#
proc SaveSettings {widget} {
   set name        [DevNameFromWidget $widget]
   set description [GetDescription $name]
   set comment     [lindex $description 3]
   
   set filename [tk_getSaveFile                  \
			    -defaultextension  .hv_settings    \
			    -initialfile $name.hv_settings        \
			    -title "Save file for $comment" \
                                  -filetypes {              \
					{{HV Settings}  {.hv_settings} }  \
                                        {{All Files}     *      }      \
                                 }  ]
 
   if {$filename != ""} {
      SaveSettingsTo $name $filename
   }
   
}
#
#  Evidently, checkuttons always have variables associated with them
#  even if you tell them not to.  The procedure below
#  uniquifies the names of these variables by programming them to
#  $name_$widgetname
#
proc AdjustCheckboxVariables name {
    foreach channel {a b} {
       foreach box {zero  manual kill ramp stable on \
                          ilimit vlimit} {
	    set widget .$name.ch
	    append widget $channel $box
	    set var $name
	    append var _ch $channel $box
	    $widget configure -variable $var
        }
     }
}     

proc DefineVhq {name} {
   global VhqDevices
   global $name

    # A VHQ device will be represented
    # by a list of its characteristics.
    # in addition, a vhq device gets made.

    set cratenum [eval {set $name\(crate)}]

    lappend VhqDevices($name) [eval {set $name\(base)}]
    lappend VhqDevices($name) [eval {set $name\(maxv)}]
    lappend VhqDevices($name) [eval {set $name\(maxi)}]
    lappend VhqDevices($name) [eval {set $name\(name)}]
  
    lappend VhqDevices($name) [vhq::create                \
                               [lindex $VhqDevices($name) 0] \
			       $cratenum]
    lappend VhqDevices($name) [eval {set $name\(resi)}]
    lappend VhqDevices($name) $cratenum
    
    SetSerial $name
    SetLabel  $name
    MonitorModule $name

    # Read and display values of programmable settings/

    DisplayRampSpeed $name a
    DisplayRampSpeed $name b
    DisplaySetPoint  $name a
    DisplaySetPoint  $name b
    DisplayLimits    $name a
    DisplayLimits    $name b

    AdjustCheckboxVariables $name
    
    MaintainFailsafe $name                  ;# Keep failsafe file periodically.
}
#
#  Exit if confirmed...
#
proc Exit {} {
   set ok [tk_dialog .confirm "Ok to exit"  \
	"Are you sure you want to exit?"    \
	questhead 1 Yes No]
   if {$ok == 0} exit
}
#
#
#  Set the lock state.
#
proc SetLockState {name state text} {
	.$name.lock configure -text $text
	
	foreach channel {a b} {
	   foreach item {speed setspeed ilimitvalue setilimit setpoint startramp} {
	      set widget .$name.ch$channel
	      append widget $item
	      $widget configure -state $state
           }
        }
}
#
#   Toggle the lock state from what it is now.
#
proc ToggleLock {widget} {
   set name [lindex [split $widget .] 1]
   
   set state [.$name.lock cget -text]
   
   if {$state == "Lock"} {
      SetLockState $name disabled Unlock
      .$name.lockstate configure -text "Locked!!" -fg red
   } else {
      SetLockState $name normal Lock
      .$name.lockstate configure -text ""
   }
   
}
#
#  Used to restore the ramp speed when the channel goes to zero.
#
proc RestoreRamp {dev channel speed} {

	set status [vhq::stat1 $dev]
	if {$channel == "a" } {
	   set idx 0
        } else {
           set idx 1
        }
	set chanstat [lindex $status $idx]
	set zstat [lindex $chanstat 0]
	set stat [lindex $zstat 1]
	
	if {$stat == 1} {                ;# Reached zero
	    vhq::rampspeed $dev $channel $speed
        } else {
            after 500 "RestoreRamp $dev $channel $speed"
      }	    

}
#
#  Turn off a channel. This is done by:
#  - saving the current ramp speed.
#  - Setting a ramp speed of 255
#  - Starting a ramp -> 0.
#  
#  when zero is reached the ramp speed is set back the way it was.
#  Parameters: 
#   widget  - widget requesting the ramp.
#   channel - a or b.
#
proc TurnOff {widget channel} {
    set dev [DevFromWidget $widget]
    
    set oldspeed [vhq::rampspeed $dev $channel]
    vhq::rampspeed $dev $channel 255
    
    vhq::setv $dev $channel 0
    
    RestoreRamp $dev $channel $oldspeed
}
#
#   Maintain a failsafe settings file for a module
#   Every 10 seconds or so the settings for the channel get written
#   to the file $name_hv_failsafe.hv_settings.
#
proc MaintainFailsafe {name} {
	append filename $name _hv_failsafe.hv_settings
	SaveSettingsTo $name $filename

	 # reschedule myself.

	after 10000 "MaintainFailsafe $name"
}

#
#   Procedure to restore the settings associated with a
#   widget from a given file.
#
proc ReadSettings {name filename} {
	#  Set the defaults for the various values.
	#  from the current configuration.
	
	foreach channel {a b} {
	#                              widget   arrayname
	    foreach pair { {setpoint SetPoint} {speed RampSpeed} \
	                         {ilimitvalue Ilimit}} {
		set widget .$name.ch$channel
		append widget [lindex $pair 0]
		set value [$widget get]
		set [lindex $pair 1]($channel) $value
	     }
        }
	
	# Source the file
	
	set status [catch "source $filename" fd]
	if {$status == 1} {
	   tk_dialog .sourcefailed "source failure" \
		   "Unable to source $filename : $fd" \
		   error 0 "Dismiss"
		   return
	}
	#  We should have SetPoint(a|b)
	#                               RampSpeed(a|b)
	#                               ILimit(a|b).
	#
	foreach channel {a b} {
	     SetPoint     $name $channel $SetPoint($channel)
	     RampSpeed $name $channel $RampSpeed($channel)
	     Ilimit         $name $channel $Ilimit($channel)
	     
        }
	
		
}
#
#  Procedure to restore the settings associated with a 
#  widget (the restore button).
#
proc RestoreSettings {widget} {
	set name   [DevNameFromWidget $widget]
        set description [GetDescription $name]
        set comment     [lindex $description 3]

	set filename [tk_getOpenFile  \
			    -defaultextension  .hv_settings    \
			    -initialfile $name.hv_settings        \
			    -title "Read settings file for $comment" \
                                  -filetypes {              \
					{{HV Settings}  {.hv_settings} }  \
                                        {{All Files}     *      }      \
                                 }  ]
				 
	if {$filename != ""} {
	     ReadSettings $name $filename
     }
	
}

# end additional interface code

}


# Allow interface to be run "stand-alone" for testing

catch {
    if [info exists embed_args] {
	# we are running in the plugin
	vhqpanel_ui .
    } else {
	# we are running in stand-alone mode
	if {$argv0 == [info script]} {
	    wm title . "Testing vhqpanel_ui"
	    vhqpanel_ui .
	}
    }
}
