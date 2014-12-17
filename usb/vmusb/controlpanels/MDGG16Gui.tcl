

package provide mdgg16gui 1.0

package require snit
package require Tk
package require Utils
package require mdgg16proxy

namespace eval MDGG16ChannelNames {
  variable chan0 Ch0
  variable chan1 Ch1
  variable chan2 Ch2
  variable chan3 Ch3
  variable chan4 Ch4
  variable chan5 Ch5
  variable chan6 Ch6
  variable chan7 Ch7
  variable chan8 Ch8
  variable chan9 Ch9
  variable chan10 Ch10
  variable chan11 Ch11
  variable chan12 Ch12
  variable chan13 Ch13
  variable chan14 Ch14
  variable chan15 Ch15
}


snit::widget CheckbuttonColumn {

  hulltype ttk::frame 

  option -presenter -default {} -configuremethod SetPresenter
  option -stylename -default {} -configuremethod SetStyle
  delegate option * to hull

  variable _bit0
  variable _bit1
  variable _bit2
  variable _bit3
  variable _bit4
  variable _bit5
  variable _bit6
  variable _bit7
  variable _bit8
  variable _bit9
  variable _bit10
  variable _bit11
  variable _bit12
  variable _bit13
  variable _bit14
  variable _bit15

  variable _outOfSync

  variable _widgets

  constructor {args} {
    set _widgets [list]

    $self configurelist $args

    $self SetOutOfSync 1

    for {set ch 0} {$ch < 17} {incr ch} {
      set _bit$ch 1
      trace add variable [myvar _bit$ch] write [mymethod BitChanged]
    }


    $self BuildGUI
    $self UpdateStyle [$self cget -style]
  }

  ## @brief Buil
  #
  method BuildGUI {} {
    for {set ch 0} {$ch < 16} {incr ch} {
      lappend _widgets [ttk::checkbutton $win.bit$ch -variable [myvar _bit$ch]] ;#-text "Enable"]
    }
    set sep [ttk::separator $win.separator -orient horizontal]
    ttk::checkbutton $win.bit16 -variable [myvar _bit16]
    trace add variable [myvar _bit16] write [mymethod Synchronize]

    for {set row 0} {$row<16} {incr row} {
      grid [lindex $_widgets $row] -sticky ew -padx 4 -pady 4
    }
    grid $sep -sticky ew
    grid $win.bit16 -sticky ew -padx 4 -pady 4

    grid rowconfigure $win [Utils::sequence 0 [llength $_widgets] 1] -weight 1 
    grid columnconfigure $win 0 -weight 1
  }

  ## @brief Set a specific bit to a value
  #
  # This does not check for valid parameter values and leaves it to the caller
  # to make sure that they are reasonable.
  #
  # @param index  bit index (must be in range [0,7])
  # @param val    value  (must be either 0 or 1)
  method SetBit {index val} {
    set _bit$index $val
  }

  ## @brief Retrieve the index-th bit value
  # 
  # There is no parameter checking in this so it is up to the caller to pass in
  # a good bit index.
  #
  # @param index  index of bit (must be in range [0,7])
  #
  # @returns value of bit
  method GetBit {index} {
    return [set _bit$index]
  }

  method SetPresenter {opt val} {
    set options($opt) $val
  }

  method UpdateStyle {stylename} {
    if {[llength $_widgets]>0} {
      foreach w $_widgets {
        $w configure -style "$stylename.TCheckbutton"
      }
      $win.bit16 configure -style "$stylename.TCheckbutton"
      $self configure -style "$stylename.TFrame"
    }
  }

  method Synchronize {name1 name2 op} {
    # get the state of the variable
    set state [set [$win.bit16 cget -variable]]
    foreach w $_widgets {
      set [$w cget -variable] $state
    }
  }

  method SetStyle {opt val} {
    $self UpdateStyle $val
    set options($opt) $val
  }

  method BitChanged {name1 name2 op} {
    if {!$_outOfSync} {
      $self SetOutOfSync 1
    }
  }

  method SetOutOfSync {state} {
    set _outOfSync $state
  }
  method GetOutOfSyncVar {} { return [myvar _outOfSync]}
}


