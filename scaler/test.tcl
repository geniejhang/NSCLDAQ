set dataurl ""

set dir [file dirname [info script]]

puts "Source $dir/scaler.tcl"

source $dir/scaler.tcl
source ../config/hardware.tcl

page All "All of the scalers"

display_ratio All see.ppac.u see.ppac.d
display_ratio All see.ppac.l see.ppac.r
display_single All see.scint



set clientpid [exec sclclient $dataurl & ]

bind . <Destroy> "exec kill  $clientpid"

proc UserUpdate {} {
  puts Update
}
proc UserBegin {} {
  puts Begin
}
proc UserEnd {} {
  puts End
}