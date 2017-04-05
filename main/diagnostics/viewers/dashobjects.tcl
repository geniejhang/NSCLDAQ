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
package require eventBuilderObject
package require evbSerializer

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
##
# loadEventBuilders
#    Loads/displays event builders from a database experiment def file.
#
# @param dburi    - URI of the database file.
# @param canvas   - Canvas on which to draw elements.
# @return list   - Of dashboard objets created.
#
proc ::DashboardDatabase::loadEventBuilders {dburi canvas} {
    set evbs [::Serialize::deserializeEventBuilders $dburi]
    return [createItems $evbs $canvas EventBuilderIcon]
}
##
# _inBBox
#   Given a point determines if it's inside a bounding box.
#
# @param bbox - bounding box in window coordinates
# @param pt   - x/y pt.
#
proc ::DashboardDatabase::_inBBox {bbox pt} {
    set ulx [lindex $bbox 0]
    set uly [lindex $bbox 1]
    set lrx [lindex $bbox 2]
    set lry [lindex $bbox 3]
    
    
    set x [lindex $pt 0]
    set y [lindex $pt 1]
    
    return [expr {($x >= $ulx)        &&                          \
                  ($x <= $lrx)        &&                          \
                  ($y >= $uly)        &&                          \
                  ($y <= $lry)                                    \
    }]
}

##
# _vertical
#    Given a pair of bounding boxes that are exactly vertical
#    relative to each other, compute the line segment that joins them.
#    this is the shortest of the segments between the midpoints of the top of
#    bbox1 and bottom of box2 or the bottom of bbox1 and top of bbox2.
#    length is just difference in y here so we can compute that first
#
proc ::DashboardDatabase::_vertical {bbox1 bbox2} {

    set ulx1 [lindex $bbox1 0];       # Xcoord of upper left.
    set uly1 [lindex $bbox1 1];       # and its y.
    set lrx1 [lindex $bbox1 2];       # lower right
    set lry1 [lindex $bbox1 3];
    
    set uly2 [lindex $bbox2 1];      # verticality implies we don't need x. 
    set lry2 [lindex $bbox2 3];
    
    set x [expr {($ulx1 + $lrx1)/2}]
    
    
    #  What matters is whichi s above the other (assume no vert. overlap):
    
    if {$uly1 > $lry2} {
        #  bbox1 is below bbox2
        
        set y1 $uly1
        set y2 $lry2
        
    } else {
        # bbox1 is above bbox2
        
        set y1 $lry1
        set y2 $uly2
    }
    
    return [list $x $y1 $x $y2]
    
}
##
# _intersection
#   Given a line from or to the interior of a bounding box, returns the intersection
#   point of that line with the bounding box.
#
# @param lseg   begin/end points of the line.
# @param bbox   Bounding box points.
#
proc ::DashboardDatabase::_intersection {lseg bbox} {
    
    set ulx [lindex $bbox 0]
    set uly [lindex $bbox 1]
    set lrx [lindex $bbox 2]
    set lry [lindex $bbox 3]
    
    # Compute the line formula y as a function of x:
    
    set x1 [lindex $lseg 0]
    set y1 [lindex $lseg 1]
    set x2 [lindex $lseg 2]
    set y2 [lindex $lseg 3] 
    
    set m [expr {double($y2-$y1)/($x2 - $x1)}]
    set b [expr {$y1 - $m*$x1}]
    set u  {$m*$x + $b};             #Defered evaluation of y = mx+b
    
    # Try for intersection with vertical segments of bbox.
    set x $ulx
    set y [expr $u];                # m,x,b get substituted after u is expanded.

    if {($y >= $uly) && ($y <= $lry)  &&                      \
        ($x >= min($x1, $x2))  && ($x <= max($x1,$x2))} {
        return [list $x $y]
    }
    set x $lrx
    set y [expr $u]

    if {($y >= $uly) && ($y <= $lry)      &&                      \
        ($x >= min($x1, $x2))  && ($x <= max($x1,$x2))} {
        return [list $x $y]
    }
    # Figure out x as a function of y so we can try the horizontal segments.
    
    
    set u {($y - $b)/$m}
    set y $uly
    set x [expr $u]
    
    if {($x >= $ulx)  && ($x <= $lrx)               &&                   \
        ($y >= min($y1, $y2))  && ($y <= max($y1,$y2))} {
        return [list $x $y]
    }
    
    set y $lry
    set x [expr $u]
    
    if {($x >= $ulx)  && ($x <= $lrx)               &&                   \
        ($y >= min($y1, $y2))  && ($y <= max($y1,$y2))} {
        return [list $x $y]
    }
    # Given the geometry there must be an intersection so:
    
    error "Bug of some sort as bounding box intersection for $lseg with $bbox not found"
    
}