##########################

snit::widget NameColumn {

  option -presenter -default {} -configuremethod SetPresenter

  constructor {args} {
    $self configurelist $args

    set _rows [list]

    $self BuildGUI
  }

  ## @brief Buil
  #
  method BuildGUI {} {
    for {set ch 0} {$ch < 16} {incr ch} {
      lappend _rows [ttk::entry $win.name$ch -textvariable MDGG16ChannelNames::chan$ch \
        -width 20\
        -validate focus -validatecommand [mymethod ValidateName %P] \
        -invalidcommand [mymethod ResetChannelName %W]]
    }
    lappend _rows [ttk::label $win.common -text "Common"]

    foreach row $_rows {
      grid $row -sticky ew -padx 4 -pady 4
    }

    grid rowconfigure $win [Utils::sequence 0 [llength $_rows] 1] -weight 1 
    grid columnconfigure $win 0 -weight 1
  }

  ## @brief check whether a channel name contains non-whitespace characters
  # 
  # this is called when a channel entry loses focus
  #
  # @param name   candidate string 
  #
  # @return boolean
  # @retval 0 - string was empty or all whitespace
  # @retval 1 - otherwise
  method ValidateName {name} {
    set name [string trim $name]
    set good [expr [string length $name]!=0]

    return $good
  }

  ## @brief reset channel to a simple string
  #
  # typically called with validatename returns false. it will set the string
  # to "ch#".
  #
  # @returns ""
  method ResetChannelName {widget} {
    set str [$widget cget -textvariable]
    regexp {^.*(\d+)$} $widget match ch
    set $str "Ch$ch"
  }

  method SetPresenter {opt val} {
    set options($opt) $val
  }
}

############################

snit::widget MDGG16View {

  option -presenter -default {}

  component _colNames
  component _colA
  component _colB
  component _colC
  component _colD
  component _updateButton

  variable _outOfSyncAVar
  variable _outOfSyncBVar
  variable _outOfSyncCVar
  variable _outOfSyncDVar

  constructor {args} {
    $self configurelist $args

    $self BuildGUI
    $self SetUpSync

  }

  destructor {
  }

  method BuildGUI {} {
    set title [ttk::label $win.title -text "MDGG-16 Controls" -style Title.TLabel]
    
    set header [ttk::frame $win.headers -style "Header.TFrame"]
    ttk::label $header.names -text "Names" -width 24 -style Header.TLabel
    ttk::label $header.orA -text "OR A" -style Header.TLabel
    ttk::label $header.orB -text "OR B" -style Header.TLabel
    ttk::label $header.orC -text "OR C" -style Header.TLabel
    ttk::label $header.orD -text "OR D" -style Header.TLabel
    grid $header.names $header.orA $header.orB $header.orC $header.orD -sticky nsew -padx 4 -pady 4
    grid columnconfigure $header 0 -weight 4 -uniform a
    grid columnconfigure $header {1 2 3 4} -weight 1 -uniform a

    set cols [ttk::frame $win.cols]
    install _colNames using NameColumn $cols.colNames
    install _colA using CheckbuttonColumn $cols.colA -stylename "Even"
    install _colB using CheckbuttonColumn $cols.colB -stylename "Odd"
    install _colC using CheckbuttonColumn $cols.colC -stylename "Even"
    install _colD using CheckbuttonColumn $cols.colD -stylename "Odd"
    grid $_colNames $_colA $_colB $_colC $_colD -sticky nsew
    grid rowconfigure $cols {0 1 2 3 4} -weight 0
    grid columnconfigure $cols 0 -weight 4 -uniform a
    grid columnconfigure $cols {1 2 3 4} -weight 1 -uniform a

    set buttons [ttk::frame $win.buttons]
    ttk::button $buttons.commit -text "Commit to Device" -command [mymethod Commit]
    set _updateButton [ttk::button $buttons.update -text "Update from Device" \
                                                   -command [mymethod Update]]
    grid $buttons.commit $buttons.update -sticky ew
    grid columnconfigure $buttons {0 1} -weight 1

    grid $title -sticky new
    grid $header -sticky ew
    grid $cols -sticky nsew
    grid $buttons -sticky new
    grid rowconfigure $win {0 1 2 3} -weight 1
    grid columnconfigure $win 0 -weight 1

  }
  
  method SetUpSync {} {
    set _outOfSyncAVar [$_colA GetOutOfSyncVar]
    trace add variable $_outOfSyncAVar write [mymethod SyncChanged]
    set _outOfSyncBVar [$_colB GetOutOfSyncVar]
    trace add variable $_outOfSyncBVar write [mymethod SyncChanged]
    set _outOfSyncCVar [$_colC GetOutOfSyncVar]
    trace add variable $_outOfSyncCVar write [mymethod SyncChanged]
    set _outOfSyncDVar [$_colD GetOutOfSyncVar]
    trace add variable $_outOfSyncDVar write [mymethod SyncChanged]

  }

  method Commit {} {
    set pr [$self cget -presenter]
    if {$pr ne {}} {
      $pr Commit
    }
  }

  method Update {} {
    set pr [$self cget -presenter]
    if {$pr ne {}} {
      $pr Update 
    }
  }

  method GetBit {col ch} {
    set widget [$self MapColumnToWidget $col]
    return [$widget GetBit $ch]
  }

  method SetBit {col ch val} {
    set widget [$self MapColumnToWidget $col]
    return [$widget SetBit $ch $val]
  }

  method MapColumnToWidget {col} {
    return [dict get [dict create 0 $_colA 1 $_colB 2 $_colC 3 $_colD] $col]
  }

  method SyncChanged {name1 name2 op} {
    set outA [set $_outOfSyncAVar]
    set outB [set $_outOfSyncBVar]
    set outC [set $_outOfSyncCVar]
    set outD [set $_outOfSyncDVar]
    if { $outA || $outB || $outC || $outD } {
      $_updateButton state !disabled
    } else {
      $_updateButton state disabled
    }
  }
}

