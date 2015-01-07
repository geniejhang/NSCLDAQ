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



package provide mscf16gui 1.0

package require snit
package require Tk
package require ChannelLabel
package require TransientLabel

# the original implementation used the convention of 
# starting counting at 1 rather than 0. For programming
# simplicity, the code uses the C convention of counting from
# 0. However, the user only ever cares about what we show them. 
# So we will just display the numbers as though we used a counting
# scheme starting from 1.
namespace eval MSCF16ChannelNames {
  variable chan0 Ch1
  variable chan1 Ch2
  variable chan2 Ch3
  variable chan3 Ch4
  variable chan4 Ch5
  variable chan5 Ch6
  variable chan6 Ch7
  variable chan7 Ch8
  variable chan8 Ch9
  variable chan9 Ch10
  variable chan10 Ch11
  variable chan11 Ch12
  variable chan12 Ch13
  variable chan13 Ch14
  variable chan14 Ch15
  variable chan15 Ch16
}


snit::widget MSCF16Form {

  option -committable -default 1 -configuremethod SetCommittable ;#!< changing widget vals schedules commit

  variable _scheduledCommitOpId -1 

  method SetCommittable {opt val} {
    set options($opt) $val
  }

  # there are a lot of variable managed by this...
  variable monitor
  variable remote
  variable single

  variable th0
  variable th1
  variable th2
  variable th3
  variable th4
  variable th5
  variable th6
  variable th7
  variable th8
  variable th9
  variable th10
  variable th11
  variable th12
  variable th13
  variable th14
  variable th15
  variable th15
  variable th16

  variable pz0
  variable pz1
  variable pz2
  variable pz3
  variable pz4
  variable pz5
  variable pz6
  variable pz7
  variable pz8
  variable pz9
  variable pz10
  variable pz11
  variable pz12
  variable pz13
  variable pz14
  variable pz15
  variable pz16

  variable ga0
  variable ga1
  variable ga2
  variable ga3
  variable ga4

  variable sh0
  variable sh1
  variable sh2
  variable sh3
  variable sh4

  option -presenter  -default {} 
  component _statusLbl

  constructor {args} {
    $self configurelist $args

    $self InitArray
    $self SetupGUI
    
  }

  method InitArray {} {
    set monitor 0
    set remote on
    set single common
    for {set i 0} {$i < 5} {incr i} {
      set ga$i 0
      set sh$i 0
    }

    for {set i 0} {$i < 17} {incr i} {
      set th$i 0
      set pz$i 0
    }
  }

  method SetupGUI {} {

    # build the table
    set w $win.table
    ttk::frame $w

    # build the header row
    set w $win.table.header
    ttk::frame $w -style Header.TFrame
    ttk::label $w.na -text Name -padding 4 -style Header.TLabel
    ttk::label $w.ch -text Channel -padding 4 -style Header.TLabel
    ttk::label $w.ga -text Gain -padding 4 -style Header.TLabel
    ttk::label $w.sh -text Shaping -padding 4 -style Header.TLabel
    ttk::label $w.pz -text "Pole zero" -padding 4 -style Header.TLabel
    ttk::label $w.th -text Threshold -padding 4 -style Header.TLabel
    ttk::label $w.mo -text Monitor -padding 4 -style Header.TLabel
    grid $w.na $w.ga $w.sh $w.pz $w.th $w.mo -sticky sew
    grid columnconfigure $w {0 1 2 3 4 5} -weight 1 -uniform a

    # grid rows 4 at a time
    for {set group 0} {$group < 4} {incr group} {

      # every 4 rows should be grouped...
      set w $win.table.group$group
      ttk::frame $w -style Group.TFrame

      # these are the widgets that only happen every 4 rows
      ttk::spinbox $w.ga$group -textvariable [myvar ga$group] \
                   -width 4 -from 0 -to 15 \
                    -style Group.TSpinbox
      trace add variable [myvar ga$group] write [mymethod OnGainChanged]
      ttk::spinbox $w.sh$group -textvariable [myvar sh$group] \
                   -width 4 -from 0 -to 3 \
                    -style Group.TSpinbox
      trace add variable [myvar sh$group] write [mymethod OnShapingTimeChanged]

      # construct widgets that happen every single row
      for {set subrow 0} {$subrow < 4} {incr subrow} {
        set i [expr $group*4+$subrow]
        ChannelLabel $w.na$i -width 8 -textvariable MSCF16ChannelNames::chan$i \
          -style Group.TEntry -defaultstring "Ch[expr $i+1]"
        ttk::spinbox $w.pz$i -textvariable [myvar pz$i] -width 4 \
          -from 0 -to 255 \
          -style Group.TSpinbox
        trace add variable [myvar pz$i] write [mymethod OnPoleZeroChanged]

        ttk::spinbox $w.th$i -textvariable [myvar th$i] -width 4 \
          -from 0 -to 255 \
          -style Group.TSpinbox
        trace add variable [myvar th$i] write [mymethod OnThresholdChanged]

        ttk::radiobutton $w.mo$i -variable [myvar monitor] \
          -value $i -style Group.TRadiobutton
        trace add variable [myvar monitor] write [mymethod OnMonitorChanged]

        # within this loop, we will grid... the first row should have
        # the widgets that only appear once every 4 rows...
        if {$subrow==0} {
          grid $w.na$i $w.ga$group $w.sh$group $w.pz$i $w.th$i $w.mo$i \
            -sticky news -padx 4 -pady 4
        } else {
        # the remaining 3 rows in the group
          grid $w.na$i ^ ^ $w.pz$i $w.th$i $w.mo$i \
            -sticky news -padx 4 -pady 4
        }
      }

      grid columnconfigure $w {0 1 2 3 4 5} -weight 1 -uniform a
      grid rowconfigure $w {0 1 2 3} -weight 1
    }

    # common widgets
    set w $win.table.group4
    ttk::frame $w
    ttk::checkbutton $w.si -text Common -variable [myvar single]\
                           -onvalue common -offvalue individual 
    trace add variable [myvar single] write [mymethod OnModeChanged]

    ttk::spinbox $w.gac -textvariable [myvar ga4] -width 4 \
                        -from 0 -to 15 
    trace add variable [myvar ga4] write [mymethod OnGainChanged]

    ttk::spinbox $w.shc -textvariable [myvar sh4] -width 4 \
                        -from 0 -to 15 
    trace add variable [myvar sh4] write [mymethod OnShapingTimeChanged]

    ttk::spinbox $w.pzc -textvariable [myvar pz16] -width 4 \
                        -from 0 -to 255 
    trace add variable [myvar pz16] write [mymethod OnPoleZeroChanged]

    ttk::spinbox $w.thc -textvariable [myvar th16] -width 4 \
                        -from 0 -to 255 
    trace add variable [myvar th16] write [mymethod OnThresholdChanged]

    grid $w.si  $w.gac $w.shc $w.pzc $w.thc x -sticky news
    grid columnconfigure $w {0 1 2 3 4 5} -weight 1 -uniform a

    grid $win.table.header -sticky nsew -padx 4
    grid $win.table.group0 -sticky nsew -padx 4 -pady 4
    grid $win.table.group1 -sticky nsew -padx 4 -pady 4
    grid $win.table.group2 -sticky nsew -padx 4 -pady 4
    grid $win.table.group3 -sticky nsew -padx 4 -pady 4
    grid $win.table.group4 -sticky nsew -padx 4 -pady 4
    grid columnconfigure $win.table 0 -weight 1
    grid rowconfigure $win.table {0 1 2 3 4 5} -weight 1

    # build remote frame
    set w $win.remote
    ttk::frame $w
    ttk::checkbutton $w.remote -text Remote -variable [myvar remote] -onvalue on \
                     -offvalue off -command [mymethod RemoteLocal]
    set _statusLbl [TransientLabel $w.status -text {}]
    grid $_statusLbl $w.remote -sticky ew
    grid rowconfigure $w 0 -weight 1
    grid columnconfigure $w 0 -weight 1


    # grid the large chunks together
    grid $win.table -sticky nsew -padx 4 -pady 4
    grid $win.remote -sticky nsew -padx 4 -pady 4
    grid columnconfigure $win 0 -weight 1
    grid rowconfigure $win 0 -weight 1
  }

  method SetStateOfIndividualControls {state} {
    for {set grp 0} {$grp < 4} {incr grp} {

      $win.table.group$grp.ga$grp state $state
      $win.table.group$grp.sh$grp state $state

      for {set subgrp 0} {$subgrp<4} {incr subgrp} {
        set i [expr $grp*4+$subgrp]
        $win.table.group$grp.pz$i state $state
        $win.table.group$grp.th$i state $state
#        $win.table.group$grp.mo$i state $state
      }
    }
  }

  method SetStateOfCommonControls {state} {
    $win.table.group4.gac state $state
    $win.table.group4.shc state $state
    $win.table.group4.pzc state $state
    $win.table.group4.thc state $state
  }

  method RemoteLocal {} {
    if {[$self cget -presenter] ne {}} {
      [$self cget -presenter] OnEnableRC $remote
      $_statusLbl configure -text "Transitioned to remote $remote"
    }
  }

  # Setter/Getter interface
  method SetThreshold {index val} {set th$index $val }
  method GetThreshold index {return [set th$index] }

  method SetPoleZero {index val} { set pz$index $val }
  method GetPoleZero index { return [set pz$index] }

  method SetMonitor {val} { set monitor $val }
  method GetMonitor {} { return $monitor }

  method SetGain {index val} { set ga$index $val  }
  method GetGain {index} { return [set ga$index]}

  method SetShapingTime {index val} { set sh$index $val }
  method GetShapingTime {index} { return [set sh$index] }

  method SetMode {val} {set single $val }
  method GetMode {} { return $single }


  method ExtractEndingIndex {string pattern} {
    set index [string last $pattern $string]
    set index [expr $index+[string length $pattern]]
    return [string range $string $index end]
  }

  method OnGainChanged {name1 name2 op} {
    set index [$self ExtractEndingIndex $name1 ga]
    $self DelayedChanCommit Gain $index [set $name1]
  }

  method OnShapingTimeChanged {name1 name2 op} {
    set index [$self ExtractEndingIndex $name1 sh]
    $self DelayedChanCommit ShapingTime $index [set $name1]
  }

  method OnPoleZeroChanged {name1 name2 op} {
    set index [$self ExtractEndingIndex $name1 pz]
    $self DelayedChanCommit PoleZero $index [set $name1]
  }

  method OnThresholdChanged {name1 name2 op} {
    set index [$self ExtractEndingIndex $name1 th]
    $self DelayedChanCommit Threshold $index [set $name1]
  }

  method OnMonitorChanged {name1 name2 op} {
    $self DelayedCommit Monitor [expr [set $name1]]
  }
  
  method OnModeChanged {name1 name2 op} {

    if {[$self cget -presenter] ne {}} {
      [$self cget -presenter] OnSetMode $single
    }

    if {$single eq "common"} {
      $self SetStateOfIndividualControls disabled
      $self SetStateOfCommonControls !disabled
    } else {
      $self SetStateOfIndividualControls !disabled
      $self SetStateOfCommonControls disabled
    }
    $_statusLbl configure -text "Transitioned to $single mode"
  }
  
  method DelayedChanCommit {param chan val} {
    # it doesn't matter what changed... we will just schedule the commit
    if {([$self cget -committable]==1) && ([$self cget -presenter] ne {})} {
      if {$_scheduledCommitOpId != -1} {
        after cancel $_scheduledCommitOpId
        set _scheduledCommitOpId -1
      }
      set _scheduledCommitOpId [after 350 \
           [list [$self cget -presenter] CommitSingleChan $param $chan $val]]
    }
  }

  method DelayedCommit {param val} {
    # it doesn't matter what changed... we will just schedule the commit
    if {([$self cget -committable]==1) && ([$self cget -presenter] ne {})} {
      if {$_scheduledCommitOpId != -1} {
        after cancel $_scheduledCommitOpId
        set _scheduledCommitOpId -1
      }
      set _scheduledCommitOpId [after 350 \
        [list [$self cget -presenter] CommitSingle $param $val]]

    }
  }

  method SetStatus {message} {
    $_statusLbl configure -text $message
  }
}

