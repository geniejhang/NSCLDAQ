#!/bin/sh
#
#   Rollout script for See experiments.
#   We must:
#     Do a partial tar of the reference directory:
#       ~/see/template 
#     Make a link to the stage area  which we have to get by prompting the 
#     user.
#
# \
exec wish ${0} ${@} 

lappend auto_path /usr/opt/daq/lib /usr/opt/daq/Scripts

package require Wait


set copyprompt    "   Welcome to the SEETF account rollout script."
append copyprompt "This script should be run once, when a SEE account has "
append copyprompt "been activated. This script copies the reference account "
append copyprompt "to this account, and sets up the event stage area link. "
append copyprompt "Click on \"Next\" below to continue. Click on cancel "
append copyprompt "if you don't want to continue."

set    stageprompt " Event files taken by the NSCL data acquisition system "
append stageprompt "are stored on a large raid disk set.  Links in your "
append stageprompt "home directory hierarchy hide much of this from you. "
append stageprompt "You must, however tell me the \"stage area\" your "
append stageprompt "was assigned so that I can make a link in your home "
append stageprompt "directory that will allow the software to know where "
append stageprompt "store your event files.  Please type the path to that "
append stageprompt "stage area below and click OK."

set passwdprompt    "  In order to record to the event area we will need to "
append passwdprompt "know your password.  To improve the chance you get it "
append passwdprompt "right please fill it in in both of the text entry boxes "
append passwdprompt "below with your account password.  What you type will "
append passwdprompt "not be meaningfully echoed"




set dest ~
set done 0
set StageArea ""

#
#  This procedure initiates a self propagating progress dialog that shows
#  how much stuff has been copied to $dest.
#  Parameters:
#     pid   - process id of copying process.
#             when this exits, we are done.
#             and we increment the global variable done.
#
proc InitiateProgressDialog {pid} {
    global done
    global dest
    #
    # If necessary, create the label widget:
    #
    if {[lsearch -exact [pack slaves .] .progress] == -1} {
	label .progress
	pack .progress
    }

    catch "exec bash -c \"du -sh $dest\"" amountlist
    set amount [lindex $amountlist 0]
    .progress config -text "$amount copied thus far..."
    set status [catch "Wait -pid $pid -poll" waitlist]
    if {$status != 0} {
	set waitlist  12345;		# Some times seems like I need this
    }
    if {[lindex $waitlist 0] == 0} {
	#
	# repropagate:
	#
	after 1000 "InitiateProgressDialog $pid"
    } else {
	# Flag done (breaks the vwait in DoFileCopy).
	#
	incr done
	destroy .progress
	wm withdraw .
    }

}
#
#  Prompt for confirmation that we want to rollout a see experiment
#  if we don't get it exit.
#  If we do then do the copy.
#
proc DoFileCopy {} {
    global copyprompt
    global kcopied
    global done
    global dest
    global env
    #
    #   Responses in dialogs:
    #
    set cancel 0
    set next   1
    #
    set answer [tk_dialog .copyok "Copy?"  $copyprompt questhead 1 \
		cancel "Next >"]
    if {$answer == $cancel} exit
#
#     Do the copy operation.
#

    set wd [pwd]
    cd $dest
    set expandeddest [pwd];           # tilde expanded.
    cd ~see/template
    set pid \
     [exec tar czf -  .icons bin Desktop ReadoutCallouts.tcl config spectcl .bashrc .bash_profile |  \
	  tar  xzfC - $expandeddest  & ]
    set pid [lindex $pid 0]
    wm deiconify .    
    InitiateProgressDialog $pid

    vwait done;				# Don't want to continue till done.

    cd $dest
    catch "exec mkdir scalerdir"
    catch "exec mkdir savedspectra"
    #
    #  Make the link to config:
    #

    catch "exec mkdir -p $env(HOME)/experiment/current"
    catch "exec ln -s $env(HOME)/config $env(HOME)/experiment/current/config" msg
    cd $wd
    #
    #  Remove the expert desktop icons:
    #
    foreach f {Discriminators "HV Control" "Shaper control"} {
	exec rm -f $expandeddest/Desktop/$f
    }

}

#
#  Prompt for a stagearea name:
#
proc PromptForStageArea {} {
    global stageprompt
    global done
    global StageArea

    toplevel .t
    frame .t.prompter
    frame .t.action -relief groove -borderwidth 3

    #
    #  set up the prompt area:
    #
    message .t.prompter.explanation -text $stageprompt
    entry .t.prompter.path        -textvariable StageArea
    pack  .t.prompter.explanation .t.prompter.path
    pack  .t.prompter -fill x
    #
    #  The action area just has an Ok button centered.
    #
    button .t.action.ok -text Ok -command {incr done}
    pack   .t.action.ok -anchor c
    pack   .t.action    -fill x

    while {1} {
	vwait done
	if {![file writable $StageArea] } {
	    tk_dialog .badfile "Bad stagearea" \
		"Stage area must be a directory you can write to" \
		error 0 Ok
	    set StageArea ""
	} else {
	    break
	}
		 
    }
    set d [pwd]
    cd $StageArea
    set StageArea [pwd]
    cd $d
    destroy .t
    return $StageArea

}
#
#  Prompt for contents of .passwd file.
#
proc PromptPasswd {} {
    global dest
    global passwdprompt
    toplevel .pass
    frame     .pass.prompt
    frame    .pass.action  -relief groove -borderwidth 3

    message .pass.prompt.explanation -text $passwdprompt
    entry   .pass.prompt.pass1       -show * 
    entry   .pass.prompt.pass2       -show * 
    pack    .pass.prompt.explanation
    pack    .pass.prompt.pass1
    pack    .pass.prompt.pass2
    pack    .pass.prompt -fill x

    button  .pass.action.ok  -text Ok -command {incr done}
    pack    .pass.action.ok
    pack    .pass.action -fill x

    while {1} {
	vwait done
	set pass1 [.pass.prompt.pass1 get]
	set pass2 [.pass.prompt.pass2 get]
	if {$pass1 != $pass2} {
	    #  Empty and keep prompting.
	    .pass.prompt.pass1 delete @0 end
	    .pass.prompt.pass2 delete @0 end
	    tk_dialog .e "Mismatch: " \
		"The two password strings don't match" error \
		0 "Ok"

	} else {
	    # Match..
	   
	    set fd [open $dest/.passwd w]
	    puts $fd $pass1
	    close $fd
	    exec chmod 0600 $dest/.passwd
	    break
	}
    }
}


set wd [pwd]
cd $dest
set dest [pwd]
cd $wd

wm withdraw .
DoFileCopy
set stagearea [PromptForStageArea]
puts "Linking $dest/stagearea -> $stagearea"
catch "exec ln -s $stagearea $dest/stagearea" errormsg

PromptPasswd

exec ~see/see/adm/ssh-setup

exit
