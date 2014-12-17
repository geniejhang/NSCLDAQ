#!/usr/bin/env tclsh

package require cmdline

# the next two lines are a workaround to prevent the Tk argument parsing from seeing
# switches like -help.
set argv2 $argv
set argv {}

package require mdgg16gui
package require mdgg16proxy


# Handle the options
set options {
  {-module.arg     ""          "name of module registered to slow-controls server"}
  {-host.arg       "localhost" "host running VMUSBReadout slow-controls server" }
  {-port.arg       27000       "port the slow-controls server is listening on" }
}
set usage " -module value ?option value? :"

if {("-help" in $argv2) || ("-?" in $argv2)} {
  puts [cmdline::usage $options $usage]
  exit
}

if {"--module" ni $argv2} {
  puts "The -module option is MANDATORY and was not provided."
  puts [cmdline::usage $options $usage]
  exit
}

set res [catch {
  array set ::params [::cmdline::getoptions argv2 $::options]
} msg]
if {$res == 1} {
  puts $msg
  exit
}

# Set up the style
ttk::style configure "Title.TLabel" -foreground "midnight blue" \
                                    -font "helvetica 28 bold"
ttk::style configure "Header.TLabel" -background "cornflower blue"
ttk::style configure "Header.TFrame" -background "cornflower blue"
#ttk::style configure "Even.TCheckbutton" -background ""
ttk::style configure "Odd.TCheckbutton" -background "snow3"
ttk::style configure "Odd.TFrame" -background "snow3"



set paramDict [array get ::params]
MDGG16Proxy ::proxy -server [dict get $paramDict -host] \
                    -port [dict get $paramDict -port] \
                    -module [dict get $paramDict -module]

MDGG16View .view 
MDGG16Presenter ::pres -view .view -handle ::proxy

grid .view -sticky nsew -padx 8 -pady 8
grid rowconfigure . 0 -weight 1
grid columnconfigure . 0 -weight 1
wm resizable . false false
