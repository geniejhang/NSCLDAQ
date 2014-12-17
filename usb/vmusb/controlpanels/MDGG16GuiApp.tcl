
package provide mdgg16guiapp 1.0

package require snit
package require mdgg16gui
package require mdgg16proxy


snit::type MDGG16AppOptions {
  option -module  -default {}
  option -host  -default localhost
  option -port  -default 27000

  constructor {args} {
    $self configurelist $args
  }
}


snit::type MDGG16GuiApp {

  option -name -default {.view}

  variable _menu

  component _options
  component _proxy
  component _view
  component _presenter

  delegate option * to _options

  constructor {args} {
    install _options using MDGG16AppOptions %AUTO%
    $self configurelist $args

    install _proxy using MDGG16Proxy %AUTO% -server [$self cget -host] \
                                            -port [$self cget -port] \
                                            -module [$self cget -module]

    install _view using MDGG16View [$self cget -name]
    install _presenter using MDGG16Presenter %AUTO% -view [$self cget -name] \
      -handle $_proxy

    $self configureMenu

    grid [$self cget -name] -sticky nsew -padx 8 -pady 8
    grid rowconfigure . 0 -weight 1
    grid columnconfigure . 0 -weight 1

  }

  destructor {

  }


  method configureMenu {} {

    set top [winfo toplevel [$self cget -name]]
    if {$top eq "."} {
      set _menu [menu ${top}menu]
    } else {
      set _menu [menu $top.menu]
    }
    $_menu add command -label "Save as..." -command [mymethod SaveAs]

    . configure -menu .menu
  }

  method SaveAs {} {
    set path [tk_getSaveFile -confirmoverwrite 1 -title {Save as} ] 
    if {$path ne {}} {
      $_presenter SaveCurrentStateToFile $path
    }
  }



}