##############################################################################

## @brief The logic for the ChannelEnableDisableView
#
snit::type MDGG16Presenter {

  option -view -default {} -configuremethod SetView
  option -handle -default {} -configuremethod SetHandle

  ## @brief Parse options and construct
  #
  constructor {args} {
    $self configurelist $args
  }

  ## @brief Handler for when the view state is to be transmitted to device
  #
  # First commits the mask and then reads the state back from the device
  #
  method Commit {} {
    $self CommitMask
    $self UpdateViewFromModel
  }

  ## @brief Read the state from the device and synchronize to the view
  #
  method Update {} {
    $self UpdateViewFromModel
  }

  ## @brief Read the state from the device and synchronize to the view
  #
  # @throws error if no handle exists
  # @throws error if no view exists
  # @throws error if communication fails
  method UpdateViewFromModel {} {
  # verify that first there is a device to communicate with
    set handle [$self cget -handle]
    if {$handle eq {}} {
      set msg {MDGG16Presenter::UpdateViewFromModel }
      append msg {Cannot access model because it does not exist.}
      return -code error $msg
    }

    # verify that first there is a view to communicate with
    set view [$self cget -view]
    if {$view eq {}} {
      set msg {MDGG16Presenter::UpdateViewFromModel }
      append msg {Cannot update view because it does not exist.}
      return -code error $msg
    }

    ## set the logical OR bits for A and B
    set mask [$handle GetLogicalORAB]
    # split the mask into a list of bits
    set bits [$self DecodeMaskIntoBits $mask]

    # update the view
    for {set bit 0} {$bit < 16} {incr bit} {
      $view SetBit 0 $bit [lindex $bits $bit]
    }

    for {set bit 16} {$bit < 32} {incr bit} {
      $view SetBit 1 [expr $bit-16] [lindex $bits $bit]
    }

    ## set the logical OR bits for C and D
    set mask [$handle GetLogicalORCD]
    set bits [$self DecodeMaskIntoBits $mask]

    for {set bit 0} {$bit < 16} {incr bit} {
      $view SetBit 2 $bit [lindex $bits $bit]
    }

    for {set bit 16} {$bit < 32} {incr bit} {
      $view SetBit 3 [expr $bit-16] [lindex $bits $bit]
    }

    [$view MapColumnToWidget 0] SetOutOfSync 0
    [$view MapColumnToWidget 1] SetOutOfSync 0
    [$view MapColumnToWidget 2] SetOutOfSync 0
    [$view MapColumnToWidget 3] SetOutOfSync 0
  }

  ## @brief Write the state of the view to the device
  #
  # @throws error if no handle exists
  # @throws error if no view exists
  #
  method CommitMask {} {
  # check for the presence of a handle
    set handle [$self cget -handle]
    if {$handle eq {}} {
      set msg {MDGG16Presenter::CommitMask }
      append msg {Cannot access model because it does not exist.}
      return -code error $msg
    }

    # check for the presence of a view
    set view [$self cget -view]
    if {$view eq {}} {
      set msg {MDGG16Presenter::CommitMask }
      append msg {Cannot update view because it does not exist.}
      return -code error $msg
    }

    ### Logical OR AB
    set bits [list]
    for {set index 0} {$index < 16} {incr index} {
      lappend bits [$view GetBit 0 $index]
    }
    for {set index 0} {$index < 16} {incr index} {
      lappend bits [$view GetBit 1 $index]
    }
    # turn list of bits into a number
    set mask [$self EncodeMaskIntoBits $bits]
    $handle SetLogicalORAB $mask

    ### Logical OR CD 
    set bits [list]
    for {set index 0} {$index < 16} {incr index} {
      lappend bits [$view GetBit 2 $index]
    }
    for {set index 0} {$index < 16} {incr index} {
      lappend bits [$view GetBit 3 $index]
    }
    # turn list of bits into a number
    set mask [$self EncodeMaskIntoBits $bits]
    $handle SetLogicalORCD $mask
  }

  # UTILITY METHODS

  ## @brief Split an integer into a list of bits
  #
  # Given an integer, convert it to a list of 0s and 1s that represent it. Split
  # the bits up and form a list. For example, passing 100 (0x64) into this method, the
  # result will be {0 0 1 0 0 1 1 0}
  #
  # @returns list of 8 bits (least significant bit first)
  method DecodeMaskIntoBits {mask} {
    set bits [list]

    # interpret mask as an actual byte
    set byteValue [binary format iu1 $mask]

    # convert byte into representation of bits as a string
    set count [binary scan $byteValue b32 binRep]

    # split each character up to form a list of bits
    return [split $binRep {}]
  }

  ## @brief Turn a list of bits into an equivalent integer
  #
  # This is the opposite operation of the DecodeMaskIntoBits. Given a list i
  # {0 0 1 0 0 1 1 0}, the method will return a value of 100.
  #
  # @param list of bits (least significant bit first)
  #
  # @returns an integer
  method EncodeMaskIntoBits {bits} {
    set mask 0

    # collapse list of bits into a single word by removing spaces
    set binRepStr [join $bits {}]

    # convert string representation of bits into an actual byte
    set binByte [binary format b32 $binRepStr]

    # interpret the byte as an 16-bit signed number
    set count [binary scan $binByte iu1 mask]

    # becuase the number if signed and padded, we mask out upper bits
    return $mask
  } 

  ## @brief Callback for a "configure -view" operation
  # 
  # Performs the handshake required when the view is set. A new view is passed
  # $self as the value to its -presenter option. If a handle exists, it is
  # appropriate to update the state of the view from it.
  #
  # @param opt    option name (should be -view)
  # @param val    value (name of view)
  method SetView {opt val} {
  # store the new view (opt="-view", val = new view name)
    set options($opt) $val
    $val configure -presenter $self

    # we have a handle already, then update!
    set handle [$self cget -handle]
    if {$handle ne {}} {
      $self UpdateViewFromModel
    }
  }

  ## @brief Callback for a "configure -handle" operation
  #
  # Sets the -handle option to the new value and also updates the view from it
  # if the -view option is set.
  #
  # @param opt  option name (should be -handle)
  # @param val  value (name of handle)
  #
  method SetHandle {opt val} {
  # store the new handle (opt="-handle", val = new handle name)
    set options($opt) $val

    # we have a view already, then update!
    set view [$self cget -view]
    if {$view ne {}} {
      $self UpdateViewFromModel
    }
  }
}

