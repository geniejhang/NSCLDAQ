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
##
# @class GenericPropertyEditor
#
#  Wraps a generic property (string property) with a factory for an appropriate
#  editor.   In this case, the editor produces a PropertyView which
#  allows an arbitray string to be entered.
#
# METHODS:
#   *  makeEditor - creates an editor.
#   *  saveValues - Saves value(s) from the editor.
#
snit::type GenericPropertyEditor {
    component prop
    
    # These make us a simple wrapper around the property we install:
    
    delegate option * to prop
    delegate method * to prop
    
    ##
    # constructor
    #   - Install the property with a temporary name
    #   - Process configuration options (hopefully updates the temp name).
    #
    # @param[in] args - construction time configuration options.
    #
    constructor args {
        install prop using property %AUTO% -name temp
        $self configurelist $args
    }
    #-------------------------------------------------------------------------
    # Factory methods:
    
    ##
    # makeEditor
    #   Given a widget path creates an editor widget with that name
    #   The widget is stocked with the current name/vaule and its -readonly
    #   value determines if it's readable:
    #
    # @param[in] path - widget path.
    # @return    path - just like all widget making things
    method makeEditor path {
        PropertyView $path                                                  \
            -name [$prop cget -name] -value [$prop cget -value]               \
            -readonly [expr {![$prop cget -editable]}]
        return $path
    }
    ##
    # saveValues
    #   Stores the values from a view into the property.
    #
    # @param[in] path - widget path containing the view.
    #
    method saveValues path {
        $self configure -value [$path cget -value]
    }
}