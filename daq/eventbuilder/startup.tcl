lappend auto_path /usr/opt/daq/10.2/TclLibs

package require EventBuilder
package require EVB::connectionList
package require EVB::GUI
EVB::Start


EVB::createGui .test
pack .test

EVB::maintainGUI .test

