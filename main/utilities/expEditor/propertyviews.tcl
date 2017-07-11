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
    
    option -value    -default "" 
    option -readonly -default 0  -readonly 1
    
    delegate method * to hull
    delegate option * to hull
    
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
        installhull using ttk::entry  -textvariable [myvar options(-value)]
        
        $self configurelist $args
        
        
        if {$options(-readonly)} {
            $hull configure -state disabled
        }
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
            -readonly [expr {![$prop cget -editable]}]                      \
            -value    [$prop cget -value]
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
        
        install spinbox using ttk::spinbox $win.spinbox  -width 5
        install entry   using ttk::entry   $win.entry    -width 5
        
        $self configurelist $args
        
        if {$options(-usespinbox)} {
            set editor $spinbox
        } else {
            set editor $entry
        }
        
        grid $editor -sticky w
        
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
        install prop using IntegerProperty %AUTO% -name temp
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

        IntegerPropertyView $path                                              \
            -value [$prop cget -value] -readonly [expr {![$prop cget -editable]}]     \
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
##
# @class EnumeratedView
#   Provides an editor for an enumerated type.  This will be a combo box
#   where the initial value is loaded into the box and the values are the
#   drop down.
#
# OPTIONS
#   *  -readonly - if true the item is readonly. (can only be configured at construction).
#   *  -name     - Name of the item.
#   *  -value    - Value of the itemm.
#   *  -values   - Legal values of the item.
#
# @note it is not an error to configure -value with an item not in -values
#       though that may cause an error when the editor is saved back to the
#       property.
#
snit::widgetadaptor EnumeratedView {
    
    option -readonly -default 0 -readonly 1
    option -value -default ""
    
    
    delegate method * to hull
    delegate option * to hull
    
    ##
    # constructor
    #   - hull is installed as a ttk::frame.
    #   - label is made
    #   - chooser is installed.
    #   - options processed.
    #   - widgets laid out
    #   - if -readonly the chooser is disabled.
    #
    # @parameter[in] args - the constrution time option/value pairs.
    #
    constructor args {
        installhull using  ttk::combobox -textvariable [myvar options(-value)] \
            -width 10
        
        $self configurelist $args
        
        
        if {$options(-readonly)} {
            $chooser configure -state readonly
        }
          
    }
}
##
# @class EnumeratedEditor
#    Wraps an EnumeratedProperty supplying factory methods for producing
#    an editor (view) and receiving the data from that view if it is to be
#    saved.
#
snit::type EnumeratedEditor {
    component prop
    delegate option * to prop
    delegate method * to prop
    
    ##
    # constructor:
    #   install the property as an EnumeratedProperty with a temp name
    #   configure the property - which should set the name.
    #
    # @param[in] args - construction time configuration option/value pairs.
    #
    constructor args {
        install prop using EnumeratedProperty %AUTO% {*}$args;   # All opts delegated.
        
    }
    #--------------------------------------------------------------------------
    # Factory support methods:
    
    ##
    # makeEditor
    #   Returns a view for the object.
    #
    # @param[in] path - the widget path for the object to be created.
    # @return path.
    #
    method makeEditor path {

        return [EnumeratedView $path                                         \
            -values [$prop cget -values] -value [$prop cget -value]          \
            -readonly [expr {![$prop cget -editable]}]                       \
        ]   
    }
    ##
    # saveValues
    #   Called when it's time to savae an editor's values to the underlying
    #   property.
    #
    # @param[in] path - widget path of the editor.
    #
    method saveValues path {
        $prop configure -value [$path cget -value]
    }
}
##
# @class ListView
#     Provides an editing widget for a property that consists of a list of values.
#     this is a label for the property name.  An entry which can be used
#     to add values to the list.  A button which can be used to move the entry
#     value into the list (<Return> will as well).
#     A list box that shows the list (<Double-1> removes the item unter the mouse
#     from that list).
#
#
# OPTIONS:
#    *  -name  - Name of the property (label value).
#    *  -value - List of current values.
#    *  -maxlen - Largest number of values allowed in the list if adding a value
#                would exceed this, bell is rung. If this is an empty string
#                there is o practical limit. (only configurable at construction
#                time).
#    * -readonly - Disables list manipulation if enabled.
#
snit::widgetadaptor ListView {
    component list
    component entry
    
    option -name  -default ""
    option -value -default [list]
    option -maxlen -default "" -readonly 1
    option -readonly -default 0 -readonly 1
    
    typevariable rightarrow
    
    ##
    # create the right arrow bitmap.
    
    typeconstructor {
        set rightarrow [image create bitmap -data {
            #define right_width 11
            #define right_height 11
            static char right_bits = {
            0x00, 0x00, 0x20, 0x00, 0x60, 0x00, 0xe0, 0x00, 0xfc, 0x01, 0xfc,
            0x03, 0xfc, 0x01, 0xe0, 0x00, 0x60, 0x00, 0x20, 0x00, 0x00, 0x00
            }
        }]
        
    }
    
    ##
    # constructor
    #    install a ttk::frame as the hull.
    #    Create the label, entry and list box.
    #    Process the configuration options.
    #    Bind event handlers appropriately.
    #
    # @param[in] args - Construction time configuration option/value pairs.
    #
    constructor args {
        installhull using ttk::frame
        
        install entry using ttk::entry $win.entry -width 15
        
        ttk::button $win.b -image $rightarrow -command [mymethod _addToList]
        
        install list using listbox $win.list                               \
            -listvariable [myvar options(-value)]                          \
            -yscrollcommand [list $win.yscroll set] -selectmode single     \
            -height 5
        ttk::scrollbar $win.yscroll -command [list $list yview]
        
        
        $self configurelist $args
        
        # Lay this all out
        
        grid $entry  -row 0 -column 0 -sticky ew
        grid $win.b  -row 0 -column 1
        grid $list -row 0 -column 2 -sticky nsew
        grid $win.yscroll -row 0 -column  3 -sticky nsw
        
        # Let the entry expand in x and the list expand in y.
        
        grid columnconfigure $win 0 -weight 1
        grid columnconfigure $win 2 -weight 1
        foreach col [list  1 3] {
            grid columnconfigure $win $col -weight 0
        }
        #  If readonly we don't allow any modification.
        
        if {$options(-readonly) } {
            $entry configure -state disable
            $list configure -state disable
            $win.b configure -state disable
        } else {
            # If enabled we also need to enable the event bindings:
            
            bind $list <Double-1> [mymethod _removeFromList %x %y]
            bind $entry <Return> [mymethod _addToList]
        }
    }
    #--------------------------------------------------------------------------
    # Event handling.
    #
    
    ##
    # _addToList
    #    Add the entry contents (if not empty) to the end of the list of values.
    #
    method _addToList {} {

        set value [$entry get]
        if {$value ne ""} {
            
            # If this is too many entries bell.
            
            set entryCount [llength $options(-value)]
            incr entryCount
            if {($options(-maxlen) ne "") && ($entryCount > $options(-maxlen))} {
                bell
                bell
                bell
                return
            }
            
            $list insert end $value
            $list yview end
            $entry delete 0 end;       # Clear the entry.
            focus -force $entry;             # Put the cursor in the entry again.
        }
    }
    ##
    # _removeFromList
    #   Removes the entry under the cursor from the list.
    #
    # @param[in] x,y - mouse coordinates at event time relative to listbox origin.
    method _removeFromList {x y} {
        set index [$list index @$x,$y]
        $list delete $index
        
    }
}
##
# @class ListEditor
#    Wrap a list valued property in a factory for editors of that
#    property.
#
#
snit::type ListEditor {
    component prop
    
    delegate option * to prop
    delegate method * to prop
    
    ##
    # constructor
    #    Just wrap the property and configure it:
    #
    # @param[in] args
    #
    constructor args {
        install prop using ListProperty %AUTO% {*}$args
    }
    #--------------------------------------------------------------------------
    #  Factory support methods.
    #
    
    ##
    # makeEditor
    #    Create a new editor.
    #
    # @param[in]  path - widget path to assign to the editor.
    # @return path
    #
    method makeEditor path {
        ListView $path  -value [$prop cget -value] \
            -maxlen [$prop cget -maxlen] -readonly [expr {![$prop cget -editable]}]
        
        return $path
    }
    ##
    # saveValues
    #   Save the values from an editor into the property.
    #
    # @param[in] path - widget path of the editor widget.
    #
    method saveValues path {
        $prop configure -value [$path cget -value]
    }
}

