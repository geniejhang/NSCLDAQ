#===================================================================
# class AGD16XLM72Panel
#===================================================================

package provide gd16xlm72panel 1.0

package require Itcl
package require snit
package require usbcontrolclient

itcl::class AGD16XLM72Panel {

  private variable name
  private variable proxy

	constructor {widgetname modulename host port} {
    set name $widgetname
    set proxy [::AGD16XLM72PanelProxy %AUTO% -host $host \
                                              -port $port \
                                              -name $modulename \
                                              -widget $this]
    InitializeArray 
    SetupGUI $name
    UpdateGUIGD16
  }

	public method InitializeArray {}
	public method SetupGUI {name} 
	public method UpdateGUIGD16 {}
	public method CheckModuleGD16 {}
	public method GetModuleGD16 {}
	public method PutModuleGD16 {}
	public method ReadFileGD16 {}
	public method WriteFileGD16 {}
	public method SetInspect {}
	public method SetBypass {}
	public method DrawEntryGD16 {c x y id var label color}
	public method IncrementGD16 {var}
	public method DecrementGD16 {var}
	public method StopRepeatGD16 {}
	public method LockGD16 {}
	public method UnlockGD16 {}

  public method SetConfigFileName {path}
  public method OnDtChanged {name1 name2 op}
  public method NeedsUpdate {name1 name2 op}
  public method UpdateCalibratedValues {}
}


# The following methods are adapted from the original gd16.tcl code
itcl::body AGD16XLM72Panel::InitializeArray {} {
    global gd16

    set gd16(dt) 1
    for {set ch 1} {$ch <= 16} {incr ch} {
      set gd16(dw$ch) 0
      set gd16(delay$ch) 0
      set gd16(width$ch) 0
      trace add variable ::gd16([format "delay%d" $ch]) write [list $this NeedsUpdate]
      trace add variable ::gd16([format "width%d" $ch]) write [list $this NeedsUpdate]
    }

    set gd16(bypass)  0
    set gd16(inspect) 0
    set gd16(locked)  0

    trace add variable ::gd16(dt) write [list $this NeedsUpdate] 

  }


