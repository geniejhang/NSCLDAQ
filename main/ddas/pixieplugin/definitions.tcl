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
# @file   definitions.tcl
# @brief  Provide an autogen generated file that locates our toys.
# @author Ron Fox <fox@nscl.msu.edu>
#

######################################################################
#  WARNING TO MAINTAINERS:
#
# DO NOT EDIT definitions.tcl  it is generated from definitions.in
#             if edits are required you must edit that file instead
#             and run configure again to generate definitions.tcl
#

# Pull definitions of our root directory and the DDAS root directory
# associated with this NSCLDAQ build in using autoconf:
#

set DDAS_INSTDIR 
set DAQ_INSTDIR /usr/opt/daq/11.4-029
