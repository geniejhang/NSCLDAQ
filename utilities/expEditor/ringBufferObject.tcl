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

package provide ringBufferObject 1.0
package require ringBuffer
package require daqObject
package require Tk
package require img::png


##
# @class RingBufferObject
#   This class encapsulates a ring buffer item, that is pure data with a
#   DaqObject that handles the display stuff.
#
snit::type RingBufferObject {
    component data
    component gui
    
    delegate option -provider to data
    delegate option -canvas   to gui
    
    # Expose all but clone (which we have to handle)
    # to the world:
    
    delegate method getProperties to data
    delegate method addSink       to data
    delegate method clearSinks    to data
    delegate method rmSink        to data
    delegate method getSinks      to data
    
    delegate method drawat        to gui
    delegate method moveto        to gui
    delegate method moveby        to gui
    delegate method addtag        to gui
    delegate method rmtag         to gui
    delegate method tags          to gui
    
    ##
    # typeconstructor
    #    Create the image that will be bound into all our GUI elements.
    typeconstructor {
        image create photo RingBufferIcon -format png -file ringbuffer.png
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
}