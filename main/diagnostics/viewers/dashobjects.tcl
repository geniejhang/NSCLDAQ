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
# @file dashboard.tcl
# @brief Provide dashboard objects for the status display dashboard.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide Dashboard 1.0
package require Tk
package require snit
package require ringSerializer
package require ringBufferObject
package require stateProgramSerializer
package require stateProgram
package require dataSourceObject
package require dsourceSerializer
##
#  The purpose of this file is to provide a dashboard that shows data flow
#  bottlenecks in an experiment that has been defined by an experiment database.
#  This file is the view of the dashboard.  It's still necessary to provide the
#  logic that indicates how items will be rendered (e.g. to show ongoing
#  bottlenecks).  For the most part this is an extended canvas that understands
#  items like 'Ring's, 'StateProgram's, 'EventSources', 'EventBuilder's,
#  'Service's and 'Flows'.   Each of these items can have properties
#   associated with it.  The properties can alter the visual display of the
#   item.
#

##
#  This type represents a visual object.  Each object has the following
#  options:
#
#  - -x   - X position on the canvas.
#  - -y   - Y position on the canvas.
#  - -title - Title shown below the image.
#  - -highlight - Highlight color.  The highlight is a filled rectangle below
#                 that circumscribes the item and is beneath it in stacking order.
#  - -id    - Id of the object to display.
#  - -canvas - Canvas the object is displayed on.
#  - -data   - Arbitrary data that can be carried along with the object.
#
# @note Connectors are not displayable objects but uhm.. connectors as they have
#       a from - to rather than specific coordinates.
#

snit::type DashboardObject {
    option -x -configuremethod _move -default 0 
    option -y -configuremethod _move -default 0
    option -title -default "" -configuremethod _drawtitle
    option -highlight -default white -configuremethod _highlight
    option -id -readonly 1
    option -canvas -readonly 1
    option -data   -default ""
    
    variable titleId     "";        # Canvas id of title text.
    variable highlightId "";        # ID of highlight rectangle.
    
    ##
    # Constructor
    #   Create the object.  We're going to invoke _render because it's possible
    #   the object id was not supplied prior to the other attributes.
    
    constructor args {
        $self configurelist $args
        $self _render
    }
    ##
    # public methods:
    #
    
    ##
    # itemconf
    #    Change the configuration parameters of the encapsulated item.
    #
    method itemconf {option value} {
        $options(-canvas) itemconfigure $options(-id) $option $value
    }    
    #--------------------------------------------------------------------------
    # Private methods
    #
    
    ##
    # _render
    #   Draw the object completely.  To do this we do need to make an assumption
    #   if an object has multiple coordinates, the first one specifies the old
    #   -x, -y values and we move appropriately.
    #
    #  - Move the base item to x,y
    #  - If there's a highglight rectangle move it as well and lower it.
    #  - If there's a title, move it.
    #
    method _render {} {
        set c $options(-canvas)
        set x $options(-x)
        set y $options(-y)

        set coords [$c coords $options(-id)]
        set oldx [lindex $coords 0]
        set oldy [lindex $coords 1]
        
        set dx [expr {$x - $oldx}]
        set dy [expr {$y - $oldy}]
        
        #  Move the object etc.:
        
        $c move $options(-id) $dx $dy
        set coords [$c bbox $options(-id)];   #both highlight and title need this.
        # Take care of the outline IF there's already an object move it otherwise
        # draw it.
        
        if {$highlightId ne ""} {
            $c move $highlightId $dx $dy
            $c lower $highlightId
        } elseif {$options(-highlight) ne "white"} {
            set highlightId [$c create rectangle                    \
                $coords -fill $options(-highlight) -outline white   \
            ]
            $c lower $highlightId
        }

        # Take care of the title - as with highlight.
        
        if {$titleId ne ""} {
            $c move $titleId $dx $dy
        } elseif {$options(-title) ne ""} {
            set tx [lindex $coords 0]
            set ty [lindex $coords 3]   #  Lower left corner.
            set titleId [$c create text $tx $ty -text $options(-title) -justify left -anchor nw]
        }
    }
    ##
    # _move
    #   Move the item by changing either x or y.
    #
    # @param optname - option being changed.
    # @param optval  - New proposed value.
    # @note - any title and highlight rectangle will be moved as well.#
    #         We can only move if the -id has been set.
    #
    method _move {optname optval} {
        if {$options(-id) ne ""} {
            set oldx $options(-x)
            set oldy $options(-y)
               
            set options($optname) $optval;
            
            #  Figure out the move:
            
            set dx [expr {$options(-x) - $oldx}]
            set dy [expr {$options(-y) - $oldy}]
            
            set c $options(-canvas)
            
            #  Move the item any title and any highlight:
            
            $c move $options(-id) $dx $dy
            if {$highlightId ne ""} {
                $c move $highlightId $dx $dy
            }
            if {$titleId ne ""} {
                $c move $titleId $dx $dy
            }
        } else {
            set options($optname) $optval
        }
        
    }
    ##
    # _drawtitle
    #    If  title already exists it is destroyed.  A new title is
    #    draw with the text given.  The title is anchored at its upper left
    #    on the lower left corner of the items bounding box.
    #
    # @param opname - name of the option (-title) being modified.
    # @param text   - New title text.
    # @note - can only actually do anything if -id has been established:
    #
    method _drawtitle {optname text} {
       if {$options(-id) ne ""} { 
            set c $options(-canvas)
            
            #  Get rid of any old title:
            
            if {$titleId ne ""} {
                $c delete $titleId
            }
            #  Draw the new one:
            
            set bbox [$c bbox $options(-id)]
            set x [lindex $bbox 0]
            set y [lindex $bbox 3]
            
            set titleId [$c create text $x $y -text $text -anchor nw -justify left]
       }
        set options($optname) $text
    }
    ##
    # _highlight
    #    If there's already a highlight, just change its color.  If not,
    #    create it.  Unhighlighting is just setting the color to white.
    #
    # @param optname - option name.
    # @param color   - New color of the highlight.,
    #
    method _highlight {optname color} {
        set c $options(-canvas)
        
        # If needed create a highlight rectangle.  We'll set the
        # coloration and lower it in the following common code.
        
        if {$highlightId eq ""} {
            set item $options(-id)
            set coords [$c bbox $item];             # rectangle will be bounding box.
            set highlightId [$c create rectangle $coords]
        }
        #  Set the background and outline color then lower the rectangle:
        
        $c itemconfigure $highlightId -outline $color -fill $color
        $c lower $highlightId
        
        
        #  Save the value for later.
        
        set options($optname) $color
    }
}
namespace eval DashboardDatabase {
    
}
##
# getProperty
#   Given an object that has a getProperties method returns the value of a
#   specific property.
#
# @param obj   - The object we want info from
# @param prop  - Named property to fetch.
# @result string - The property value.
#
proc ::DashboardDatabase::getProperty {obj prop} {
    set props [$obj getProperties]
    set propv  [$props find $prop]
    if {$propv eq ""} {
        error "Property $prop does not exist in the object $obj"
    }
    return [$propv cget -value]
}

