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
# @file properties.tcl
# @brief Tcl code to implement properties and property lists.
# @author Ron Fox <fox@nscl.msu.edu>
#

##
# A property is a triple that contains:
#
#   *  A name (property name)
#   *  A value (property value)
#   *  A validator that can provide a data type to the property.
#

package provide properties 1.0
package require snit

##
# @class property
#    Defines a single property:
#
# OPTIONS
#    * -name     - Name of the property.
#    * -value    - Property value.
#    * -validate - validation object for value (snit::validation object).
#    * -editable - the property can be modified by a user via GUI.
#    * -changecmd- Script called when the value of the property changed.
#
# SCRIPT SUBSTITUTIONS:
#   The -changecmd script accepts the following substitutions:
#
#    *   %P  - The property object.
#    *   %N  - Name of the property.
#    *   %V  - New value of the property.
#
snit::type property {
    
    option -name -readonly 0
    option -value -default "" -configuremethod _validate
    option -validate -default ""
    option -editable -default 1 -type snit::boolean
    option -changecmd -default [list]
    
    constructor args {
        $self configurelist $args
        
        if {$options(-name) eq ""} {
            error "Property constructor -- properties must have a -name"
        }
    }
    ##
    # _validate
    #   If there's a validation use it to check the validity of the
    #   a value.
    #
    # @param optname - name of option to be validated (-value).
    # @param value   - proposed value.
    #
    #   If value is acceptable the option is updated otherwise an error is raised.
    method _validate {optname value} {
        if {$options(-validate) eq ""} {
            set options($optname) $value
        } else {
            set validator $options(-validate)
            set options($optname) [uplevel #0 {*}$validator validate [list $value]]
        }
        #  Validation makes an error so we only get here if the config succeeded.
        
        $self _dispatch \
            -changecmd [list %P $self %N [list $options(-name)] %V [list $options($optname)]]
        
    }
    ##
    # _dispatch
    #   Dispatch a callback
    #
    # @param optname - Name of the option holding the callback script.
    # @param substs  - Map string holding the parameter substitutions.
    #                  this is in the same format as a mapping string for
    #                  [stiring map].
    #
    # @return - the return value of the script which is executed at the global level.
    # @note If there is no script "" is returned.
    # @note The script is run at the global level.
    #
    method _dispatch {optname substs} {

        set script $options($optname)
        if {$script ne ""} {
            set script [string map $substs $script]
            return [uplevel #0  $script]
        } else {
            return ""
        }
    }
}
#------------------------------------------------------------------------------
#  Strongly typed properties:
#

##
# @class IntegerProperty
#
#   Encapsulates a property that must be an integer.  The encapsulate just
#   forces control over the -validate option
#
# OPTIONS:
#   - -signed - if true the optino can be negative otherwise it must be >= 0
#
snit::type IntegerProperty {
    component prop
    
    option -signed -default 0
    
    #  Note that we don't export -validate because we want exlusive control
    #  over that.
    
    delegate option -name      to prop
    delegate option -value     to prop
    delegate option -editable  to prop
    delegate option -changecmd to prop
    
    delegate method * to prop
    delegate option * to prop
    
    ##
    # constructor
    #   Constrution just installs the base class object, runs the configuration and
    #   installs our validator.
    #
    # @param[in] args - the construction time configuration option/value pairs.
    #
    constructor args {
        install prop using property %AUTO% -name temporary
        $self configurelist $args
        $prop configure -validate [mymethod _integer]
    }
    destructor {
       $prop destroy
    }
    #---------------------------------------------------------------------------
    # _integer
    #   Validate that the new value must be an integer.
    #
    # @param validate - hard coded value 'validate' given how this is called.
    # @param value    - new proprosed value.
    # @return value if ok
    # @throw error if not ok.
    # @note - the -signed option determines if negative values are allowed:
    #
    method _integer {validate value} {
        if {![string is integer -strict $value]} {
            error "[$prop cget -name] property must be an integer was $value"
        }
        if {$options(-signed) && ($value < 0)} {
            error" [$prop cget -name] property must ba positive integer was $value"
        }
        return $value
    }
}

##
# @class EnumeratedProperty
#
#    Property that must be selected from a list of valid properties.
#
# OPTIONS:
# #  - - list of valid values.
#
snit::type EnumeratedProperty {
    component prop
    
    option -values -default [list]
    
    #  Note that we don't export -validate because we want exlusive control
    #  over that.
    
    delegate option -name      to prop
    delegate option -value     to prop
    delegate option -editable  to prop
    delegate option -changecmd to prop
    
    delegate method * to prop
    delegate option * to prop
    
    ##
    # constructor
    #   Constrution just installs the base class object, runs the configuration and
    #   installs our validator.
    #
    # @param[in] args - the construction time configuration option/value pairs.
    #
    constructor args {
        install prop using property %AUTO% -name temporary
        $self configurelist $args
        $prop configure -validate [mymethod _inlist]
    }
    destructor {
       $prop destroy
    }
    #---------------------------------------------------------------------------
    # _inlist
    #   Validate that the new value must be in -values
    #
    # @param validate - hard coded value 'validate' given how this is called.
    # @param value    - new proprosed value.
    # @return value if ok
    
    #
    method _inlist {validate value} {
        if {$value ni $options(-values)} {
            error "[$prop cget -name] \
property is $value but must be in the set: {[join $options(-values) ", "]}"
        }
        
        return $value
    }    
}

##
# @class ListProperty
#    Class to ensure that property values are valid Tcl lists.  It's possible
#    to specify minimum and maximum list sizes. 
#
# OPTIONS
#   -  -minlen - minimumlist length (defaults to 0).
#   -  -maxlen - Maximum list length  default to no limit.
#
snit::type ListProperty {
    component prop
    
    option -minlen  0
    option -maxlen ""
    
    #  Note that we don't export -validate because we want exlusive control
    #  over that.
    
    delegate option -name      to prop
    delegate option -value     to prop
    delegate option -editable  to prop
    delegate option -changecmd to prop
    
    delegate method * to prop
    delegate option * to prop
    
    ##
    # constructor
    #   Constrution just installs the base class object, runs the configuration and
    #   installs our validator.
    #
    # @param[in] args - the construction time configuration option/value pairs.
    #
    constructor args {
        install prop using property %AUTO% -name temporary
        $self configurelist $args
        $prop configure -validate [mymethod _islist]
    }
    destructor {
       $prop destroy
    }
    #---------------------------------------------------------------------------
    # _islist
    #   Validate that the new value must be a tcl list.
    #
    # @param validate - hard coded value 'validate' given how this is called.
    # @param value    - new proprosed value.
    # @return value if ok
    
    #
    method _islist {validate value} {
        if !{[string is list -strict $value]} {
            error "Property [$prop cget -name] must be a Tcl list but was $value"
        }
        set len [llength $value]
        if {$len < $options(-minlen)} {
            error "Property [$prop cget -name] must have a least $options(-minlen) elements: $value; $len elements"
        }
        if {($options(-maxlen) ne "") && ($len > $options(-maxlen))} {
            error "Property [$prop cget -name] must have a most $options(-maxlen) elements: $value; $len elements"
        }
        return $value
    }    
}


#------------------------------------------------------------------------------
#  Structures that encapsulate one or more property objects:


##
# @class propertylist
#    List of properties.
#
# METHODS:
#    get   - Returns he current property list.
#    add   - Adds a new property to the list.
#    clear - Clear the property list.
#    foreach - Execute a script for each property in the list.
#
# OPTIONS
#   *  -changecmd - Sets the -changecmd of all properties in the list.
#                this is memorized so that as new properties are added to the
#                list this is set for them as well.
#
snit::type propertylist {
    variable props [list]
    
    option -changecmd -default [list] -configuremethod _setChangeCmd
    
    
    constructor args {
        $self configurelist $args
    }
    ##
    # _setChangeCmde
    #   Modifies the -changecmd option
    #   *   Set the value for all current properties.
    #   *   Save it to set in future properties.
    #
    # @param optname - Name of the option to change.
    # @param value   - new script value    
    method _setChangeCmd {optname value} {
        set options($optname) $value
        $self foreach p  {
            $p configure -changecmd $value
        }
    }
    
    #--------------------------------------------------------------------------
    #  Public methods
    #
    
    
    ##
    # get
    #   Return the list of properties:
    #
    method get {} {
        return $props
    }
    ##
    #  add
    #    Append a property to the list:
    #
    # @param property  - property object to append.
    method add {property} {
        lappend props $property
        $property configure -changecmd $options(-changecmd)
    
    }
    ##
    #  clear
    #
    # Clear the property list.  Note that the properties themselves are not
    # destroyed.
    #
    method clear {} {
        set props [list]
    }
    ##
    #  foreach
    #    Iterate over all properties in the list applying a script to each.
    # @param var - variable that will receive the property object in script's
    #              stack level.
    # @param script - script to execute
    #
    method foreach {var script} {
        upvar 1 $var property
        foreach property $props {
            uplevel 1 $script
        }
    }
    ##
    # find
    #   @param name - name of property to find.
    #   @return property - empty string if does not exist.
    method find {name} {
        set result ""
        
        $self foreach property {
            if {[$property cget -name] eq $name} {
                set result $property 
		break
            }
        }
        return $result
    }
}

