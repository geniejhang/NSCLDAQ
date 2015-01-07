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



package provide mscf16usb 1.0

package require snit
package require Utils

snit::type MSCF16USB {

  variable m_serialFile ;# the communication channel to the device 
  variable m_needsUpdate ;# has device state changed since last update?
  variable m_moduleState ;# the dict storing the state

  component m_parser

  ## @brief Construct the driver 
  #
  # Opens the file
  constructor {serialFile} {
    install m_parser using MSCF16DSParser %AUTO%

    set m_serialFile [open $serialFile "r+"]

    chan configure $m_serialFile -blocking 0

    # we don't know anything about the state of the module
    # so we are certainly in need of an update
    set m_needsUpdate 1
    set m_moduleState [dict create]
  }

  destructor {
    catch {$m_parser destroy}
    catch {close $m_serialFile}
  }

  ##
  #
  method SetGain {ch val} {
    if {![Utils::isInRange 0 4 $ch]} {
      set msg "Invalid channel provided. Must be in range \[0,4\]."
      return -code error -errorinfo MSCF16USB::SetGain $msg
    }
    if {![Utils::isInRange 0 15 $val]} {
      set msg "Invalid value provided. Must be in range \[0,15\]."
      return -code error -errorinfo MSCF16USB::SetGain $msg
    }
    return [$self _Transaction [list SG [expr $ch+1] $val]]
  }

  method GetGain {ch} {
    if {$m_needsUpdate} {
      $self Update
    }
    if {![Utils::isInRange 0 4 $ch]} {
      set msg "Invalid channel provided. Must be in range \[0,4\]."
      return -code error -errorinfo MSCF16USB::GetGain $msg
    }

    return [lindex [dict get $m_moduleState Gains] $ch]
  }

  method SetShapingTime {ch val} {
    if {![Utils::isInRange 0 4 $ch]} {
      set msg "Invalid channel provided. Must be in range \[0,4\]."
      return -code error -errorinfo MSCF16USB::SetShapingTime $msg
    }
    if {![Utils::isInRange 0 3 $val]} {
      set msg "Invalid value provided. Must be in range \[0,3\]."
      return -code error -errorinfo MSCF16USB::SetShapingTime $msg
    }
    return [$self _Transaction [list SS [expr $ch+1] $val]]
  }

  method GetShapingTime {ch} {
    if {$m_needsUpdate} {
      $self Update
    }
    if {![Utils::isInRange 0 4 $ch]} {
      set msg "Invalid channel provided. Must be in range \[0,4\]."
      return -code error -errorinfo MSCF16USB::GetShapingTime $msg
    }
    return [lindex [dict get $m_moduleState ShapingTime] $ch]
  }

  method SetPoleZero {ch val} {
    if {![Utils::isInRange 0 16 $ch]} {
      set msg "Invalid channel provided. Must be in range \[0,16\]."
      return -code error -errorinfo MSCF16USB::SetPoleZero $msg
    }
    if {![Utils::isInRange 0 255 $val]} {
      set msg "Invalid value provided. Must be in range \[0,255\]."
      return -code error -errorinfo MSCF16USB::SetPoleZero $msg
    }

    return [$self _Transaction [list SP [expr $ch+1] $val]]
  }

  method GetPoleZero {ch} {
    if {$m_needsUpdate} {
      $self Update
    }
    if {![Utils::isInRange 0 16 $ch]} {
      set msg "Invalid channel provided. Must be in range \[0,16\]."
      return -code error -errorinfo MSCF16USB::GetPoleZero $msg
    }

    return [lindex [dict get $m_moduleState PoleZero] $ch]
  }

  method SetThreshold {ch val} {
    if {![Utils::isInRange 0 16 $ch]} {
      set msg "Invalid channel provided. Must be in range \[0,16\]."
      return -code error -errorinfo MSCF16USB::SetPoleZero $msg
    }
    if {![Utils::isInRange 0 255 $val]} {
      set msg "Invalid value provided. Must be in range \[0,255\]."
      return -code error -errorinfo MSCF16USB::SetPoleZero $msg
    }

    return [$self _Transaction [list ST [expr $ch+1] $val]]
  }

  method GetThreshold {ch} {
    if {$m_needsUpdate} {
      $self Update
    }
    if {![Utils::isInRange 0 16 $ch]} {
      set msg "Invalid channel provided. Must be in range \[0,16\]."
      return -code error -errorinfo MSCF16USB::GetThreshold $msg
    }

    return [lindex [dict get $m_moduleState Thresholds] $ch]
  }

  method SetMonitor {channel} {
    if {![Utils::isInRange 0 15 $channel]} {
      set msg "Invalid channel provided. Must be in range \[0,15\]."
      return -code error -errorinfo MSCF16USB::SetMonitor $msg
    }
    return [$self _Transaction [list MC [expr $channel+1]]]
  }

  method GetMonitor {} {
    if {$m_needsUpdate} {
      $self Update
    }
    set val [dict get $m_moduleState Monitor]
    return [expr $val-1]
  }

  method SetMode {mode} {
    set code 0
    switch $mode {
      common     {set code 0} 
      individual {set code 1} 
      default {
        set msg "Invalid mode provided. Must be either common or individual."
        return -code error $msg
      }
    }
    return [$self _Transaction [list SI $code]]
  }

  method GetMode {} {
    if {$m_needsUpdate} {
      $self Update
    }

    set rawMode [dict get $m_moduleState Mode]
    
    set mode unknown
    switch $rawMode {
      single {set mode individual}
      common {set mode common} 
      default {
        set msg "Configuration mode of device ($rawMode) is not supported by "
        append msg "driver."
        return -code error MSCF16USB::GetMode $msg
      }
    } 
    return $mode
  }

  method EnableRC {on} {
    if {![string is boolean $on]} {
      set msg "Invalid argument provided. Must be a boolean value."
      return -code error -errorinfo MSCF16USB::EnableRC $msg
    }

    if {[string is true $on]} {
      return [$self _Transaction "ON"]
    } else {
      return [$self _Transaction "OFF"]
    }
  }

  method RCEnabled {} {
    if {$m_needsUpdate} {
      $self Update
    }

    return [string is true [dict get $m_moduleState Remote]]
  }

  ## @brief Send the DS command and parse response
  #
  method Update {} {
    set response [$self _Transaction "DS"]

    set m_moduleState [$m_parser parseDSResponse $response]
    set m_needsUpdate 0
  }

  #---------------------------------------------------------------------------#
  # Utility methods
  #

  ## @brief _Write to the device a command
  #
  # This is the low-level method for sending a command to the device without
  # reading back. It keeps track of whether the command string will end up
  # changing the device state or not. 
  #
  # @important It is the caller's responsibility to ensure that the command is
  #            valid.
  #
  # @param script   the command string
  method _Write {script} {
  # check that the command name (the first element of the script list) is
  # going to modify the state of the module
  
    if {[lindex $script 0] ni [list DS]} { 
    # we will need to update our state the next time the user queries a value
      set m_needsUpdate 1
    }

    puts $m_serialFile $script
    chan flush $m_serialFile
  }

  ## @brief Read response from the device
  #
  # Low-level command to read data from the device. The command will poll the
  # device until it receives the entire response. It consider a response
  # complete once the "mcfd-16>" string is found. Each successive attempt to
  # read from the device occurs after a 25 ms pause.
  #
  # @return response from device
  method _Read {} {
    set totalResponse "" ;# string we will keep building and ultimately return

    # read and append result to totalResponse
    set response [chan read $m_serialFile]
    append totalResponse $response 

    # keep trying to read until mscf> or mscf-RC> is found in the response
    # Depending on the state of remote control, the prompt returns a different
    # string.
    set patt {mscf(-RC){0,1}>$}
    set iter 0
    while {![regexp $patt $totalResponse]} {
      after 25 
      set response [chan read $m_serialFile]
      append totalResponse $response
      incr iter
    }

    # done
    return $totalResponse
  }

  ## @brief Complete a symmetric transaction (_Write then _Read)
  #
  # @param script basic command to execute
  # 
  # @returns response from device
  method _Transaction {script} {
    $self _Write $script

    return [$self _Read]
  }

  method _setModuleState {state} {
    set m_moduleState $state
  }

}

