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
# @file <filename>.tcl
# @brief <brief purpose>
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide Service 1.1
package require serviceData
package require daqObject
package require img::png
package require Label


##
# @class Service
#   Encapsulates the data and user interface for defining a service item.
#   A service is a program that must run but is neither connected to data flow
#   nor to the state manager.
#
snit::type Service {
    component data
    component gui
    component Label
 
    delegate option -provider to data
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

    
    ##
    # Construct our icon image:
    
    typeconstructor {
        image create photo ServiceIcon  \
            -format png                 \
            -file [file join [file dirname [info script]] sysprogram.png] 
    }
    ##
    # constructor
    #   Install the pieces and configure
    #
    constructor args {
        install data using ServiceData %AUTO%
        install gui  using DaqObject %AUTO% -image ServiceIcon
        
        $self configurelist $args
    }
    ##
    # destructor -- destroy our components.
    #
    destructor {
        if {$data ne ""} {
            $data destroy
        }
        if {$gui ne ""} {
            $gui destroy
        }
        if {$Label ne ""} {
            $Label destroy
        }
    }
    ##
    #  clone
    #   Create a copy of self.
    #
    # @return copy of self.
    #
    method clone {} {
        set newObj [Service %AUTO%]
        set myprops [$data getProperties]
        set newprops [$newObj getProperties]
        
        $myprops foreach property {
            set name [$property cget -name]
            set value [$property cget -value]
            
            set newprop [$newprops find $name]
            $newprop configure -value $value
        }
        
        return $newObj
    }
    ##
    # type
    #   Return the object type
    #
    method type {} {
        return service
    }
    ##
    # connect
    #   This tosses an error since we've told the world we can't be a party
    #   to any connections.
    #
    # @param direction
    # @param object
    #
    method connect {direction object} {
        error "This object cannot participate in any connections"
    }
    ##
    # disconnect
    #   Throws an error since we can't have been connected to anything.
    #
    # @param object
    #
    method disconnect object {
        error "disconnect - This object cannot have been connected to anything."
    }
    ##
    # isConnectable
    #
    #  @param direction
    #  @return false
    #
    method isConnectable direction {
        return false
    }
    ##
    # connectionPropertyChanged
    #    Called if the properties of an object we are connected to changed.
    #    Note that since we cannot be connected to anything this is a no-op.
    # @param object - the object whose properties changed.
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