##
# createObjects
#   Given a list of objects that are dicts with keys object, x, y
#   creates the graphical versions of those objects.
#   Each object is assumed to have a name and host property that are used
#   to label the object.
#
# @param items  - list of objects to create.
# @param canvas - Canvas on which to render them.
# @param icon   - Name of image to use to display them.
# @return list of DashboardObjects that were created. Note that the underlying
#         objects are stored in the -data property of each dashboard object.
#
proc ::DashboardDatabase::createItems {items canvas icon} {
    set result [list]
    set bg     [$canvas cget -background]
    foreach item $items {
        set o [dict get $item object]
        set x [dict get $item x]
        set y [dict get $item y]
    
        set objId [$canvas create image $x $y -image $icon]
        set ditem [DashboardObject %AUTO% -id $objId -canvas $canvas -x $x -y $y]
        $ditem configure -highlight $bg
        set name [getProperty $o name]
        set host [getProperty $o host]
        $ditem configure -title $name@$host
        $ditem configure -data $o
        lappend result $ditem
    }    
            
    
    return $result
    
}

##
# loadRingBuffers
#   Given a database URI that contains an experiment definitions, loads
#   the ring buffers into the specified canvas;
#
# @param dburi -- URI Pointing to the database to read.
# @param canvas -- Canvas on which the rings should be displayed.
# @return list  -  List of resulting DashboardObjects.  The -data value
#                  of each dashboard object is the RingBufferObject
#                  generated from the database definitions.
# @note - the ring buffers will be given a title like name@host.
# @note - A highlight rectangle will be created with the canvas background
#         color (unhighlighted essentially).
#
proc ::DashboardDatabase::loadRingBuffers {dburi canvas} {
    set rings [::Serialize::deserializeRings $dburi]
    return [createItems $rings $canvas RingBufferIcon]
}
##
# loadStatePrograms
#   given a database URI that contains an experiment definition,
#   loads and displays the state sensitive programs as dashboard objects
#   that encapsulates the property container objects for the
#   state programs.
#
# @param dburi - URI pointing to the database to read.
# @param canvas - Canvas on which to display the objects.
# @return list - Dashboard objects generated by the load.
#
proc ::DashboardDatabase::loadStatePrograms {dburi canvas} {
    set programs [::Serialize::deserializeStatePrograms $dburi]
    return [createItems $programs $canvas StateProgramIcon]
}
##
# loadDataSources
#    Loads/displays the data sources from a database experiment def file.
#
# @param dburi - database uri
# @param canvas - canvas on whih to display the stuff.
# @return list  - List of resulting dashboard objects.
# @note  A bit of fudging is done post call to createItems.  The -data
#        values become a two element list containing, in order, the object and
#        the name of the eventbuilder this data source outputs to.
#
proc ::DashboardDatabase::loadDataSources {dburi canvas} {
    set sources [::Serialize::deserializeDataSources $dburi]
    set result [createItems $sources $canvas DataSourceIcon]
    
    #  Fudge the -data back on the result items:
    
    foreach obj $result source $sources {
        set evbName [dict get $source evbName]
        $obj configure -data [list [$obj cget -data] $evbName]
    }
    
    return $result
}
