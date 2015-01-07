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



package provide mscf16guiapp 1.0

package require snit
package require mscf16usb
package require mscf16gui


##
#
#
snit::type MSCF16AppOptions {

  option -serialfile -default {}
  option -channelconfig -default {}
  option -widgetname -default {.app}

  constructor {args} {
    $self configurelist $args
  }
}

##
#
#
snit::type MSCF16GuiApp {

  component _options
  component _view
  component _presenter
  component _handle

  delegate option * to _options

  constructor {args} {
    install _options using MSCF16AppOptions %AUTO%
    $self configurelist $args
    set res [catch {
      install _handle using MSCF16USB %AUTO% [$self cget -serialfile]
      install _view using MSCF16Form [$self cget -widgetname]

      install _presenter using MSCF16Presenter %AUTO% -view $_view \
                                     -handle $_handle
      # read in the names of the channels.
      $self ReadInChannelNames [$self cget -channelconfig]
    } msg]
    
    if {$res} {
      puts "Error during application startup. Reason given : \"$msg\""
      exit
    }

  }

  destructor {
    # save state to file
    $self SaveChannelNames [$self cget -channelconfig]

    $_options destroy
    $_handle destroy
    catch {destroy $_view}

  }

  ## some helper procs for loading and saving channel names
  method ReadInChannelNames {path} {
    if {[catch {open $path r} f]} {
      set msg  "User provided nonexistent file for channel names. For now, defaults "
      append msg "will be used instead. On exit, the specified file will be "
      append msg "generated while saving the then current channel names."
      tk_messageBox -icon error -message $msg
      return
    }

    # otherwise we succeeded to open the file
    set content [chan read $f]
    close $f

    set names [split $content "\n"]

    # if we have all of the channels present, then use them.
    # there is at least 16 lines... the 17th line is the timestamp
    if {[llength $names] >= 16} {
      for {set ch 0} {$ch<16} {incr ch} {
        set ::MSCF16ChannelNames::chan$ch [lindex $names $ch]
      }
    }
  }

  method SaveChannelNames {path} {
    set f [open $path w]
    for {set i 0} {$i < 16} {incr i} {
      chan puts $f [set ::MSCF16ChannelNames::chan$i] 
    }
    chan puts $f "Generated on [clock format [clock seconds]]"
    close $f
    exit
  }


}
