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
    # setLabelText
    #   Set the text of the label.  It is an error to call this if Label has not
    #   yet been installed.
    #
    # @param str
    #
    method setLabelText str {
        if {$Label eq "" } {
            error "Service::setLabelText - Label has not yet been installed"
        } else {
            $Label configure -text $str
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
    ##
    # gui
    #   @return the GUI element.
    #
    method gui {} {
        return $gui
    }
    ##
    # data
    #   Return the current data item and optionally replace it.
    #
    # @param newData - optional if supplied the data element is replaced by this.
    # @return current data object.
    #
    method data {{newData ""}} {
        set result $data
        if {$newData ne ""} {
            set data $newData
        }
        return $result
    }
}
##
# @class DataFlow
#    Packages a data flow item along with its graphical user interface elements.
#    We are a connectible dataflow object with a different graphical representation.
#
snit::type DataFlow {
    component ServiceObject
    
    variable inring ""
    variable outring ""
    
    # Anything we don't override gets handled by the 'base' class.
    
    delegate method * to ServiceObject
    delegate option * to ServiceObject
    
    typevariable icon
    
    ##
    # typeconstructor
    #    Create the image; save it in the icon.
    #
    typeconstructor {
        set here [file dirname [info script]]
        set icon [image create photo -format png -file [file join $here analysis.png]]
    }
    ##
    # constructor
    #   Construct the base class.
    #   Set the GUI icon
    #   Replace the data with one of ours.
    #   Do additional configuration indicated by the command line:
    #
    constructor args {
        install ServiceObject using Service %AUTO% {*}$args
        [$self gui] configure -image $icon
        set oldData [$self data [DataFlowData %AUTO%]]
        $oldData destroy
        
        $self configurelist $args
    }
    
    destructor {
        if {$ServiceObject ne ""} {
            $ServiceObject destroy
        }
    }
    
    ##
    # isConnectable
    #    This object is connectable in any direction.
    #
    # @param direction - direction of the connection.
    # @return true
    #
    method isConnectable direction {
        return true
    }
    ##
    # connect
    #   Processes the connection of an object to us.  We can only accept
    #   connections from and to a ringbuffer.  This means that pipelines
    #   must be represented as single elements (a script) that has an input
    #   and output ring.
    #
    # @param direction - Can be:
    #                   *  from - the connection is from us (to the output ring).
    #                   *  to   - the connection is to us (from the input ring).
    # @param object    - The object we are being connected to/from.
    # @note - from connections will disable the host and set it from the ring's
    #         URI.
    #
    method connect {direction object} {
        if {[$object type] ne "ring"} {
            error "Data flow programs can only be connected to rings"
        }
        
        # From is from us to our Output Ring
        
        if {$direction eq "from"} {
            $self _setOutRingProperties $object
            set outring $object
            $self _disableHostEditing
        } elseif {$direction eq "to"} {
            $self _setInRingProperties $object
            set inring $object
        } else {
            error  "Invalid Direction: $direction for connect."
        }
        return 1;                   # Success.
    }
    ##
    # disconnect
    #    Called when a ring is being disconnected from us (by deleting a
    #    connector).
    #
    # @param ring  - the ring being disconnected from us.
    #
    method disconnect ring {
        if {$ring eq $outring} {
            $self _clearOutRingProperties
            set outring ""
            $self _enableHostEditing
        } elseif {$ring eq $inring} {
            $self _clearInRingProperties
            set inring ""
        } else {
            error "The object $ring is not connected to us."
        }
    }
    ##
    # connectionPropertyChanged
    #   If a ringbuffer we are connected to has a property changed, this is
    #   called.  We'll figure out which it is (input or output ring) and
    #   update the properties accordingly.
    #
    # @param obj   - The object that's had a change.
    #
    method connectionPropertyChanged obj {
        if {$obj == $inring} {
            $self _setInRingProperties $obj
        } elseif {$obj == $outring} {
            $self _setOutRingProperties $obj
        } else {
            error "$obj is not connected to this object."
        }
    }
    ##
    #  clone
    #   Create a copy of self.
    #
    # @return copy of self.
    #
    method clone {} {
        set newObj [DataFlow %AUTO%]
        set myprops [$self getProperties]
        set newprops [$newObj getProperties]
        
        $myprops foreach property {
            set name [$property cget -name]
            set value [$property cget -value]
            
            set newprop [$newprops find $name]
            $newprop configure -value $value
        }
        
        return $newObj
    }
    #-------------------------------------------------------------------------
    # Private methods:
    
    ##
    # _setOutRingProperties
    #    Given a ring buffer that is being connected to us:
    #    - Set our host to the host of the ring.
    #    - Set our Output Ring to the ring's name.
    #
    # @param ring - ring buffer we are geing connected to.
    #
    method _setOutRingProperties ring {
        set rprops [$ring getProperties]
        set host [[$rprops find host] cget -value]
        set ring [[$rprops find name] cget -value]
        
        set myprops [$self getProperties]
        [$myprops find {Output Ring}] configure -value $ring
        [$myprops find host] configure -value $host
    }
    ##
    # _clearOutRingProperties
    #
    #   Clear the value of the Output Ring property.
    #
    method _clearOutRingProperties {} {
        set props [$self getProperties]
        [$props find {Output Ring}] configure -value {}
    }
    ##
    #  _disableHostEditing
    #    Turn off the ability to edit the host.  This is needed when an output
    #    ring is defined as the ring's host constrains our host.
    #
    # @note at this time there's no way to ghost the host on property editors
    #       that are already up.
    #
    method _disableHostEditing  {} {
        set props [$self getProperties]
        [$props find host] configure -editable 0
    
    }
    ##
    # _enableHostEditing
    #   Turn on the ability to edit the host.  This ism done when an output ring
    #   is disconnected from the progra as the program no longer needs to co-locate
    #   with that ring.
    #
    #  See, however the note in _disableHostEditing
    #
    method _enableHostEditing {} {
        set props [$self getProperties]
        [$props find host] configure -editable 1
    }
    ##
    # _setInRingProperties
    #
    #   Sets our input ring properties to be the URI of the input ring.
    #   Also save the input ring.
    #
    # @param ring - the new input ring
    #
    method _setInRingProperties ring {
        
        # Figure out the URI of the ring:
        
        set rprops [$ring getProperties]
        set name [[$rprops find name] cget -value]
        set host [[$rprops find host] cget -value]
        set uri tcp://$host/$name
        
        #  Set our Input Ring value to the URI:
        
        set props [$self getProperties]
        [$props find {Input Ring}] configure -value $uri
        set inring $ring
        
    }
    ##
    # _clearInRingProperties
    #    Clears the input ring properties;
    #    - clears the URI of our Input Ring property.
    #    - clears the inring object.
    #
    method _clearInRingProperties {} {
        set props [$self getProperties]
        [$props find {Input Ring}] configure -value ""
        set inring ""
    }
}