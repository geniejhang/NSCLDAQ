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
#  To do list:
#    - If no database has been selected, many many menu entries should be
#      disabled or, alternatively yell if selected that a database needs to be
#      established.
#    - Tag list should periodically update (e.g. show new tags as they are added).
#

##
# @file Label.tcl
# @brief Provide a label to go along with objects.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide Label 1.0
package require Tk
package require snit

##
# @class Label
#    Provides a label that is associated with a parent object on a canvas.
#    the label is tagged so that if the tag is told to move, the label
#    automatically moves too.
#
#  OPTIONS:
#    - -text   Text to display in the label (dynamic).
#    - -id     Id of the parent object; Used to determine the label location.
#              (Readonly)
#    - -tag    Tag to apply to the label so that relative moves can be done
#              on the tag and the label will also move.
#              (Readonly)
#    - -canvas - Canvas on which the label is drawn (Readonly).
#
# METHODS:
#    move   - Informs the label that the parent has moved.  This allows the
#              label to move along with it if the parent moves to an absolute
#              position e.g.
#
snit::type Label {
    option -text   -default "" -configuremethod _newText
    option -id     -default "" -readonly 1
    option -tag    -default "" -readonly 1
    option -canvas -default "" -readonly 1
    
    variable id "";                     # my canvas id.
    #
    #  Cached size of the parent object so repositioning does not require
    #  any canvas queries about the parent.
    #
    variable xextent
    variable yextent
    
    ##
    # constructor
    #    Constructs a new label associated with an object.
    #    - process configuration option.
    #    - Figure out where the label goes given to object bounding box.
    #    - Draw the text.
    #    - Tag the text with the associated -tag value.
    #
    # @param args - the option/value pairs for construction time configuration.
    #
    constructor args {
        $self configurelist $args
        
        # Compute xextent, yextent and the position of the upper right corner
        # of the text.
        #
        set c $options(-canvas)
        set box [$c bbox $options(-id)]
        
        set xextent [expr {[lindex $box 2] - [lindex $box 0]}]
        set yextent [expr {[lindex $box 3] - [lindex $box 1]}]
        
        #  The position will actually be the lower left coordinate:
        
        set x [lindex $box 0]
        set y [lindex $box 3]
        
        #  Create the label:
        
        set id [$c create text $x $y                                           \
            -tags  $options(-tag) -text $options(-text) -anchor nw]
        
    }
    #--------------------------------------------------------------------------
    # Configturation management:
    
    ##
    # _newText
    #    Called when new text is requested of the label.
    #
    # @param optname - name of the option to modify in our configuration.
    # @param label   - new label value.
    #
    method _newText {optname label} {
        set options($optname) $label
        
        #  Our id must have been defined to change anything.  This handles
        #  the pathology that at construction time we'll get configured prior
        #  to the label being made.  This is ok because the constructor will
        #  make the object:
        
        if {$id ne ""} {
            set c $options(-canvas)
            $c itemconfigure $id -text $label
        }
    }
    #--------------------------------------------------------------------------
    # public methods:
    #
    
    ##
    # move
    #   Informs us that the item we're labelling has moved.  Note that
    #   the object can perform relative moves by just moving the tag.  This is
    #   needed because in an absolute move we need to maintain our position
    #   relative to the object.
    #
    # @param x  - The new x position of the object we label.
    # @param y  - The new y position of the object we label.
    #
    method move {x y} {
        # Compute the text position:
        
        set myx [expr {$x + $xextent}]
        set myy [expr {$y + $yextent}]
        
        # Move us to myx myy:
        
        set c $options(-canvas)
        $c coords $id $myx $myy
    }
}