itcl::body AGD16XLM72Panel::SetupGUI {name} {
	global gd16
	
  ttk::frame $name 

  set parent [winfo parent $name]
	wm title $parent "XLM72 16 channel Gate & Delay Generator GUI"
	set gd16(w) $name
	
	set cb lightblue
	frame $gd16(w).side -background $cb
	set w $gd16(w).side.command
	frame $w -borderwidth 2 -relief groove -background $cb
	button $w.get -text Get -command "$this GetModuleGD16" -background $cb
	button $w.put -text Put -command "$this PutModuleGD16" -background $cb
	grid $w.get -sticky news
	grid $w.put -sticky news
  grid columnconfigure $w 0 -weight 1
	pack $w -side left -expand 1 -fill x
	
	set w $gd16(w).side.file
	set cb lightblue
	frame $w -borderwidth 2 -relief groove -background $cb
	label $w.filelabel -text "Configuration File:" -background $cb
	entry $w.file -textvariable gd16(configFileName) -background $cb -width 40
	button $w.read -text Read -command "$this ReadFileGD16" -background $cb
	button $w.write -text Write -command "$this WriteFileGD16" -background $cb
	label $w.dtlabel -text "dt (ns):" -background $cb
	entry $w.dt -textvariable gd16(dt) -width 3 -background $cb
	grid $w.filelabel $w.dtlabel $w.read -sticky news
	grid $w.file $w.dt $w.write -sticky news
#	grid $w.read $w.write
#	grid $w.dtlabel $w.dt
  grid columnconfigure $w all -weight 1
	pack $w -side right -expand 1 -fill x
	pack $gd16(w).side -side top -expand 1 -fill both

	set w $gd16(w).control
	set cc pink
	frame $w -borderwidth 2 -relief sunken
	label $w.channel -text Channel -background $cc
	label $w.delay -text Delay -background $cc
	label $w.width -text Width -background $cc
	for {set i 1} {$i <= 4} {incr i} {
		label $w.inspect$i -text Inspect$i -background $cc
	}
	label $w.bypass -text Bypass -background $cc
	grid $w.channel $w.delay - $w.width - \
	$w.inspect1 $w.inspect2 $w.inspect3 $w.inspect4 $w.bypass \
	-sticky news
	for {set i 1} {$i <= 16} {incr i} {
		if {[expr ($i/2)*2] == $i} {set cc lightgreen} else {set cc lightyellow}
		entry $w.channel$i -textvariable gd16(channel$i) -width 10 -background $cc
		canvas $w.delay$i -height 22 -width 50 -background $cc
		DrawEntryGD16 $w.delay$i 0 11 delay$i delay$i "" $cc
		label $w.delaylabel$i -textvariable gd16(delaylabel$i) -background $cc
		canvas $w.width$i -height 22 -width 50 -background $cc
		DrawEntryGD16 $w.width$i 0 11 width$i width$i "" $cc
		label $w.widthlabel$i -textvariable gd16(widthlabel$i) -background $cc
		radiobutton $w.insp1ch$i -command "$this SetInspect" -variable gd16(inspect1) -value [expr $i-1] -background $cc
		radiobutton $w.insp2ch$i -command "$this SetInspect"  -variable gd16(inspect2) -value [expr $i-1] -background $cc
		radiobutton $w.insp3ch$i -command "$this SetInspect"  -variable gd16(inspect3) -value [expr $i-1] -background $cc
		radiobutton $w.insp4ch$i -command "$this SetInspect"  -variable gd16(inspect4) -value [expr $i-1] -background $cc
		checkbutton $w.bypass$i -command "$this SetBypass" -variable gd16(bypass$i) -background $cc
		grid $w.channel$i $w.delay$i $w.delaylabel$i $w.width$i $w.widthlabel$i \
		     $w.insp1ch$i $w.insp2ch$i $w.insp3ch$i $w.insp4ch$i $w.bypass$i \
		     -sticky news
	}
	grid columnconfigure $w all -weight 1
	pack $w -side left -expand 1 -fill both	
}

itcl::body AGD16XLM72Panel::UpdateGUIGD16 {} {
	global gd16
	set w $gd16(w).control
  UpdateCalibratedValues
	for {set i 1} {$i <= 16} {incr i} {
		set gd16(bypass$i) [expr ($gd16(bypass)&(1<<($i-1)))>>($i-1)]	
	}
	set gd16(inspect1) [expr $gd16(inspect)&0xf]
	set gd16(inspect2) [expr ($gd16(inspect)&0xf0)>>4]
	set gd16(inspect3) [expr ($gd16(inspect)&0xf00)>>8]
	set gd16(inspect4) [expr ($gd16(inspect)&0xf000)>>12]
}

itcl::body AGD16XLM72Panel::UpdateCalibratedValues {} {
  global gd16
	for {set i 1} {$i <= 16} {incr i} {
		if {[llength $gd16(dt)] > 0} {
			set gd16(delaylabel$i) [format "= %8.0f ns" [expr $gd16(delay$i)*$gd16(dt)]]
			set gd16(widthlabel$i) [format "= %8.0f ns" [expr $gd16(width$i)*$gd16(dt)]]
		} else {
			set gd16(delaylabel$i) "= [format %8s ????] ns"
			set gd16(widthlabel$i) "= [format %8s ????] ns"
		}
	}
}


itcl::body AGD16XLM72Panel::CheckModuleGD16 {} {
  global gd16
  
  set signature [$proxy GetFWSignature]
  puts [format %x $signature]
  set validConfiguration [expr {$signature == $gd16(configuration)}]


  if {! $validConfiguration} {
    set errmsg "XLM72 has wrong configuration! Found [format 0x%x $signature], "
    append errmsg "expected [format 0x%x $gd16(configuration)]"
		tk_messageBox -icon error -message $errmsg
  }

  return $validConfiguration
}

