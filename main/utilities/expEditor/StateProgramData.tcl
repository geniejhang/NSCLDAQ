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
# @file StateProgramData.tcl
# @brief Contains the data for a state program.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide stateProgramData 1.0
package require snit
package require properties

##
# @class
#   StateProgramData
#
#   Encapsulates the property list for a state aware program.
#
snit::type StateProgramData {
    component properties
    
    delegate option -changecmd to properties
    
    ##
    # constructor
    #   Create the propertylist.
    #
    constructor args {
        install properties using propertylist %AUTO%
        
        $properties add [property %AUTO% \
            -name name]
        $properties add [property %AUTO% -name host ]
        $properties add [EnumeratedProperty %AUTO% -values {true false} \
            -name enable -value true]
        $properties add [EnumeratedProperty %AUTO% -values {true false} \
            -name standalone -value false ]
        $properties add [property %AUTO% -name path ]
       
        $properties add [property %AUTO% -name {Input Ring} -editable 0]
        $properties add [property %AUTO% -name {Output Ring} -editable 0]
        $properties add [property %AUTO% -name {type} -editable 0 -value StateProgram]
        $properties add [ListProperty %AUTO% -name {Program Parameters} -editable 1]
        
        $self configurelist $args
    }
    ##
    # destructor
    #   Clean up properties an list:
    #
    destructor {
        $properties foreach property {
            $property destroy
        }
        $properties destroy
    }
    #------------------------------------------------------------------------
    #  Public methods
    #
    
    ##
    # getProperties
    #   Return the property list:
    
    method getProperties {} {
        return $properties
    }
    ##
    #   clone
    #
    #  produce a duplicate of this object
    #
    
    method clone {} {
        set newObj [StateProgramData %AUTO%]
        
        ## Propagate current property values:
        
        set newprops [$newObj getProperties]
        $properties foreach prop {
            set newprop [$newprops find [$prop cget -name]]
            $newprop configure -value [$prop cget -value]
        }
        return $newObj
    }
    
    
}
##
# @class ReadoutProgram.
#
#   This is a state program with additional properties:
#   *  port     - If nonempty the port on which the Tcl server starts listening.
#   *  sourceid - Source id emitted in body headers (--sourceid) (default 0)
#   *  init script - Initialization script path (--init-script) (default empty)
#   *  appname  - Application name which if non empty overrides the default which is
#                 the 
#   *  status service - Status service name (--status-service) (Default StatusAggregator).
#
snit::type ReadoutProgram {
    component stateProgram
    
    delegate option * to stateProgram
    delegate method * to stateProgram
    
    ##
    # constructor
    #    install the state program and extend its property list.
    #
    constructor args {
        install stateProgram using StateProgramData %AUTO%
        
        set properties [$stateProgram getProperties]
        $properties add [IntegerProperty %AUTO% -name port]
        $properties add [IntegerProperty %AUTO% -name sourceid -value 0]
        $properties add [property %AUTO% -name {init script}]
        $properties add [property %AUTO% -name appname]
        $properties add [property %AUTO% -name {status service} -value StatusAggregator]
        
        # Set the type property to Readout:
        
        [$properties find type] configure -value Readout
        
        $self configurelist $args
    }
    ##
    # destructor
    #
    destructor {
        $stateProgram destroy
    }
}
##
# @class EventLogProgram
#
#   State program with the following addition properties:
#   *  destdir      - Where the event files get written (default - empty which is wd of the program)
#   *  segmentsize  - Size of event file segments.      (default 2g)
#   *  checksum     - Enable creation of checksum files (default true).
#   *  combine runs - Glues all the runs together into one event file (default false).
#   *  prefix       - Text prefix to prepend on event files (default empty string)
#   *  freewarn     - Percentage of disk space below which to log a warning (default 10)
#   *  freesevere   - Percentage of disk space below which to log a sever error (default 1)
#   *  appname      - Application name - if not blank overrides the default with is the name.
#   *  status service - Service to which log messages are sent (default StatusAggregator).
#
snit::type EventLogProgram {
    component stateProgram
    
    delegate method * to stateProgram
    delegate option * to stateProgram
    
    ##
    # constructor
    #   install the state program base class.
    #   add our properties.
    #   set the type to EventLog
    #   process any construction time configuration options.
    #
    constructor args {
        install stateProgram using StateProgramData %AUTO%
        
        set properties [$stateProgram getProperties]
        $properties add [property %AUTO% -name destdir]
        $properties add [property %AUTO%                                       \
            -name segmentsize -value 2g -validate [mymethod _validSize]   \
        ]
        $properties add [EnumeratedProperty %AUTO%                             \
            -name checkum -values [list true false] -value true                \
        ]
        $properties add [EnumeratedProperty %AUTO%                             \
            -name {combine runs} -values [list true false] -value false        \
        ]
        $properties add [property %AUTO% -name prefix]
        $properties add [IntegerProperty %AUTO% -name freewarn -value 10]
        $properties add [IntegerProperty %AUTO% -name freesevere -value 1]
        $properties add [property %AUTO% -name appname]
        $properties add [property %AUTO%                                       \
            -name {status service} -value StatusAggregator                     \
        ]
    
        # Set the type property to Readout:
        
        [$properties find type] configure -value Readout
        
        # process construction time configuration.
    
        $self configurelist $args
    }
    #--------------------------------------------------------------------------
    # Custom validators.
    #
    
    ##
    # _validSize
    #   Throw an error if a proposed value is not a valid size. Valid sizes are
    #   either integers or integers followed by one of the following suffix
    #   unit specification characters:
    #   *   k   - Units are Kbytes (1024).
    #   *   m   - Units are Mbytes (1024*1024).
    #   *   g   - Units are Gbytes (1024*1024*1024).
    #
    # @param validate  - if this were an object that would be the method name
    #                    to do the validation.
    # @param proposed  - proposed new value.
    # @throw error if the value is not a valid size specification.
    #
    method _validSize {validate proposed} {
        # Integer values are fine:
        
        set propsed [string trim $proposed]
        if {[string is integer -strict $proposed]} {
            return $proposed
        }
        
        # Otherwise separate the string into last charater and all the preceding
        # ones:
        
        set prefix [string range $proposed 0 end-1]
        set suffix [string index $proposed end]
        
        if {($suffix in [list k m g]) && [string is integer -strict $prefix]} {
            return $proposed
        }
        error "Invalid size specification: $proposed"
            
    }
}
