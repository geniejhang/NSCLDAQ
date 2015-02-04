#
#  Procedure to save current operational parameters to a file.
#

proc SaveParams {file} {
    global ConfigDir
    global paramlist
    set fd [open $ConfigDir/$file w]

    set date [exec date]
    puts $fd "# Operational parameter file saved $date"

    foreach param $paramlist {
	global $param
	set value [eval set $param]
	puts $fd "set $param \"$value\""
    }
    puts $fd
    puts $fd "# List of operational parameters"
    puts $fd
    puts $fd "set paramlist {$paramlist}"
    close $fd
    
}

#
# Procedure to save current operational parameters to setup file.
#
proc SaveStartupParams {} {
    SaveParams opparams.tcl
}

#
# Procedure to save current operationsl parameers to a temp file.
#
proc SaveTempParams {} {
 
    SaveParams tempparams.tcl
}