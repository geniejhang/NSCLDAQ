#
#   This script defines the hardware used by the SEE readout system.
#   

#  Setup the V785 (ADC)  in crate 0 slot 5.  Enable only the first 4 channels.
#  Users will probably want to change the names of the parameters and
#  values of the threshold.


# module v785  caenv785 slot 11 geo true
# v785 config enable {1 1 1 1 1 0 0 0  \
# 	            0 0 0 0 0 0 0 0  \
# 		    0 0 0 0 0 0 0 0  \
# 		    0 0 0 0 0 0 0 0 } ;# only first 4 channels enabled.
# v785 config threshold {0  0  0  0  0  0  0  0      \
# 	               0  0  0  0  0  0  0  0      \
# 		       0  0  0  0  0  0  0  0      \
# 		       0  0  0  0  0  0  0  0 } ;# Set meaningful threshold.
# v785 config parameters {see.ppac.u see.ppac.d see.ppac.l see.ppac.r \
# 	                see.ppac.a "" "" "" \
# 	                "" "" "" "" "" "" "" "" \
# 			"" "" "" "" "" "" "" "" \
# 			"" "" "" "" "" "" "" "" } ;# Make these meaningful!!


adc create caenv785 [expr 11 << 24]
adc config caenv785 -geo 11 -suppressrange on
adc config caenv785 -thresholds [list 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
				    0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]

set adcParameters(caenv785) [list see.ppac.u see.ppac.d \
				 see.ppac.l see.ppac.r see.ppac.a]




#
#  Set up the V792 (QDC) in virtual  slot 12 
#  Note that this is actually base addressed as if it were in slot 7,
#  but we know that addresss is a free one in the vme space so leave it
#  alone.
#
# module v792 caenv792 slot 12 geo false base [expr 7 << 24]
# v792 config enable  {1 0 1 0 1 0 1 0  0 0 0 0 0 0 0 0 \
#      	             0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0 } ;# One channel only.
# v792 config threshold  {0 0 0 0 0 0 0 0  \
#                         0 0 0 0 0 0 0 0  \
#                         0 0 0 0 0 0 0 0 \
#                         0 0 0 0 0 0 0 0  } ;# Set meaningful threshold.
# v792 config Iped 128
# v792 config parameters {see.sci.u "" see.sci.d ""  see.sci.l "" see.sci.r "" \
# 	                 \
# 	                "" "" "" "" "" "" "" "" \
# 			"" "" "" "" "" "" "" "" \
# 			"" "" "" "" "" "" "" ""  } ;# Set meaningful paramnames


adc create caenv792 [expr 7 << 24]
adc config caenv792 -geo 12 -iped 128
adc config caenv792 -thresholds [list 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
				     0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]
set adcParameters(caenv792) [list see.sci.u "" see.sci.d "" see.sci.l "" see.sci.r]


## Define the event readout...first the 785 thne the 792:

stack create event
stack config event -triger nim1 -delay 10 -modules [list caenv785 caenv792]



#------------------------------------------------------------------
# Configure the scaler readout:

# scaler v830 caenv830 slot 3 geo false base 0x80000000
# v830 config enables {1 1 0 0  0 0 0 0 \
# 	             0 0 0 0  0 0 0 0 \
# 		     1 1 1 1  1 0 0 0 \
# 		     1 1 1 1  0 0 0 0 }
# v830 config trigger 1 wide true header false autoreset false
# v830 config autoreset false manualclear true packetize false
# v830 config vmetrigger true
# v830 config parameters {master      master.live "" ""       \
# 	                ""  "" ""  ""
# 	                "" "" "" ""   "" "" "" ""           \
# 			see.ppac.u see.ppac.d see.ppac.l see.ppac.r \
# 			see.ppac.a    "" "" "" \
# 			see.scint.u see.scint.d see.scint.l see.scint.r \
#                            "" "" "" "" }


v830 create caenv830 0x80000000
v830 config caenv830 -channels 0xc000f8f0 -header off 0 -trigger vme 
v830 config caenv830 -autoreset on

stack create scaler
stack config scaler -trigger scaler -period 2 -delay 1 -modules [list caenv830]


################ Do not edit below this line ###############################
#
#   The code below depends on whether or not this script is sourced into
#   SpecTcl or the readout.  SpecTcl provides the variable 
#   SpecTclHome which the readout does not so we can distinguish between the
#   two.


#  In Readout, we want to load the cfd and shaper.
#  HV is always done by hand since it can destroy the PPAC among other things
#  if done automatically.


if {[info var SpecTclHome] == ""} {


    puts "--------------- Initializing CFD  ---------"

   
    catch "exec $cfddir/loadcfd.tcl $configdir/seecfd.cfd      \
	    $configdir/seecfd_default.cfd_settings " msg
    
    puts $msg

    puts "----------------- Initializing shaper: -----"

    catch "exec $shaperdir/loadshaper.tcl $configdir/shaper.cfg   \
	             $configdir/shaper_defaults.shaper_values " msg

    puts $msg


}

