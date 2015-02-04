#!/bin/sh
#
#   Script to burn the account to a DVD. 
#   - Create an ISO in /scratch
#   - Check the size of the iso:
#       0 < size < 600Mbytes - prompt for a CD or DVD
#       600 < size < 1Gbyte  - prompt for a DVD.
#       1Gbyte < size        - Fail.. need assistance to split the
#                              data across multiple DVD's.
# \
    exec tclsh ${0} ${@}

set isopath /scratch/see.iso;		# where the iso gets built.
set temppath /scratch/see
set cddevice "0,0,0";			# May need to be changed with sys.

cd ~
set tilde [pwd]

proc MakeIsoImage {} {
    global isopath
    global tilde
    global temppath
    puts "Generating temp image of the directory:... this may take a while."
    exec mkdir -p $temppath
    catch "exec tar  czhfC - $tilde $tilde --exclude stagearea | \
	tar xzfvC - $temppath >& /dev/tty"
    puts "Building the ISO image for the disk in $isopath"
    catch "exec mkisofs  -f  -J -r -o $isopath $temppath  >& /dev/tty "
}

proc PromptForMedia {} {
    global isopath
    set Mbyte [expr 1024*1024]
    set Gbyte [expr 1024*$Mbyte]

    set size [file size $isopath]
    if {$size < [expr 600*$Mbyte]} {
	puts "Please put either a CD or a DVD in the burner drive"
    } elseif {$size < [expr 4*$Gbyte]} {
	puts "Too much data for a CD, however it will fit on a DVD"
	puts "Please put a DVD in the burner drive"
    } else {
	puts "Too much data for either a CD or DVD... please consult with"
	puts "NSCL Staff about busting this up into pieces and burning it"
	puts "a bit at a time."
	exit
    }
    puts "When you have put the blank media requested in the burner drive"
    puts -nonewline "Strike the enter key to start the burn"
    flush stdout
    gets  stdin
}

proc Burn {} {
    global isopath
    global cddevice 

    puts "Burning the disk"
    catch "exec cdrecord -v dev=$cddevice  $isopath >& /dev/tty "
}


MakeIsoImage
PromptForMedia
Burn
puts "Removing temporary image of the directory..."
exec rm -rf $temppath $isopath

puts "Done!"


