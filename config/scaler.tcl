
set ::configdir $::env(CONFIGDIR)
set ::daqroot   $::env(DAQROOT)
set ::exproot   $::env(HOME)/experiment
set ::current   $::env(HOME)/experiment/current

#
# Channel definitions:
#

# Trigger channels.

channel master 0.1
channel master.live 1.1


# PPAC channels.

channel see.ppac.u  16.1
channel see.ppac.d  17.1
channel see.ppac.l  18.1
channel see.ppac.r  19.1
channel see.ppac.a  20.1


# Scintilator channels:

channel see.scint.u 24.1
channel see.scint.d 25.1
channel see.scint.l 26.1
channel see.scint.r 27.1

#
#  Page display definitions:
#

page ALL "All of the scalers"


display_ratio  ALL master.live master

display_ratio  ALL  see.ppac.u see.ppac.d
display_ratio  ALL  see.ppac.l see.ppac.r
display_single ALL  see.ppac.a

display_ratio  ALL see.scint.u see.scint.d
display_ratio  ALL see.scint.l see.scint.r

# Strip charts

stripparam see.scint.u
stripparam see.scint.d
stripparam see.scint.l
stripparam see.scint.r

stripratio see.scint.u see.scint.d
stripratio see.scint.l see.scint.r




#  Code to implement scaler rate logging:


set open 0
set fd   ""


proc UserBeginRun {} {
    global fd
    global open
    global current
    
    set title [getTitle]
    set run   [getRunNumber]

    set fd [open $current/scint.log w]
    
    puts $fd "\"$title\",$run"
    puts $fd "Time,up,down,left,right"
    flush $fd
    set open 1
}

proc UserEndRun {} {
    global fd
    global open

    if {$open} {
	close $fd
	set open 0
    }
}

proc UserUpdate {} {
    global open
    global fd


    if {$open} {
	set up    [getRate see.scint.u]
	set down  [getRate see.scint.d] 
	set left  [getRate see.scint.l]
	set right [getRate see.scint.r]
        
	set time [clock seconds]
	set time [clock format $time]
        
	puts $fd "\"$time\",$up,$down,$left,$right"

	flush $fd
    }
}