# Update the state of the GUI
itcl::body AGD16XLM72Panel::GetModuleGD16 {} {
	global gd16
	if {![CheckModuleGD16]} {return}
	for {set i 1} {$i <= 16} {incr i} {
		set delayWidth [$proxy GetDelayWidth $i]
    puts $delayWidth
		set gd16(delay$i) [lindex $delayWidth 0]
		set gd16(width$i) [lindex $delayWidth 1]
	}
	set gd16(bypass) [$proxy GetBypass]
	set gd16(inspect) [$proxy GetInspect]
	UpdateGUIGD16
}

# write the state of the gui to the device
itcl::body AGD16XLM72Panel::PutModuleGD16 {} {
	global gd16
	if {![CheckModuleGD16]} {return}
	for {set i 1} {$i <= 16} {incr i} {
    $proxy SetDelayWidth $i $gd16(delay$i) $gd16(width$i)
	}
	$this SetInspect 
	$this SetBypass
}

itcl::body AGD16XLM72Panel::ReadFileGD16 {} {
	global gd16
	if {![file exist $gd16(configFileName)]} {
		tk_messageBox -icon error -message "File $gd16(configFileName) doesn't exist!"
		return
	}
	source $gd16(configFileName)
	foreach n [array names gd16] {set gd16($n) [lindex [array get gd16 $n] 1]}
	UpdateGUIGD16
}

itcl::body AGD16XLM72Panel::WriteFileGD16 {} {
	global gd16
	if {[catch {set file [open $gd16(configFileName) w]}]} {
		tk_messageBox -icon error -message "Unable to open file $gd16(configFileName) for writing!"
		return
	}
	puts $file "# gd16 configuration file written on [clock format [clock second]]"
	puts $file [format "set gd16(configFileName) %s" $gd16(configFileName)]
	puts $file [format "set gd16(configuration) %s" $gd16(configuration)]
	puts $file [format "set gd16(dt) %g" $gd16(dt)]
	for {set i 1} {$i <= 16} {incr i} {
		puts $file [format "set gd16(channel$i) %s" $gd16(channel$i)]
		puts $file [format "set gd16(delay$i) %d" $gd16(delay$i)]
		puts $file [format "set gd16(width$i) %d" $gd16(width$i)]
	}
	puts $file [format "set gd16(bypass) 0x%x" $gd16(bypass)]
	puts $file [format "set gd16(inspect) 0x%x" $gd16(inspect)]
	close $file
}

if {0} {
itcl::body AGD16XLM72Panel::SetDelayWidth {i} {
	global gd16
	if {![CheckModuleGD16]} {return}
	GrabBusGD16
	if {[llength $gd16(dt)] > 0} {
		set gd16(delaylabel$i) [format "= %.0f ns" [expr $gd16(delay$i)*$gd16(dt)]]
		set gd16(widthlabel$i) [format "= %.0f ns" [expr $gd16(width$i)*$gd16(dt)]]
	}
	set gd16(dw$i) [expr $gd16(delay$i) + ($gd16(width$i)<<8)]
	XsetGD16 [expr $i*4] $gd16(dw$i)
	ReleaseBusGD16
}
}
itcl::body AGD16XLM72Panel::SetInspect {} {
	global gd16
	if {![CheckModuleGD16]} {return}
	set gd16(inspect) [expr $gd16(inspect1) + ($gd16(inspect2)<<4) + ($gd16(inspect3)<<8) + ($gd16(inspect4)<<12)]
	return [$proxy SetInspect $gd16(inspect)]
}

