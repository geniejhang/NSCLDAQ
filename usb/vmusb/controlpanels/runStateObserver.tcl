

package provide RunStateObserver 1.0

package require snit
package require TclRingBuffer
package require ring
package require Thread


snit::type RunStateObserver {

  option -ringurl -default {} -configuremethod onRingURL

  option -onbegin -default {}
  option -onpause -default {}
  option -onresume -default {}
  option -onend   -default {}

  variable acqThread {}

  constructor {args} {
    $self configurelist $args
  }

  destructor {
    $self detachFromRing
  }

  method onRingURL {opt val} {
    # I shamelessly stole this nice regular expression recipe for matching
    # URLs from http://www.beedub.com/book/2nd/regexp.doc.html.
    set uriPattern {([^:]+)://([^:/]+)(:([0-9]+))?(/.*)}
    if {![regexp $uriPattern $val match protocol server x port path]} {
      return -code error "Invalid URI passed to RunStateObserver -ringurl option. Check URI syntax."
    }
    # drop the leading backslash from the path
    set path [string range $path 1 end]

    set options($opt) $val
    
  }

  method attachToRing {} {
   set acqThread [$self startAcqThread ]
  }

  method detachFromRing {} {
    if {$acqThread ne {}} {
      thread::send $acqThread [list set running 0]
      thread::join $acqThread
      set acqThread {}
    }

  }

  method startAcqThread {} {
    set acqThread [thread::create -joinable]
    if {[thread::send $acqThread [list lappend auto_path $::env(DAQLIB)] result]} {
        puts "Could not extend thread's auto-path"
        exit -1
    }
    if {[thread::send $acqThread [list package require TclRingBuffer] result]} {
        puts "Could not load RingBuffer package in acqthread: $result"
        exit -1
    }
    
    if {[thread::send $acqThread [list ring attach $options(-ringurl)] result]} {
        puts "Could not attach to ring buffer in acqthread $result"
        exit -1
    }
    
    #  The main loop will forward data to our handleData item.
    
    set myThread [thread::id]
    set getItems "proc getItems {obj tid uri} { 
        while \$::running {                                             
            set ringItem \[ring get \$uri {1 2 3 4}]             
            thread::send \$tid \[list \$obj handleData \$ringItem]     
        }                                                     
    }                                                         
    set running 1
    getItems $self $myThread $options(-ringurl)
    "
    thread::send -async $acqThread $getItems

    
    return $acqThread
  }

  method handleData {item} {
    puts $item
    
    switch [dict get $item type] {
      "Begin Run"  { 
        if {$options(-onbegin) ne {} } {
            uplevel #0 $options(-onbegin) [list $item]
        }
      }
      "End Run"  { 
        if {$options(-onend) ne {} } {
          uplevel #0 $options(-onend) [list $item]
        }
      }
      "Pause Run"  { 
        if {$options(-onpause) ne {} } {
          uplevel #0 $options(-onpause) [list $item]
        }
      }
      "Resume Run"  { 
        if {$options(-onresume) ne {} } {
          uplevel #0 $options(-onresume) [list $item]
        }
      } 
      default { puts [dict get $item type] }
    }
  }




}
