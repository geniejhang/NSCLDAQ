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
# @file ServiceData.tcl
# @brief <brief purpose>
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide serviceData 1.0
package require snit
package require properties

##
# @class ServiceData
#
#  Contains the data that is associated with a service program.
#  Service programs cannot be connected to data flows or the state machine.
#  Service programs only have a name, a host and a command string.
#
snit::type ServiceData {
    component propertylist;              # The properties in the data.
    delegate option -changecmd to propertylist
    
    ##
    # constructor
    #   Create a property list and stock it with the standard properties:
    #
    constructor args {
        install propertylist using propertylist %AUTO%
        
        $propertylist add [GenericPropertyEditor %AUTO% -name name]
        $propertylist add [GenericPropertyEditor %AUTO% -name host]
        $propertylist add [FileEditor %AUTO% -name path]
        $propertylist add [DirectoryEditor %AUTO% -name wd -value [pwd]] 
        $propertylist add [GenericPropertyEditor %AUTO% -name type -value service -editable 0]
        $propertylist add [ListEditor %AUTO% -name args]
        
        $self configurelist $args
        
    }
    ##
    # destructor
    #   Destroy properties and the list:
    #
    destructor {
        if {$propertylist ne""} {
            $propertylist foreach property {
                $property destroy
            }
            $propertylist destroy
        }
    }
    
    ##
    # getProperties
    #    Return the current property set:
    #
    # @return propertylist
    #
    method getProperties {} {
        return $propertylist
    }
    ##
    # clone
    #   Produce a duplicate of this object.  Properties are also cloned.
    #
    # @return cloned copy of self.
    #
    method clone {} {
        set newObj [ServiceData %AUTO%]
        set newProps [$newObj getProperties]
        
        $propertylist foreach prop {
            set name [$prop cget -name]
            set value [$prop cget -value]
            
            set np [$newProps find $name]
            $np configure -value $value
        }
        
        return $newObj
    }
}
##
# @class DataFlowData
#     Derives from Service Data to provide a data flow object.  Dataflow
#     objects are like services but they can appear in the dataflow.
#     Unlike the other dataflow object types, however they do not participate
#     in state transitions.  Examples of dataflow objects are filters, the
#     scaler display programs root hooked to nscldaq and so on.
#
#   From the data point of view, this is a service with the additional
#   properties:
#     * Input Ring - optional ring from which the program takes data.
#     * Output Ring - optional ring into which the program puts data.
#
#  @note - The rings are not editable.  They are set by doing connections.
#
snit::type DataFlowData {
    component service
    
    delegate method * to service
    delegate option * to service
    
    ##
    # constuctor
    #    Install service, get its property list and augment it.
    #    Configure the item.
    #
    constructor args {
        install service using ServiceData %AUTO%
        
        set props [$service getProperties]
        $props add [GenericPropertyEditor %AUTO% -name {Input Ring} -editable 0]
        $props add [GenericPropertyEditor %AUTO% -name {Output Ring} -editable 0]
        [$props find type] configure -value dataflow
        
        $self configurelist $args
    }
    destructor {
        if {$service ne ""} {
            $service destroy;                 # destroy base class.
        }
    }
    ##
    # clone
    #   Duplicate self.
    #
    # @return DataFlowData object.
    #
    method clone {} {
        set newObj       [DataFlowData %AUTO%]
        set newProps     [$newObj getProperties]
        set propertyList [$self getProperties]
        
        # Copy our properties into the new object.
        
        $propertylist foreach prop {
            set name [$prop cget -name]
            set value [$prop cget -value]
            
            set np [$newProps find $name]
            $np configure -value $value
        }
        
        return $newObj
    }
}