itcl::body AGD16XLM72Panel::SetBypass {} {
	global gd16
	if {![CheckModuleGD16]} {return}
	set gd16(bypass) 0
	for {set i 1} {$i <= 16} {incr i} {
		set gd16(bypass) [expr $gd16(bypass) + ($gd16(bypass$i)<<($i-1))]
	}
	return [$proxy SetBypass $gd16(bypass)]
}

itcl::body AGD16XLM72Panel::DrawEntryGD16 {c x y id var label color} {
	global gd16
	$c create text $x $y -text $label -anchor e
	label $c.$id -textvariable gd16($var) -width 4 -background $color
	$c create window $x $y -window $c.$id -anchor w
	set up [$c create polygon [expr $x+35] [expr $y-2] \
	[expr $x+45] [expr $y-2] [expr $x+40] [expr $y-10] -fill white -outline black]
	set down [$c create polygon [expr $x+35] [expr $y+2] \
	[expr $x+45] [expr $y+2] [expr $x+40] [expr $y+10] -fill white -outline black]
	$c bind $up <Enter> "$c itemconfigure $up -fill black"
	$c bind $up <Leave> "$c itemconfigure $up -fill white"
	$c bind $up <Button-1> "$this IncrementGD16 $var"
	$c bind $up <ButtonRelease> "$this StopRepeatGD16"
	$c bind $down <Enter> "$c itemconfigure $down -fill black"
	$c bind $down <Leave> "$c itemconfigure $down -fill white"
	$c bind $down <Button-1> "$this DecrementGD16 $var"
	$c bind $down <ButtonRelease> "$this StopRepeatGD16"
	return $c.$id
}

itcl::body AGD16XLM72Panel::IncrementGD16 {var} {
	global gd16
	if {[info exist gd16(firstClick)] == 0} {
		set gd16(firstClick) 1
	}
	if {$gd16(firstClick) == 1} {
		set gd16(repeatID) [after 500 $this IncrementGD16 $var]
		set gd16(firstClick) 0
	} else {
		set gd16(repeatID) [after 50 $this IncrementGD16 $var]
	}
	if {$gd16(locked) == 1} {
		return
	}
	set gd16($var) [expr $gd16($var) + 1]
	if {$gd16($var) > 255} {
		set gd16($var) 255
	}
	if {[string match delay* $var]} {set i [string trimleft $var delay]}
	if {[string match width* $var]} {set i [string trimleft $var width]}
	$proxy SetDelayWidth $i $gd16(delay$i) $gd16(width$i)
}

itcl::body AGD16XLM72Panel::DecrementGD16 {var} {
	global gd16
	if {[info exist gd16(firstClick)] == 0} {
		set gd16(firstClick) 1
	}
	if {$gd16(firstClick) == 1} {
		set gd16(repeatID) [after 500 $this DecrementGD16 $var]
		set gd16(firstClick) 0
	} else {
		set gd16(repeatID) [after 50 $this DecrementGD16 $var]
	}
	if {$gd16(locked) == 1} {
		return
	}
	set gd16($var) [expr $gd16($var) - 1]
	if {$gd16($var) < 0} {
		set gd16($var) 0
	}
	if {[string match delay* $var]} {set i [string trimleft $var delay]}
	if {[string match width* $var]} {set i [string trimleft $var width]}
	$proxy SetDelayWidth $i $gd16(delay$i) $gd16(width$i)
}

itcl::body AGD16XLM72Panel::StopRepeatGD16 {} {
	global gd16
	after cancel $gd16(repeatID)
	set gd16(firstClick) 1
}

itcl::body AGD16XLM72Panel::LockGD16 {} {
	global gd16
	set gd16(locked) 1
#	set keep "$gd16(w).side.command.get"
	set keep ""
	foreach c [winfo children $gd16(w).side.command] {
		if {[lsearch $keep $c] == -1} {
			$c configure -state disabled
		}
	}
	set keep "$gd16(w).side.file.read"
	foreach c [winfo children $gd16(w).side.file] {
		if {[lsearch $keep $c] == -1} {
			$c configure -state disabled
		}
	}
	foreach c [winfo children $gd16(w).control] {
		$c configure -state disabled
	}
}

