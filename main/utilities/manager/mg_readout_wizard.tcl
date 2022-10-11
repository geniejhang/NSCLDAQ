#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#             Ron Fox
#             Giordano Cerriza
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


##
# @file mg_readout_wizard.tcl
# @brief Wizard to create readout programs.
# @author Ron Fox <fox@nscl.msu.edu>
#

if {[array names env DAQTCLLIBS] ne ""} {
    lappend auto_path $env(DAQTCLLIBS)
}
package require Tk
package require snit

package require programs
package require sequence
package require containers

##
# Provides a wizard that creates all of the stuff to make a readout program work.
# This is not only the Readout program itself but the REST clients that
# are needed to start it.  Each Readout program named xxx
# will create the following programs.
#
#  -  xxx the readout itself.
#  -  xxx_init - initializes the hardware.
#  -  xxx_setrun - sets the run number from the KV store.
#  -  xxx_settitle - sets the title from the KV Store.
#  -  xxx_beginrun - Starts the run.
#  -  xxx_endrun   - Ends the run.
#  -  xxx_shutdown - Shuts the readout down.
#
#  If the following sequences don't exist they are created
#
#   -  bootreadout - triggered by the BOOT transition and xxx is added to that sequence.
#   -  initreadout - triggered by the  HWINIT transition and xxx_init is added to that sequence.
#   -  beginreadout- triggered by the BEGIN transition, xxx_setrun, xxx_settitle
#                     and xxx_beginrun are added to that sequence.
#   -  endreadout  - Triggered by the END transtion, xxx_endrun is added to it.
#   -  shutdownreadout - Triggered by SHUTDOWN< xxx_shutdown is added to it.
#
#  For each Readout we need the following:
#   *  Type of Readout - XIA/DDAS-12+, VMUSB, CCUSB, Customized
#   *  Container - can be none or any of the defined containers.
#   *  Host
#   *  Working directory.
#   *  IF customized, the location of the Readout program.
#   *  Source id  - for the event builder.
#   *  ring   - Name of ring buffer.
#   *  Rest service name (defaults to ReadoutREST)
#   IF XXUSBReadout:
#    ctlconfig, daqconfig
#   IF XIADDAS-12+:
#     sort host, sort ring, sort window, fifo threshold, buffersize, infinity clock,
#     clock multiplier, scaler readout period.
#   If Customized
#     Any options the user wants to add.
#
#   Environment variables the program will have... in addition to those set up by
#   the container start script.
#
#  Note that prior to DDAS readout prior to V12 should use customized and select
#  $DAQBIN/DDASReadout configuring it manually using environment variables etc.
#


 #------------------------------------------------------------------------------
 #   GUIs.
 #     The Main GUI has several components:
 
 ##
 #  Form to allow the input of common attributes;
 #
 # *   - -type -  one of: {ddas, vmusb, ccusb, custom}
 # *   - -container  If not an empty string the container that will run the readout.
 # *   - -host    Host in which the Readout will run.
 # *   - -directory - working directory in which the Readout will run.
 # *   - -sourceid - Event builder source id.
 # *   - -ring  - output ring buffer.
 # *   - -service - REST service name used to control the Readout.
 #
 #  Other OPTIONS:
 #    -containers - containers to select between.
 #    -typeselectcommand - script when the radio buttons holding the Readout Type
 #        change.
 #
 snit::widgetadaptor rdo::CommonAttributes {
    option -type
    option -containers  -default [list] -configuremethod _configContainers;       # Containers to chose between
    option -container;        # Selected container.
    option -host
    option -directory
    option -sourceid -default 0
    option -ring -default $::tcl_platform(user)
    option -service -default ReadoutREST
    option -typeselectcommand -default [list]
    
    variable daqtypes [list XIA VMUSB CCUSB Custom]
    
    constructor args {
        installhull using ttk::frame
        
        #  The Readout types
        
        ttk::labelframe $win.types -text {Readout Type}
        set radios [list]
        foreach type $daqtypes {
            lappend radios [ttk::radiobutton $win.types.[string tolower $type] \
                -variable [myvar options(-type)] -value $type -text $type \
                -command [mymethod _dispatchType]]
        }
        set options(-type) [lindex $daqtypes 0]
        grid {*}$radios
        
        #  Container and host:
        
        ttk::labelframe $win.container -text {Container}
        ttk::combobox $win.container.container \
            -values [list] \
            -textvariable [myvar options(-container)]
        ttk::entry $win.container.host -textvariable [myvar options(-host)]
        ttk::label $win.container.hostlabel -text Host
        grid $win.container.container  $win.container.host $win.container.hostlabel
        
        # directory, sourceid
        
        ttk::labelframe $win.dir -text {Directory}
        ttk::entry $win.dir.dir -textvariable [myvar options(-directory)]
        ttk::button $win.dir.browse -text {Browse...}
        grid $win.dir.dir $win.dir.browse -sticky ew
    
        ttk::labelframe $win.sid -text {Source Id}
        ttk::spinbox $win.sid.sid \
            -from 0 -to 100000 -increment 1 \
            -textvariable [myvar options(-sourceid)]
        ttk::label $win.sid.label -text {Source Id}
        grid $win.sid.sid $win.sid.label -sticky ew
        
        grid $win.types $win.container -sticky ew
        grid $win.dir $win.sid -sticky ew
         
        $self configurelist $args 
        
    }
    #--------------------------------------------------------------------------
    #
    # Configuration methods
    
    
    ##
    # _configContainers
    #    Configure the containers in the $win.container.container combobox.
    #
    # @param name - name of configuration option
    # @param value - new value
    #
    method _configContainers {name value} {
        set options($name) $value
        $win.container.container configure -value $value
    }
    #-------------------------------------------------------------------------
    #  Event handling
    
    
    ##
    # _dispatchType
    #   Called when the type changes.
    #
    method _dispatchType {} {
        puts "Dispatch type $options(-type)"
    }
 }
 

#