snit::type MSCF16DSParser {

  variable mscf
  constructor {args} {
  }

  method parseDSResponse {response} {
    set response [split $response "\n"]

    # this is just assumed to be in the right order b/c I don't have an
    # actual module to tell me what it should be.
    set tmp [$self parseGains [lindex $response 15]]
    dict set mscf Gains $tmp

    set tmp [$self parseThresholds [lindex $response 16]]
    dict set mscf Thresholds $tmp

    set tmp [$self parsePoleZero [lindex $response 17]]
    dict set mscf PoleZero $tmp

    set tmp [$self parseShapingTime [lindex $response 18]]
    dict set mscf ShapingTime $tmp

    set tmp [$self parseMonitor [lindex $response 20]]
    dict set mscf Monitor $tmp
    
    set tmp [$self parseConfigMode [lindex $response 24]]
    dict set mscf Mode $tmp

    set tmp [$self parseRemote [lindex $response 26]]
    dict set mscf Remote $tmp

    return $mscf
  }

  method parseCode {line} {
    set code [string range $line 0 [string first : $line]]
  }

  method parseGains {line} {

    set lineVals [list]
    set patt {^gains: (\d+ ){4}c:(\d+)$}
    if {[regexp $patt $line]} {
      set line [string range $line 7 end]
      set lineVals [split $line " "]
      set comVal [string range [lindex $lineVals 4] 2 end]
      lset lineVals 4 $comVal
    }
    return $lineVals
  }

  method parseThresholds {line} {
    set lineVals [list]

    set patt {^threshs: (\d+ ){16}c:(\d+)$}
    if {[regexp $patt $line]} {
      set line [string range $line 9 end]
      set lineVals [split $line " "]
      set comVal [string range [lindex $lineVals 16] 2 end]
      lset lineVals 16 $comVal

    }
    return $lineVals
  }

  method parsePoleZero {line} {

    set lineVals [list]

    set patt {^pz: (\d+ ){16}c:(\d+)$}
    if {[regexp $patt $line]} {
      set line [string range $line 4 end]
      set lineVals [split $line " "]
      set comVal [string range [lindex $lineVals 16] 2 end]
      lset lineVals 16 $comVal

    }
    return $lineVals
  }

  method parseShapingTime {line} {
    set lineVals [list]
    set patt {^shts: (\d+ ){4}c:(\d+)$}
    if {[regexp $patt $line]} {
      set line [string range $line 6 end]
      set lineVals [split $line " "]
      set comVal [string range [lindex $lineVals 4] 2 end]
      lset lineVals 4 $comVal
    }
    return $lineVals

  }

  method parseMultiplicity {line} {
    set lineVals [list]
    set patt {^mult: (\d+) (\d+)$}
    if {[regexp $patt $line v0 v1]} {
      set lineVals [list $v0 $v1]
    }
    return $lineVals
  }

  method parseMonitor {line} {
    set nSet [scan $line "monitor: %d" monitor]

    if {$nSet != 1} {
      return -code error "Failed to parse monitor"
    }
    return $monitor
  }

  method parseConfigMode {line} {
     set nSet [scan $line "%s mode" mode]

     if {$nSet != 1} {
       return -code error "Failed to parse config mode"
     }

     return $mode
  }

  method parseRemote {line} {
     set nSet [scan $line "rc %s" remote ]
     if {$nSet != 1} {
       return -code error "Failed to parse rc mode"
     }
     return $remote
  }
    
}

