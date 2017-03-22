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
#             Jeromy Tompkins 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


##
# @file propertyviews.tcl
# @brief Provide editing views for properties (type safe).
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide PropertyViews 1.0
package require Tk
package require snit
package require properties

##
# @class PropertyView
#    Provide a generic property view for generic property.
#    This consists of:
#    -  A label with the property's name.
#    -  An entry into which property values can be entered... if the
#       property is not readonly.
#
# OPTIONS:
#   - -name - Property name.
#   - -value - property value.
#   - -readonly - true if the user can't change the property value.
# 
snit::widgetadaptor PropertyView {
    
    option -name     -default "" -readonly 1
    option -value    -default "" 
    option -readonly -default 0  -readonly 1
    
    ##
    # constructor:
    #   Install a ttk::frame as our hull.
    #   Process configuration options.
    #   Create the widgets 
    #   If -readonly, disable the entry.
    #   Layout the widgets.
    #
    # @param[in] args - the construction time configuration options.
    # 
    constructor args {
        installhull using ttk::frame
        
        $self configurelist $args
        
        ttk::label $win.l -textvariable [myvar options(-name)]
        ttk::entry $win.e -textvariable [myvar options(-value)]
        
        if {$options(-readonly)} {
            $win.e configure -state disabled
        }
        
        grid $win.l $win.e -sticky nsew
        grid columnconfigure $win 1 -weight 1
        grid columnconfigure $win 0 -weight 0
        grid rowconfigure $win 0 -weight 1
    }
    
}