itcl::body AGD16XLM72Panel::UnlockGD16 {} {
	global gd16
	set gd16(locked) 0
	foreach c [winfo children $gd16(w).side.command] {
		$c configure -state normal
	}
	foreach c [winfo children $gd16(w).side.file] {
		$c configure -state normal
	}
	foreach c [winfo children $gd16(w).control] {
		$c configure -state normal
	}
}

itcl::body AGD16XLM72Panel::SetConfigFileName {path} {
  global gd16
  set gd16(configFileName) $path
}


itcl::body AGD16XLM72Panel::NeedsUpdate {name1 name2 op} {
    UpdateCalibratedValues
}

snit::type AGD16XLM72PanelProxy {

  option -host -default localhost
  option -port -default 27000
  option -connection -default {}
  option -onlost -default {}
  
  option -name  -default {}
  option -widget  -default {}

  option -ring  -default "tcp://localhost/$::tcl_platform(user)"
  constructor args {

    $self configurelist $args
    $self Reconnect $options(-host) $options(-port)

  }

  method GetFWSignature {} {
    return [$self Get [list fwsignature]]
  }

  method GetDelayWidth {channel} {
    return [$self Get [list [format "delaywidth%02d" $channel]] ]
  }

  method SetDelayWidth {channel delay width} {
    return [$self Set [list [format "delaywidth%02d" $channel] "delay${delay}width${width}"]]
  }

  method GetBypass {} {
    return [$self Get [list bypass]]
  }

  method SetBypass {value} {
    return [$self Set [list bypass $value]]
  }

  method GetInspect {} {
    return [$self Get [list inspect]]
  }

  method SetInspect {value} {
    return [$self Set [list inspect $value]]
  }

  method Set {args} {
    set name $options(-name)
    set conn $options(-connection)
    set args [lindex $args 0] 
    set command [concat [list $conn Set $name] $args]
    return [$self transaction $command]
  }

  method Get {args} {
    set name $options(-name)
    set conn $options(-connection)
    set value [$self transaction [concat [list $conn Get $name] [lindex $args 0]]]
    return $value
  }

  method Update {} {
    set name $options(-name)
    set conn $options(-connection)

    return [$self transaction [list $conn Update $name]]
  }

  method Reconnect {host port} {
    while {[catch {controlClient %AUTO% -server $host -port $port} connection]} {
      tk_messageBox -icon info \
        -message "The control panel requires readout be running, please start it: $connection"
    }
    set options(-connection) $connection
  }

  ## 
  # @brief Manage the transactions
  # 
  # Executes the request and then deals with the response.
  # If an exceptional return occurs with no result, then the
  # -onlost connection script is called. If that is not defined then
  # it just spits out a standard error. If the slow controls server
  # returns back an actual response beginning with "ERROR" then 
  # the method transforms the response into a TCL error. Otherwise,
  # a successful transaction just returns a value.
  #
  # @param script a properly formatted Set, Get, or Update command
  # 
  # @return depends on the request 
  # 
  method transaction {script} {
# loop until transaction works or error signalled.

    while {[catch {eval $script} msg] && ($msg ne "")} {
      if {$options(-onlost) ne ""} {# comm fail handler..
        puts $msg
        eval $options(-onlost) $options(-connection)
        set script [lreplace $script 0 0 $options(-connection)]
      } else {			# no handler.
        error "Communication failure $msg"
      }
    }

    puts $msg
    if {[string is list $msg]} {
      if {[lindex $msg 0] eq "ERROR"} {
        puts "THE CLIENT RECEIVED ERROR"
          error $msg;			# Transaction error.
      }
    }

  	return $msg

  }

  
}