##
# _reduceConnection
#    Given a pair of bounding boxes  and a line segment L-R direction, determine 
#    a new pair of start/stop points for the
#    for the line segment that lies on the boundary boxes.  The intent of this is to
#    compute new line segments that connect bounding boxes of objects at nearest points
#    This is done by computing the intersection points of the line with the bounding
#    box segments and only keeping the points that live in the segment.  I think this
#    works because a line segment from the center of one object to the center of
#    another must intersect the bounding box of each object on one of its lines
#    (corner intersection case is handled by allowing line segments to include
#    their end points and just picking the first 'good' intersection).
#
# @param lseg   - Line segment that joins the two objects [list [list x1 y1] [list x2 y2]]
# @param bbox1  - Bounding box of one of origin box.
# @param bbox2  - Bounding box of the destination box.
# @return reduced line segment in the same orientation.
# @note There's an assumption the bounding boxes don't overlap.
#
proc ::DashboardDatabase::_reduceConnection {lseg bbox1 bbox2} {
    
    # pull out the segment endpoints:
    
    set lx1 [lindex $lseg 0]
    set ly1 [lindex $lseg 1]
    
    set lx2 [lindex $lseg 2]
    set ly2 [lindex $lseg 3]
    
    #  Special case.  Vertical lines just go through the midpoint of the nearest
    #  bounding box line segments of the two items.
    
    if {$lx1 == $lx2} {
        return [_vertical $bbox1 $bbox2]
    }
    # All other cases do the intersection game:
    
    
    set pt1 [_intersection $lseg $bbox1]
    set pt2 [_intersection $lseg $bbox2]
    
    return [concat $pt1 $pt2]
}
##
# _drawConnector
#   Given the ids of a from and to graphical object on a canvas draws a line
#   connecting the nearest two points on the object's bounding boxes.
#
# @param canvas - Canvas on which the connector will be drawn.
# @param id1    - Connector is drawn from this id.
# @param id2    - Connector is drawn to (arrow at) this id.
#
proc ::DashboardDatabase::_drawConnector {canvas id1 id2} {
    # Figure out the centers of each bounding box;
    
    set bbox1 [$canvas bbox $id1]
    set bbox2 [$canvas bbox $id2]
    
    set x1 [expr {([lindex $bbox1 2] + [lindex $bbox1 0]) /2.0}]
    set y1 [expr {([lindex $bbox1 3] + [lindex $bbox1 1]) /2.0}]
    
    set x2 [expr {([lindex $bbox2 2] + [lindex $bbox2 0]) /2.0}]
    set y2 [expr {([lindex $bbox2 3] + [lindex $bbox2 1]) /2.0}]
    
    
    set resultingCoords [_reduceConnection [list $x1 $y1 $x2 $y2] $bbox1 $bbox2]
    
    
    $canvas create line $resultingCoords -arrow last 
}
##
# _uriToFqrn
#   Convert a URI of the form tcp://hostname/ringname to @ form:
#   ringname@hostname
#
# @param uri - A known good URI of the from tcp://hostname/ringname.
#
proc ::DashboardDatabase::_uriToFqrn {uri} {
    scan $uri tcp://%s hostAndRing
    set ringInfo [split $hostAndRing /]
    set ringInfo [list [lindex $ringInfo 1] [lindex $ringInfo 0]]
    return [join $ringInfo @]
}

