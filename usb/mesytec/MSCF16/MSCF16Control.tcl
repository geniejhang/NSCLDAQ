#!/usr/bin/env tclsh

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2015.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#    Jeromy Tompkins
#	   NSCL
#	   Michigan State University
#	   East Lansing, MI 48824-1321

set here [file dirname [file normalize [info script]]]
lappend auto_path $here

# Tk package parses argv when it is required... It will cause a crash 
# when it encounters unknown commands, so we have to hide the real arguments
# from it.
set argv_tmp $argv
set argv [list]
package require Tk

package require mscf16guiapp
package require cmdline ;# for the command line option parsing

# Handle the options
set options {
  {-serialfile.arg ""          "name of serial file (e.g. /dev/ttyUSB0) \[MANDATORY for USB\]"}
  {-channelconfig.arg "MSCF16channels.txt"       "file name to save channel names to [default MSCF16channels.txt]"}
}
set usage " --serialfile value ?option value? :"

# if the user just wants help, do nothing more than provide them what they want
if {("-help" in $argv_tmp) || ("-?" in $argv_tmp)} {
  puts [cmdline::usage $options $usage]
  exit
}

## some helper procs for loading and saving channel names
proc ReadInChannelNames {path} {
  set f [open $path r]
  for {set i 0} {$i < 16} {incr i} {
    chan gets $f ::MSCF16ChannelNames::chan$i 
  }
  close $f
}

proc Exit {path} {
  set f [open $path w]
  for {set i 0} {$i < 16} {incr i} {
    chan puts $f [set MSCF16ChannelNames::chan$i] 
  }
  chan puts $f "Generated on [clock format [clock seconds]]"
  close $f
  exit
}

#----------------------------------------------------------------------
# Here begins the code to process arguments and launch the program.
#
set res [catch {
  array set ::params [::cmdline::getoptions argv_tmp $::options]
} msg]
if {$res == 1} {
  puts $msg
  exit
}

# I really prefer to deal with dicts rathers than arrays...
set optionsDict [array get ::params]
set ChannelNamePath [dict get $optionsDict -channelconfig]

# create the app (constructs the driver and gui)
MSCF16GuiApp app -widgetname .form {*}$optionsDict

# grid it.
grid .form -sticky nsew
grid rowconfigure . 0 -weight 1
grid columnconfigure . 0 -weight 1
wm title . "MSCF-16 Controls"

# some stuff for loading and saving the channel names
ReadInChannelNames $ChannelNamePath
wm protocol . WM_DELETE_WINDOW [list Exit $ChannelNamePath]

ttk::style configure Header.TFrame -background goldenrod3
ttk::style configure Header.TLabel -background goldenrod3 \
                                   -font {helvetica 14 bold}
ttk::style configure Group.TFrame -background snow3
ttk::style configure Group.TLabel -background snow3
ttk::style configure Group.TSpinbox -background snow3
ttk::style configure Group.TCheckbutton -background snow3
ttk::style configure Group.TRadiobutton -background snow3
ttk::style configure Group.TEntry -background snow3


