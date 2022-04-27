package provide InstallRoot 1.0
namespace eval InstallRoot {
variable root /usr/opt/daq/11.4-028
proc Where {} {
variable root
return $root
}
}
