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
# @file sqlwhere.tcl
# @brief Isolate SQL WHERE Clause building for SQL itself.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide SqlWhere 1.0
package require snit

##
#  This package contains a set of classes that build SQL where clauses
#  without expressing them as Where clauses.  This allows  applications
#  to provide a conditional query API without binding the applications to
#  writing explicit WHERE clauses.  Benefits are:
#
#  *  Users can be a bit less sophisticated.
#  *  In _theory_ the underlying API implementation layer could be rewritten
#     on top of something other than SQL and only the classes in this
#     file would need to be rewritten.
#
#  @note
#    All classes in this file are expected to implement a toString method
#    that will return the actual SQL WHERE clause body (the keyword WHERE
#    should not be part of that string).
#
#

##
#  RawQueryFilter
#     Escape that allows a raw SQL Where clause to be incorporated:
#
snit::type RawQueryFilter {
    variable clause
    
    constructor string {
        set clause $string
    }
    method toString {} {
        return $clause
    }
}
#-----------------------------------------------------------------------------
##
#  Simple relations.  Note that unlike the C++ binding the relational operator
#  must be explicitly provided.
#
snit::type RelationToNonStringFilter {
    variable m_field
    variable m_op
    variable m_value
    
    constructor {field op value} {
        set m_field $field
        set m_op    $op
        set m_value $value
    }
    method toString {} {
        return "$m_field $m_op $m_value"
    }
}
snit::type RelationToStringFilter {
    variable m_field
    variable m_op
    variable m_value
    
    constructor {field op value} {
        set m_field $field
        set m_op    $op
        set m_value $value
    }
    method toString {} {
        return "$m_field $m_op '$m_value'"
    }
}

#----------------------------------------------------------------------------
# Compound filters
#
snit::type CompoundFilter {
    variable m_items
    variable m_combiner
    
    constructor {combiner} {
        set m_combiner $combiner
    }
    
    method addClause {clause} {
        lappend m_items $clause
    }
    method getItems {} {return $m_items}
    
    method toString {} {
        set itemTexts [list]
        foreach item $m_items {
            lappend itemTexts "( [$item toString])"
        }
        set condBody [join $itemTexts " $m_combiner " ]
        return "($condBody)"
    }
}

snit::type AndFilter {
    component CompoundFilter
    
    delegate method * to CompoundFilter
    
    constructor {} {
        install CompoundFilter using CompoundFilter %AUTO% AND
    }
}

snit::type OrFilter {
    component CompoundFilter
    
    delegate method * to CompoundFilter
    
    constructor {} {
        install CompoundFilter using CompoundFilter %AUTO% OR
    }
}

snit::type InFilter {
    component CompoundFilter
    
    delegate method getItems to CompoundFilter
    constructor {} {
        install CompoundFilter using CompoundFilter %AUTO% ,
    }
    method addString value {
        $CompoundFilter addClause '$value'
    }
    method addNumber value {
        $CompoundFilter addClause $value
    }
    method toString {} {
        set items [join [$CompoundFilter getItems] ", "]
        return "IN ($items)"
        
    }
    
}