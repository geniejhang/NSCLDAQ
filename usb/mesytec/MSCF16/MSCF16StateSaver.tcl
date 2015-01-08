
package provide mscf16statesaver 1.0

package require snit
package require mscf16commandlogger
package require mscf16scriptheadergenerator

snit::type MSCF16StateSaver {

  variable _opts
  variable _presenter
  component _headerGenerator

  constructor {opts presenter} {
    set _opts $opts
    set _presenter $presenter
    install _headerGenerator using MSCF16ScriptHeaderGenerator %AUTO% $_opts
  }

  destructor {
    catch {$_headerGenerator destroy}
  }

  method SaveState {path} {
    if {[catch {open $path w} logFile]} {
      return -code error "Failed to open file $path"
    }

    # create the header lines that construct the device
    set headerLines [$_headerGenerator generateHeader]
    foreach line $headerLines {
      chan puts $logFile $line
    }

    set logger [MSCF16CommandLogger %AUTO% $logFile]

    set oldUpdateState [$_presenter cget -autoupdate]
    set oldHandle [$_presenter cget -handle]

    # turn of autoupdate
    $_presenter configure -autoupdate 0
    $_presenter configure -handle $logger

    $_presenter Commit 

    $_presenter configure -autoupdate $oldUpdateState
    $_presenter configure -handle $oldHandle
    $logger destroy


    # done...
    close $logFile
  }
}
