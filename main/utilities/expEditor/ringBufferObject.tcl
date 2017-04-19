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
# @file ringBufferObject.tcl
# @brief Provide graphical wrapper on Ring buffer objects.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide ringBufferObject 1.1
package require ringBuffer
package require daqObject
package require Tk
package require img::png
package require Label


##
# @class RingBufferObject
#   This class encapsulates a ring buffer item, that is pure data with a
#   DaqObject that handles the display stuff.
#
snit::type RingBufferObject {
    component data
    component gui
    component Label
    
    delegate option -provider  to data
    delegate option -changecmd to data
    delegate option -canvas   to gui
    
    # Expose all but clone (which we have to handle)
    # to the world:
    
    delegate method getProperties to data
    delegate method addSink       to data
    delegate method clearSinks    to data
    delegate method rmSink        to data
    delegate method getSinks      to data
    
    delegate method addtag        to gui
    delegate method rmtag         to gui
    delegate method tags          to gui
    delegate method getPosition   to gui
    delegate method getId         to gui
    delegate method size          to gui
    delegate method bind          to gui

    
    # If not blank, this is the source object.
    
    variable sourceObject ""
    
    ##
    # typeconstructor
    #    Create the image that will be bound into all our GUI elements.
    typeconstructor {
        image create photo RingBufferIcon -format png \
            -file [file join [file dirname [info script]] ringbuffer.png]
    }
    
    ##
    # constructor
    #   Construct an object
    #   -  Install the components.
    #   -  Configure them:
    #
    constructor args {
        install data using RingBuffer %AUTO%
        install gui  using DaqObject %AUTO% -image RingBufferIcon
        
        $self configurelist $args
        
    }
    ##
    # destructor
    #   Destroy our components.
    
    destructor  {
        $data destroy
        $gui  destroy
        if {$Label ne ""} {
            $Label destroy
        }
    }
    
    ##
    # _replaceData
    #   Replace our data object.  Used by clone.
    #
    method _replaceData newdata {
        $data destroy
        set data $newdata
    }

    #--------------------------------------------------------------------------
    # Public methods
    #
    
    
    ##
    # clone
    #   Create a clone.  This is done by returning an object that has
    #   copies of our components as its own.
    #  @note to accomplish this we need private methods to override the existing
    #        values of the data object.
    #  @return RingBufferObject
    #
    method clone {} {
        set newObject [RingBufferObject %AUTO%]
        $newObject _replaceData [$data clone]
        
        return $newObject
    }
    
    ##
    # type
    #   Return object type.
    #
    method type {} {
        return ring
    
    }
    ##
    # connect
    #   Called when the object is connected to another object.
    #
    # @param direction - from if we are the source of the connection.
    #                    to if we are the sink for the connection.
    # @param object    - Object we are being connected to/from.
    #
    method connect {direction object} {
        if {$direction eq "to"} {
            set sourceObject $object
        } else {
            $self addSink $object
        }
    }
    ##
    # disconnect
    #    Called when an object is being disconnected
    # @param object - object we are being disconnected from/to.
    #
    method disconnect object {
        if {$object eq $sourceObject} {
            set sourceObject ""
        } else {
            $self rmSink $object
        }
    }
    ##
    # isConnectable
    #
    #   Called to determine if the object can accept a specific type of
    #   connection:
    #
    # @param direction - either 'from' or 'to' indicating whether the desired
    #                    connection is from or to this object.
    # @return bool     - True if the object can be connected as desired.
    #
    method isConnectable direction {
        if {$direction eq "from"} {
            # we can always source connections (well really there are limits
            # But the default is 100 sinks).
            
            return true
        } elseif {$direction eq "to"} {
            # We can only be a sink for one object:
            
            return [expr {$sourceObject eq ""}]
        }
    }
    ##
    # connectionPropertyChanged
    #
    #   Called if the properties of a connected object change.  At present
    #   we don't need to take any action.
    #
    # @param obj - the object whose properties changed.
    #
    method connectionPropertyChanged obj {}

    ##
    # propertyChanged
    #   The label may have changed
    #   If the name or host are not "" we'll set the label to
    #   name@host else we'll set it to "".
    #
    method propertyChanged {} {
        set properties [$data getProperties]
        set nameProp [$properties find name]
        set hostProp [$properties find host]
        
        set name [$nameProp cget -value]
        set host [$hostProp cget -value]
        
        if {($name ne "") || ($host ne "") } {
            $Label configure -text $name@$host
        } else  {
            $Label configure -text ""
        }
    }
    
    ##
    # drawat
    #    This draws the GUI at a specified location.
    #    - Tell the GUI to run its drawat.. this may be what makes the image
    #      on the canvas the first time
    #    - If we've not yet installed the label object install it
    #    - Invoke propertyChanged to set the label's text.
    #
    # @param x    - Desired x postion of the object.
    # @param y    - Desired y position of the object.
    #
    method drawat {x y} {
        $gui drawat $x $y
        if {$Label eq ""} {
            install Label using Label %AUTO%                      \
                -canvas [$gui cget -canvas] -tag $self -id [$gui getId]
            $self propertyChanged
        } else {
            $Label move $x $y
        }
    }
    ##
    #  movto
    #   Move the GUI to the specified coordinates and drag the
    #   label along with it:
    #
    # @param x,y  - new position for the object.
    #
    method moveto {x y} {

        $gui moveto $x $y
        $Label move $x $y
    }
    ##
    # moveby
    #   Move the object and label by dx,dy.  Note that we had hoped to just move
    #   the tag but cant' because the gui object maintains inner stat that tells
    #   it where it is.
    #
    # @param dx,dy - amount to move the object in x,y
    #
    method moveby {dx dy} {
        $gui moveby $dx $dy
        $options(-canvas) move $self $dx $dy;        # Moves the label.
    }

}