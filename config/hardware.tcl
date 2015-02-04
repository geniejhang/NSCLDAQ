#
#   This script defines the hardware used by the SEE readout system.
#   

#  Setup the V785 (ADC)  in crate 0 slot 5.  Enable only the first 4 channels.
#  Users will probably want to change the names of the parameters and
#  values of the threshold.

module v785  caenv785 slot 11 geo true
v785 config enable {1 1 1 1 1 0 0 0  \
	            0 0 0 0 0 0 0 0  \
		    0 0 0 0 0 0 0 0  \
		    0 0 0 0 0 0 0 0 } ;# only first 4 channels enabled.
v785 config threshold {0  0  0  0  0  0  0  0      \
	               0  0  0  0  0  0  0  0      \
		       0  0  0  0  0  0  0  0      \
		       0  0  0  0  0  0  0  0 } ;# Set meaningful threshold.
v785 config parameters {see.ppac.u see.ppac.d see.ppac.l see.ppac.r \
	                see.ppac.a "" "" "" \
	                "" "" "" "" "" "" "" "" \
			"" "" "" "" "" "" "" "" \
			"" "" "" "" "" "" "" "" } ;# Make these meaningful!!


#
#  Set up the V792 (QDC) in virtual  slot 12 
#  Note that this is actually base addressed as if it were in slot 7,
#  but we know that addresss is a free one in the vme space so leave it
#  alone.
#
module v792 caenv792 slot 12 geo false base [expr 7 << 24]
v792 config enable  {1 0 1 0 1 0 1 0  0 0 0 0 0 0 0 0 \
     	             0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0 } ;# One channel only.
v792 config threshold  {0 0 0 0 0 0 0 0  \
                        0 0 0 0 0 0 0 0  \
                        0 0 0 0 0 0 0 0 \
                        0 0 0 0 0 0 0 0  } ;# Set meaningful threshold.
v792 config Iped 128
v792 config parameters {see.sci.u "" see.sci.d ""  see.sci.l "" see.sci.r "" \
	                 \
	                "" "" "" "" "" "" "" "" \
			"" "" "" "" "" "" "" "" \
			"" "" "" "" "" "" "" ""  } ;# Set meaningful paramnames



scaler v830 caenv830 slot 3 geo false base 0x80000000
v830 config enables {1 1 0 0  0 0 0 0 \
	             0 0 0 0  0 0 0 0 \
		     1 1 1 1  1 0 0 0 \
		     1 1 1 1  0 0 0 0 }
v830 config trigger 1 wide true header false autoreset false
v830 config autoreset false manualclear true packetize false
v830 config vmetrigger true
v830 config parameters {master      master.live "" ""       \
	                ""  "" ""  ""
	                "" "" "" ""   "" "" "" ""           \
			see.ppac.u see.ppac.d see.ppac.l see.ppac.r \
			see.ppac.a    "" "" "" \
			see.scint.u see.scint.d see.scint.l see.scint.r \
                           "" "" "" "" }

################ Do not edit below this line ###############################
#
#   The code below depends on whether or not this script is sourced into
#   SpecTcl or the readout.  SpecTcl provides the variable 
#   SpecTclHome which the readout does not so we can distinguish between the
#   two.

if {[info var Readout] != ""} {
    #  This is readout.. Assume all modules get read out in the order
    #  they appear in module -list

    set modules [module -list]
    foreach module $modules {
	set mod [lindex $module 0]
	readout add $mod
    }
    puts "----- Event readout configuation:  ------"
    puts [readout list]
    foreach module $modules {
	set mod [lindex $module 0]
	puts [$mod cget]

    }
    # Assume all scaler modules also get read out in the order
    # they appear in the module list.

    set scalers [scaler -list]
    foreach scaler $scalers {
	set name [lindex $scaler 0]
	scalerbank add $name
    }
    puts "------ Scaler readout configuration: -----"
    puts [scalerbank list]
    foreach scaler $scalers {
	set name [lindex $scaler 0]
	puts [$name cget]
    }


    set configdir $env(CONFIGDIR)
    set distdir   $env(DISTRIBROOT)
    set cfddir    $distdir/controls/cfd
    set shaperdir $distdir/controls/shaper


    puts "--------------- Initializing CFD  ---------"

   
    catch "exec $cfddir/loadcfd.tcl $configdir/seecfd.cfd      \
	    $configdir/seecfd_default.cfd_settings " msg
    
    puts $msg

    puts "----------------- Initializing shaper: -----"

    catch "exec $shaperdir/loadshaper.tcl $configdir/shaper.cfg   \
	             $configdir/shaper_defaults.shaper_values " msg

    puts $msg


}
if {[info var SpecTclHome] != ""}  {
    # This is SpecTcl
    # We need to create parameters for each of the module parameters.
    # We also need to add each module to the unpacker.
    # We get the parameters from the module configurations.
    # All parameters are assumed to be 12 bits wide.
    
    set paramno 0
    set modules [module -list]
    foreach module $modules  {
	set name [lindex $module 0]
	set configlist [$name cget]
	foreach item $configlist {
	    if {[lindex $item 0] == "parameters"} {
		set paramlist [lindex $item 1]
		foreach parameter $paramlist {
		    if {$parameter != ""} {
			parameter $parameter $paramno 12
			incr paramno
		    }
		}
	    }
	}
	unpack add $name
    }
    # Parameter that are calculated.

    parameter see.ppac.x   $paramno 12; incr paramno
    parameter see.ppac.y   $paramno 12; incr paramno

    parameter see.ppac.x_profile  $paramno 12; incr paramno
    parameter see.ppac.y_profile  $paramno 12; incr paramno

    # And this one to make the definition of the 'scaler' spectrum easy.

    parameter see.sci.counts $paramno 12; incr paramno ;# If I need more.

    #  Figure out the mapping of scalers to channels. This gets put into
    #  scaler_channels(name).
    #
    set index 0                
    set scalers [scaler -list ]

    foreach scaler $scalers {
	set name [lindex $scaler 0]    
	set config [$name cget]
	foreach item $config {	       
	   set configname [lindex $item 0]
    	   if {$configname == "parameters"} {
	       set channels [lindex $item 1]  
	       foreach channel $channels {
		   if {$channel != ""} {
		       set scaler_channels($channel) $index
		       incr index
		   }
	       }
	   }
       }
   }    
    
}
if {[info var Scaler] != ""}  {
    puts "configuring for scaler"
    # 
    # This is scaler display
    # Use the scaler module listing to produce the buffer offsets
    # for the scalers.  We assume that there are names for all enabled
    #  channels.
    #  The modules are listed and the scalers are defined for each module
    #  so that they can be correctly configured into pages:
    #  the assumption is that only non-blank channels are read (the others
    #  are disabled, and supressed).  If this is incorrect, configure modules
    #  with dummy names.
    #   We use that scalers -list returns a list of {modulename type} pairs.
    #   that modulename cget returns a list of 
    #     {configparametername value} pairs and that
    #  when the configparametername is "channels", the value is a list of the
    #  defined channels.

    set index 0                
    set scalers [scaler -list ]

    foreach scaler $scalers {
	set name [lindex $scaler 0]    
	set config [$name cget]
	foreach item $config {	       
	   set configname [lindex $item 0]
    	   if {$configname == "parameters"} {
	       set channels [lindex $item 1]  
	       foreach channel $channels {
		   if {$channel != ""} {
		       channel $channel $index
		       incr index
		   }
	       }
	   }
       }

   }
}
