package require Tk


set installDir $env(READOUTDIR)
set scriptDir [file join $installDir gui]

wm withdraw .

toplevel .chooser
button   .chooser.standard -text {Normal}    -command normalMode
button   .chooser.setup    -text {LED setup} -command  setupMode
button   .chooser.calibrate -text {Calibration} -command calibrationMode

pack .chooser.standard .chooser.setup .chooser.calibrate

#
# Bring up normal readout GUI
#
proc normalMode {} {
    destroy .chooser
    wm deiconify .
    uplevel #0 { source [file join $scriptDir standardReadout.tcl] }
}


#
#  Bring up LED setup mode. GUI
#
proc setupMode {} {
    destroy .chooser
    wm deiconify .
    uplevel #0 {source [file join $scriptDir spectrumMaker.tcl]}
}



#
#  Bring up the calibration mode GUI.
#
proc calibrationMode {} {
    destroy .chooser
    wm deiconify .
    uplevel #0 {source [file join $scriptDir calibration.tcl]}
}