##
# @class FilePropertyView
#     Provides a view for a file property.    This is an entry widget
#     with a button that's labeled "..."  clicking that button brings up
#     a tk_getOpenFile dialog which will modify the value in the
#     entry.
#      -readonly both disables the entry and the buttton.
#
snit::widgetadaptor FilePropertyView {
    component fileEntry
    
    option -name  -default ""
    option -value -default ""
    option -readonly -default 0 -readonly 1
    
    option -defaultdir -default $::env(DAQBIN)
    option -filetypes  -default [list [list "All Files" *]]
    
    ##
    # constructor
    #    Creates and lays out the entry and the button:
    #
    constructor args  {
        installhull using ttk::frame
        
        install fileEntry using ttk::entry $win.entry -width 35 -textvariable [myvar options(-value)]
        button  $win.browse -text "..." -command [mymethod _browseForFile]
        
        grid $fileEntry $win.browse -sticky nsw
        grid columnconfigure $win 0 -weight 1
        grid columnconfigure $win 1 -weight 0
        grid rowconfigure $win 0 -weight 0
        
        
        $self configurelist $args
        
        if {$options(-readonly)} {
            $fileEntry configure -state disabled
            $win.browse configure -state disabled
        }
        set lastDir [pwd];               # last default dir is here.
    }
    
    ##
    # _browseForFile
    #    In response to the button, browse for a new file and set it in the
    #    -value option if one is chosen.
    #
    method _browseForFile {} {
        set newFile [tk_getOpenFile \
            -filetypes $options(-filetypes) -initialdir $options(-defaultdir) \
            -parent $win -title "Choose file for $options(-name)"             \
        ]
        if {$newFile ne ""} {
            set options(-value) $newFile
            set options(-defaultdir) [file dirname $newFile]
        }
    }
        
}

##
# @class FileEditor
#    Provides an editor for properties whose values should be filenames.
#
snit::type FileEditor {
    component prop
    delegate option * to prop
    delegate method * to prop
    
    ##
    # constructor
    #   Wrap the property and configure it.  We look like the property except that
    #   we cancreate an editor.
    #
    constructor args {
        install prop using property %AUTO% {*}$args
    }
    ##
    # makeEditor
    #    Create an editor GUI.  This is a FilePropertyView
    #
    # @param[in] path - widget the view should have.
    #
    method makeEditor path {
        FilePropertyView $path -name [$prop cget -name] -value [$prop cget -value] \
            -readonly [expr {![$prop cget -editable]}]
        
        return $path
    }
    ##
    # saveValues
    #   Save the values from an editor into the property.
    #
    # @param[in] path - widget path of the editor widget.
    #
    method saveValues path {
        $prop configure -value [$path cget -value]
    }
}