##
# _connectRingsAndPrograms
#   Given the rings and the state programs draws directed lines between the
#   each source ring and its program and between each program and its output ring.
#
# @param canvas - Canvas on which items will be drawn.
# @param rings  - Ring buffer dashboard objects.
# @param statepgms - The state programs.
#
proc ::DashboardDatabase::_connectRingsAndPrograms {canvas rings statepgms} {
    #
    #  First lets toss the ring canvas ids up in an array indexed by name:
    #
    array set ringObjects [list]
    foreach ring $rings {
        set ringObjects([$ring cget -title]) [$ring cget -id]
    }
    #
    #  Now  reform the connections:
    #
    foreach program $statepgms {
        set stateObj [$program cget -data]
        set iring     [getProperty $stateObj "Input Ring"]
        set oring     [getProperty $stateObj "Output Ring"]
        set pgmHost   [getProperty $stateObj host]
        set pgmId     [$program cget -id]
        
        # Connect program to output ring:
        
        if {$oring ne ""} {
            _drawConnector $canvas $pgmId $ringObjects($oring@$pgmHost)
        }
        #  Connect input ring to program:
        
        if {$iring ne ""} {
            set ringName [_uriToFqrn $iring]
            _drawConnector $canvas $ringObjects($ringName) $pgmId
        }
    }
}
##
# _connectRingsAndSources
#    Data sources have as sources ring buffers.  Connect their ring buffers
#    to them.
#
# @param canvas   - The canvas on which we draw the connectors
# @param rings    - list of ring buffer dashboard objects.
# @param dsources - list of data source dashboard objects.
#
proc ::DashboardDatabase::_connectRingsAndSources {canvas rings dsources} {
    
    #  Toss the ids for the data sources into an array indexed by the
    #  ring buffer's name@host:
    
    array set sources [list]
    foreach source $dsources {
        set o [lindex [$source cget -data] 0]
        set id [$source cget -id]
        set ringUrl [getProperty $o ring]
        set ring [_uriToFqrn $ringUrl]
        set sources($ring) $id
    }
    #  Now connect each ring to a data source if there is a target:
    
    foreach ring $rings {
        set name [$ring cget -title]
        if {[array names sources $name] ne ""} {
            _drawConnector $canvas [$ring cget -id] $sources($name)
        }
    }
}
##
# _connectSourcesToBuilders
#    Connects data sources to event builders.
#
# @param canvas   - Canvas on which connectors are drawn.
# @param sources  - Data source dashboard objects.
# @param builders - Event builder dashboard objects.
#
proc ::DashboardDatabase::_connectSourcesToBuilders {canvas sources builders} {
    # Push the event builder ids up in a name indexed array:
    
    array set builderids [list]
    foreach builder $builders {
        set o [$builder cget -data]
        set id [$builder cget -id]
        set name [getProperty $o name]
        set builderids($name) $id
    }
    
    #  Connect each source:
    
    foreach source $sources {
        set evbname [lindex [$source cget -data] 1]
        set id      [$source cget -id]
        _drawConnector $canvas $id $builderids($evbname)
    }
}

##
# _connectBuildersToRings
#   Connect event builders to their output ring buffers.
#
# @param canvas   - Canvas on which connectors should be drawn.
# @param builders - Event builders in the system.
# @param rings    - Ring buffers in the system.
#
proc ::DashboardDatabase::_connectBuildersToRings {canvas builders rings} {
    # push the ring ids into an array indexed by name@host
    
    array set ringids [list]
    foreach ring $rings {
        set name [$ring cget -title]
        set ringids($name) [$ring cget -id]
    }
    #  Now connect each event builder to its ring:
    
    foreach builder $builders {
        set id [$builder cget -id]
        set o  [$builder cget -data]
        set ringname [getProperty $o ring]
        set host [getProperty $o host]
        
        _drawConnector $canvas $id $ringids($ringname@$host)
    }
}

##
# connectObjects
#   Form connections between objects.  Specifically State programs connect
#   to rings (in and out).  Data sources take inputs from rings and outputs go to
#   event builders while event builders output to rings.
#
# @param canvas   - Canvas on which to draw connectors.
# @param rings    - List of ring buffer dashboard objects.
# @param statepgms- List of state sensitive program objects.
# @param dsources - List of data sources.
# @param evbs     - Event builders.
#
proc ::DashboardDatabase::connectObjects {canvas rings statepgms dsources evbs} {
    _connectRingsAndPrograms  $canvas $rings $statepgms
    _connectRingsAndSources   $canvas $rings $dsources
    _connectSourcesToBuilders $canvas $dsources $evbs
    _connectBuildersToRings   $canvas $evbs $rings
}

##
# load
#    Loads an experiment from the database:
#
# @param uri  - The uri of the database file.
# @param canvas - The canvas to draw the experiment on.
# @return List containing the list of rings, state programs data sources and event builders
#
proc ::DashboardDatabase::load {uri canvas} {
    set rings [DashboardDatabase::loadRingBuffers $uri $canvas]
    set pgms  [DashboardDatabase::loadStatePrograms $uri $canvas]
    set sources [::DashboardDatabase::loadDataSources $uri $canvas]
    set builders [::DashboardDatabase::loadEventBuilders $uri $canvas]

    connectObjects $canvas $rings $pgms $sources $builders
    
    return [list $rings $pgms $sources $builders]
    
}