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
    destructor {
        $prop destroy
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

##
# @class IntegerPropertyView
#
#   Provides a flexible editor for integers.
#
# OPTIONS:
#   - -usespinbox - if true the editor will be a spinbox, else a validated
#                   entry is used.  This is readonly for now.
#   - -from       - if usespinbox the -from option of the spinbox.
#   - -to         - if usespinbox the -to option of the spinbox.
#   - -increment  - if usespinbox true, the -increment option for that spinbox.
#   - -readonly   - the value can't be changed if true.
#
# @note in fact both an entry and a spinbox are created.  The difference
#       is which one is actually gridded.
#
snit::widgetadaptor  IntegerPropertyView {
    component spinbox
    component entry
    
    variable editor
    
    option -name       -default "" -readonly 1
    option -value      -default "" -configuremethod _setValue               \
        -cgetmethod _getValue
    option -readonly   -default 0  -readonly 1
    option -usespinbox -default 0  -readonly 1
    
    delegate option -from      to spinbox
    delegate option -to        to spinbox
    delegate option -increment to spinbox
    
    ##
    # constructor:
    #   Install a ttk::frame as the hull.
    #   create the lable and install the entry/spinbox.
    #   process configuration.
    #   select the actual editor widget.
    #   grid the user interface.
    #
    # @param[in] args - the construction time configuratino option/pairs.
    #
    constructor args {
        installhull using ttk::frame
        
        label $win.l -textvariable [myvar options(-name)]
        install spinbox using ttk::spinbox $win.spinbox
        install entry   using ttk::entry   $win.entry
        
        $self configurelist $args
        
        if {$options(-usespinbox)} {
            set editor $spinbox
        } else {
            set editor $entry
        }
        
        grid $win.l $editor -sticky nsew
        grid columnconfigure $win 1 -weight 1
        grid columnconfigure $win 0 -weight 0
        grid rowconfigure $win 0 -weight 1
        
        if {$options(-readonly) } {
            $editor configure -state disabled
        }
        
    }
    #--------------------------------------------------------------------------
    # Configuration methods:
    
    ##
    # _setValue
    #   Set a new value - both boxes will have their value set.
    #   note that to do this we need to first ensure their state is normal
    #   The assumption is that the value is legal.
    #
    # @param[in] optname - name of the option to modify.
    # @param[in] value   - new value proposed.
    #
    method _setValue {optname value} {
        
        # Update the spinbox.
        
        set s [$spinbox cget -state]
        $spinbox configure -state normal
        $spinbox set $value
        $spinbox configure -state $s
        
        # Update the entry.
        
        set s [$entry cget -state]
        $entry delete 0 end
        $entry insert end $value
        $entry configure -state $s
        
        set options($optname) $value
    }
    ##
    # _getValue
    #   Get the editor value.  Both spinbox and entry support a get method
    #   so this is pretty simple.
    #
    # @param[in] optname - name of the option being retrieved.
    #
    method _getValue optname {
        return [$editor get]
    }
}
##
# @class IntegerEditor
#   Wrap a property in the standard factory methods for producing
#   an editor appropriate to it:
#
# OPTIONS:
#   *   -usespinbox  - If true a spinbox will be created rather than an entry.
#   *   -from        - -from value for the spinbox.
#   *   -to          - -to   value for the spinbox.
#   *   -increment   - -increment value for spinbox
#
# METHODS:
#   *  makeEditor  - Create an editor widget.
#   *  saveValues  - Save an editor widget's values into a property.
#
snit::type IntegerEditor {
    component prop
    
    option -usespinbox -default 0
    option -from       -default 0
    option -to         -default 10
    option -increment  -default 1
    
    delegate option * to prop
    delegate method * to prop
    
    ##
    # constructor
    #   Construct the underlying property
    #   configure ourself.
    #
    # @param[in] args - construction time configuration option/values.
    #
    constructor args {
        install prop using property %AUTO% -name temp
        $self configurelist $args;             # Should override temp name.
    }
    destructor {
        $prop destroy
    }
    #--------------------------------------------------------------------------
    # Factory methods:
    #
    
    ##
    # makeEditor - create an editor widget.
    #
    # @param[in] path - widget path to create.
    # @return path
    #
    method makeEditor path {
        IntegerPropertyView $path -name [$prop cget -name] -value             \
            [$prop cget -value] -readonly [expr {![$prop cget -editable]}]     \
            -usespinbox $options(-usespinbox) -from $options(-from)           \
            -to $options(-to) -increment $options(-increment)
        
        
        return $path
    }
    ##
    # saveValues
    #   Save the editor value in the property.
    #
    # @param[in] path - editor widget path.
    #
    method saveValues path {
        $prop configure -value [$path cget -value]
    }
}