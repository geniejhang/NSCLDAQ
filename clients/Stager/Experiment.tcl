#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


# (C) Copyright Michigan State University 1937, All rights reserved 
#
#  Experiment management.
#  This file contains the Experiment package which manages the run
#  directory structure for the user.
#
package provide Experiment 1.0
package require ExpFileSystem
package require ReadoutControl
package require InstallRoot

namespace eval  Experiment {
    variable ftpLoghost
    variable ftpLogpasswd
    variable Logrecorder "[InstallRoot::Where]/bin/eventlog"
    variable SourceURL
    variable EventFilePath
    variable EventlogPid 0

    proc SetFtpLogInfo {host passwd} {
	variable ftpLoghost
	variable ftpLogpasswd

	set ftpLoghost   $host
	set ftpLogpasswd $passwd
    }
    proc SetSourceHost {host} {
	variable SourceURL
	set SourceURL tcp://$host:2602/
    }
    #
    #  Biggest run returns the number of the biggest run number
    #  used so far.
    #
    proc BiggestRun {} {
	set runfile [ExpFileSystem::GetRoot]/.lastrun
	if {![file exist $runfile]} {
	    exec echo 0 >$runfile
	}
	set fd [open $runfile r]
	set line1 [gets $fd]
	close $fd
	if {[scan $line1 "%d" run] == 0} {set run 0}
	incr run
	return $run
    }
    proc UpdateBiggestRun {num} {
	set biggest [BiggestRun]
	if {$num >= $biggest} {
	    exec echo $num > [ExpFileSystem::GetRoot]/.lastrun
	}
    }
    #
    # Run is about to begin.  
    # If taping was enabled the following:
    #    1. Start the taper in single shot, ftp mode.
    #    2. Make a symbolic link in the current directory to the event log
    #       file.
    # If the proc OnBegin is defined invoke it.
    #
    proc RunBeginning {} {
	variable SourceURL
	variable ftpLoghost
	variable ftpLogpasswd
	variable Logrecorder
	variable EventFilePath
	variable EventlogPid
	set      nrun [ReadoutControl::GetRun]
	if {[ReadoutControl::isTapeOn]} {
	    #
	    # Start the event logger.
	    #
	    set Stagedir [ExpFileSystem::WhereisCurrentEventData]
	    set currentdata [ExpFileSystem::WhereisCurrentData]
	    set user [exec whoami]
	    cd $Stagedir;		# Unfortunately this is necessary to
	    #                             make direct eventlogging work..

	    set EventlogPid [exec $Logrecorder -one -source $SourceURL &]
	    
	    #
	    #  Make a link in the current dir to the staged data.
	    #
	    set runname [ExpFileSystem::GenRunFile $nrun]
	    file delete $currentdata/$runname ;# Kill any hanging link.
	    exec ln -s $Stagedir/$runname $currentdata/$runname 
	    #
	    #  Wait for the logger to be ready to accept data:
	    #
	    while {![file exists .ready]} {
		update idle
		update idle
		update idle
		after 1000
	    }
	    file delete -force .ready
	}
	#
	#  If OnBegin is defined, call it.
	#
	set OnBegin [namespace eval :: {info proc OnBegin}]
	if {$OnBegin != ""} {
	    $OnBegin $nrun
	}
	set EventFilePath [ExpFileSystem::WhereisCurrentEventData]
	append EventFilePath "/" [ExpFileSystem::GenRunFile $nrun]
    }
    # If taping is on:
    #  1. Wait for the taper to end (.done appears in this dir).
    #  2. Delete the .done file.
    #  3. unlink the current link to the event file.
    #  4. Move the event data to the complete dir.
    #  5. Copy current to the appropriate run directory (following links)
    #  6. Create a new link to the event data in the run directory.
    # If OnEnd is defined, call it.
    #
    proc RunEnded {} {
	variable EventlogPid

#	puts "RunEnded: [pwd]"
	set nrun [ReadoutControl::GetRun]
	# IF OnEnd is defined, call it:
	#
	set OnEnd [namespace eval :: {info proc OnEnd}]
	if {$OnEnd != ""} {$OnEnd $nrun}

	if {[ReadoutControl::isTapeOn]} {
	    # 
	    # Wait for taper to end, and ensure that event file
	    # has been flushed to disk.
	    #
	    set EventFile [ExpFileSystem::WhereisCurrentEventData]
	    append EventFile "/" [ExpFileSystem::GenRunFile $nrun]
	    #
	    #  If the run file is on an NFS mount the first time it
	    #  appears to take some time to get the cache flushed out but
	    #  the following probably will take care of that:
	    #
	    
	    after 1000		# Probably it's ready by now.
	    catch "exec ls $EventFile"
	    #
#	    puts "Waiting for .done and $EventFile"
	    while {![file exists .done] || ![file exists $EventFile]} {
		after 1000
#		puts "Wait .done"
		update idle
		update idle
		update idle
	    }
	    # Delete the .done file
	    #
	    
	    file delete -force  .done
	    set EventlogPid     0
	    #
	    # Unlink the link to the event data.
	    #
	    set root    [ExpFileSystem::GetRoot]
	    set current [ExpFileSystem::WhereisCurrentData]
	    set rundir  $root/run$nrun
	    set rfile   [ExpFileSystem::GenRunFileBase $nrun]
	    append rfile "*.evt"         ;# There can be several segments.
	    
	    set links [glob $current/$rfile]
	    set files [glob [ExpFileSystem::WhereisCurrentEventData]/$rfile]

	    #
	    #  Now delete the links:
	    #
	    foreach link $links {file delete -force $link}
	    
	    #  Move the event data:
	    set destdir [ExpFileSystem::WhereareCompleteEventFiles]
	    file attributes $destdir -permissions 0750;     #  Directory mode to rwxr-x---
	    foreach file $files {
		set basefile [file tail $file]
		set destfile [file join $destdir  $basefile]
		file rename -force $file $destfile
		puts "Setting $destfile permissions to 0440"
		file attributes $destfile -permissions 0440;    # Make file readonly.
	    } 

	    file attributes $destdir -permissions 0550;   # Dir mode to r-xr-x---
	    #
	    # Copy 'current' dir.
	    #
	    file mkdir $root/run$nrun

	    catch {
		 exec \
   	       sh << "(cd $current; tar chf - .)|(cd $root/run$nrun; tar xpf -)"
	    }
	    #
	    # Create a new links to the event data in the runnum dir.
	    #
	    foreach segment $files {
		set name [file tail $segment]
		set target [file join $destdir $name]
		set source [file join $root run$nrun $name]
		catch {
		    exec ln -s $target $source
		}
	    }
	    
	    #
	    # Update last taped run.
	    #
	    UpdateBiggestRun $nrun

	}
    }
    #
    #  If the OnPause proc is defined, call it.
    #
    proc RunPaused {} {

	set nrun [ReadoutControl::GetRun]
	set OnPause [namespace eval :: {info proc OnPause}]
	if {$OnPause != ""} {$OnPause $nrun}
    }
    #
    #  If the onResume proc is defined, call it.
    #
    proc RunResuming {} {
	set nrun [ReadoutControl::GetRun]
	set OnResume [namespace eval :: {info proc OnResume}]
	if {$OnResume != ""} {$OnResume $nrun}
	
    }
    # If the OnStart proc is defined call it.
    #
    proc ReadoutStarting {} {
	set OnStart [namespace eval :: {info proc OnStart}]
	if {$OnStart != ""} {$OnStart}
    }
    #
    #  Perform an emergency end:  If the enentlog program is 
    #  running it is killed, and the .done file created.
    #  Next the RunEnded process is called.
    #
    proc EmergencyEnd {} {
	variable EventlogPid

	if {$EventlogPid != 0} {
	    exec kill -KILL $EventlogPid
	    exec touch .done
	}
	RunEnded
	set nrun [ReadoutControl::GetRun]
	set rundir [ExpFileSystem::GetRoot]/run$nrun
	file mkdir  $rundir
	exec touch $rundir/000RunAbnormallyEnded
    }
    #  Clean up orphaned event files in the stage area's current 
    #  and, manage any dangling links in the experiment current dir.
    #  If there is a corresponding run directory, complete the link.
    #  If not, make a link in the orphaned dir.
    #
    proc CleanOrphans {} {
	set Eventdir [ExpFileSystem::WhereisCurrentEventData]
	set completeEventDir [ExpFileSystem::WhereareCompleteEventFiles]
	set orphanfiles [glob -nocomplain $Eventdir/run*.evt]
	set root [ExpFileSystem::GetRoot]
	file mkdir $root/orphans

	# Take care of orphans in Stage current dir.

	foreach file $orphanfiles {
	    set name [file tail $file]
	    if {[scan $name "run%d-%d.evt" run size] == 2} {
		# Valid run file name.  Copy to completed:
		
		set dest [ExpFileSystem::WhereareCompleteEventFiles]
		exec mv $file $dest/$name
		set  eventfile $dest/$name

		set  current [ExpFileSystem::WhereisCurrentData]
		if {[file exist $current/$name]} {
		    file delete $current/$name
		    set destdir [ExpFileSystem::WhereisRun $run]
		    if {![file exist $destdir/$name]} {
			catch {
			    exec ln -s $eventfile $root/orphans/$name
			}
		    }

		} else {
		    #  no link to orphaned event file in current...
		    catch {
			exec ln -s $eventfile $root/orphans/$name
		    }
		}

	    } else {
		# Invalid filename... just gets copied to orphans.

		set stage [ExpFileSystem::GetStage]
		file mkdir $stage/orphan
		catch {
		    exec mv $file $stage/orphan/$name
		}
	    }
	}
	# Take care of orphans in experiment current dir.
	
	set Currentdir [ExpFileSystem::WhereisCurrentData]
	set orphanfiles [glob -nocomplain $Currentdir/*.evt]
	

	foreach file $orphanfiles {
	    set name [file tail $file]
	    if {[scan $name "run%d-%d.evt" run size] == 2} {
		set destdir [ExpFileSystem::WhereisRun $run]

		#
		# Note, non event files are considered to belong
		# in the current dir.  This file, however is an
		# Event file or link.
		#
		if {[file type $file] == "link"} {
		    file delete $file;	# Either way it's out of current.

		    # Translate the link if it's not in the destdir yet.

		    if {![file exists [file join $destdir $name]]} {
			set eventFile [file join $completeEventDir $name]
			if {[file exists $eventFile]} {
			    # Make new link in run directory.
			    set linkname [file join $destdir $name]
			    file mkdir $destdir
			    exec ln -s $eventFile $linkname
			    puts "Created link: $linkname -> $eventFile"
			}
		    }
		}
	    }
	}

    }
    #
    #  Register callbacks with runcontrol stuff:
    #
    proc Register {} {
	ReadoutControl::SetOnBegin  Experiment::RunBeginning
	ReadoutControl::SetOnEnd    Experiment::RunEnded
	ReadoutControl::SetOnPause  Experiment::RunPaused
	ReadoutControl::SetOnResume Experiment::RunResuming
	ReadoutControl::SetOnLoad   Experiment::ReadoutStarting
    }

    #   Check to see if an event file already exists for a
    #   specific run.
    # Parameters:
    #     run   - Number of the run to check.
    # Returns:
    #     1     - the file exists.
    #     0     - the file does not exist.
    #
    proc RunFileExists {run} {
	set dir [ExpFileSystem::WhereareCompleteEventFiles]
	set name [ExpFileSystem::GenRunFile $run]
	set fullname $dir/$name

	return [file exists $fullname]
	
    }

    #  Define exports:

    namespace export Register 
    namespace export SetFtpLogInfo
    namespace export SetSourceHost
    namespace export BiggestRun
    namespace export EventFilePath
    namespace export EmergencyEnd
    namespace export CleanOrphans
    namespace export RunFileExists
}