# --------------------------------------------------------------------------- #

##
#
#
snit::type MSCF16Presenter {

  option -view -default {} -configuremethod SetView
  option -handle -default {} -configuremethod SetHandle

  constructor {args} {
    $self configurelist $args
  }

  method Commit {} {
    $self CommitViewToModel
    $self UpdateViewFromModel
  }

  method CommitSingleChan {param index val} {
    set handle [$self cget -handle]
    if {$handle ne {}} {
        $handle Set$param $index $val
        $self UpdateViewFromModel
        [$self cget -view] SetStatus "Successfully updated $param $index"
    }
  }

  method CommitSingle {param val} {
    set handle [$self cget -handle]
    if {$handle ne {}} {
      $handle Set$param $val
      $self UpdateViewFromModel
      [$self cget -view] SetStatus "Successfully updated $param"
    }
  }

  method UpdateViewFromModel {} {
    set view [$self cget -view]
    set handle [$self cget -handle]

    if {($view ne {}) && ($handle ne {})} {

      # make sure the following manipulation of variables does not 
      # schedule a subsequent commit. If it did, this would cause an infinite
      # loop... not a good thing
      $view configure -committable 0

      $view SetMode [$handle GetMode]
      $view SetMonitor [$handle GetMonitor]

      for {set ch 0} {$ch < 17} {incr ch} {
        $view SetThreshold $ch [$handle GetThreshold $ch]
        $view SetPoleZero $ch [$handle GetPoleZero $ch]
      }

      for {set grp 0} {$grp < 5} {incr grp} {
        $view SetGain $grp [$handle GetGain $grp]
        $view SetShapingTime $grp [$handle GetShapingTime $grp]
      }

      # now we are done updating. If the user wants to manipulate the widgets,
      # then it will schedule a later commit.
      $view configure -committable 1
    }
  }

  method CommitViewToModel {} {
    set view [$self cget -view]
    set handle [$self cget -handle]

    if {($view ne {}) && ($handle ne {})} {
      $handle SetMode [$view GetMode]
      $handle SetMonitor [$view GetMonitor]

      for {set ch 0} {$ch < 17} {incr ch} {
        $handle SetThreshold $ch [$view GetThreshold $ch]
        $handle SetPoleZero $ch [$view GetPoleZero $ch]
      }

      for {set grp 0} {$grp < 5} {incr grp} {
        $handle SetGain $grp [$view GetGain $grp]
        $handle SetShapingTime $grp [$view GetShapingTime $grp]
      }
    }
  }

  method OnSetMode {mode} {
    if {[$self cget -handle] ne {}} {
      [$self cget -handle] SetMode $mode
    }
  }

  method OnSetMonitor {chan} {
    if {[$self cget -handle] ne {}} {
      [$self cget -handle] SetMonitor $chan
    }
  }

  method OnEnableRC {state} {
    if {[$self cget -handle] ne {}} {
      [$self cget -handle] EnableRC $state
    }
  }

  ##########################################################
  
  ##
  #
  #
  method SetView {opt val} {
    set options($opt) $val
    $val configure -presenter $self

    if {[$self cget -handle] ne {}} {
      $self UpdateViewFromModel
    }
  }

  method SetHandle {opt val} {
    set options($opt) $val

    if {[$self cget -view] ne {}} {
      $self UpdateViewFromModel
    }
  }

}
