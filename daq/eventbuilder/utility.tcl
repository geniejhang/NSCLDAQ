#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file utility.tcl
# @brief misc. EVB utility methods/snidgets etc.
#


#-----------------------------------------------------------------------------
# Package stuff
#

package provide EVBUtilities 1.0

package require Tk
package require snit
package require Iwidgets

#------------------------------------------------------------------------------
#
#  Establishing namespaces
#
#
namespace eval EVB {
    namespace eval utility {}
    namespace eval test    {}
}


#------------------------------------------------------------------------------
# Useful snidgets

##
# @class sortedPair
#
#  This class maintains  a sorted pair of numbers. The left column is sorted
#  ascending while the right column is assumed to be some associated data
#  typical use is as a set of counters with some identifier (e.g. source id or
#  fragment type) as the sort key.
#
# OPTIONS
# - -title     -   Overall Title.
# - -lefttitle -  Title of the left column
# - -righttitle - Title of the right column
#
# METHODS
#   - setItem key value sets a key/value pair.  If necessary the key's widgets are
#     created.
#   - clear value=0 All of the values are set to the new value.
#   - reinit All key/value pairs are deleted.
#
# LAYOUT
#
#  +---------------------------------------+
#  |             top title                 |
#  |   Left title        Right Title       |
#  |         id                value      ^|
#  |      ...                   ...       V|
#  +---------------------------------------+
#
snit::widget EVB::utility::sortedPair {
    component innerHull
    component container

    option -lefttitle  -default ""
    option -righttitle -default ""

    delegate option -title to innerHull as -text
    delegate option *      to innerHull

    variable container

    # List of id/value pairs.  The widgets 
    # associated with it are of the form
    # $container.${id}id and $container.${id}value
    #
    # The list is maintained sorted by id.
    #
    variable values  [list]

    ##
    # constructor
    #
    #  Create and layout the widgets.  Since snit does not 
    #  support ttk::frame hulls, innerHull is a ttk::labelframe
    #  that is pasted to fill the hull and acts like a hull
    #  for everything else.
    #  
    # The left title and right title are then gridded
    # at the top of the frame and a scrolledframe
    # is used to hold the ttk::text  items that
    # make up the individual id/value pairs.
    #
    # 
    constructor args {
	#
	# Create the inner hull, the label will be
	# centered at the top:
	
	install innerHull using ttk::labelframe $win.hull \
	    -labelanchor n

	# The top title widgets .. can be gridded now.

	ttk::label $innerHull.lefttitle \
	    -textvariable ${selfns}::options(-lefttitle) \
	    -width 10
	ttk::label $innerHull.righttitle  \
	    -textvariable ${selfns}::options(-righttitle) \
	    -width 10

	grid $innerHull.lefttitle $innerHull.righttitle

	# Now the scrolled frame that will hold
	# the widgets...it's glued to the entire
	# remainder of the lable frame:

	iwidgets::scrolledframe $innerHull.sf \
	    -hscrollmode none -vscrollmode dynamic
	grid $innerHull.sf -columnspan 2 -sticky nsew

	set container [$innerHull.sf childsite]

	grid $innerHull -sticky nsew
       
	# Process options:

	$self configurelist $args


    }
    ##
    # setItem id value
    #
    #  If necessary creates a new entry for id both 
    #  internally and in the widget set.
    #  the new widgets are inserted so that they
    #  remain sorted by id in the scrolled frame.
    #  If the id already exists, it's value widget
    #  is just modified.
    #
    #  @param id - The id of the element (key).
    #  @param value - The new value it should have
    #
    method setItem {id value} {
	set widgets [$self _getWidgets $id];  # making if needed
	$self _updateList $id $value;	     # update values

	set vWidget [lindex $widgets 1]
	$vWidget config -text $value;        # update display. 
    }
    ##
    # clear ?value?
    #
    #  Reset all values to value or zero if not supplied.
    #
    # @param value - the new value to which all value widgets
    #                will be set.
    #
    method clear {{value 0}} {
	foreach item $values {
	    set id [lindex $item 0]
	    $self setItem $id $value
	}
    }
    ##
    # reinit
    #
    #  Destroys all id/value pairs in both the internal
    #  representation and the widgets themselves.
    #
    method reinit {} {
	foreach item $values {
	    set widgets [$self _getWidgets [lindex $item 0]]
	    destroy [lindex $widgets 0]
	    destroy [lindex $widgets 1]
	}
	set values [list]
    }
    #----------------------------------------------
    #  Private methods.

    ##
    # _getWidgets id
    #  
    #  Gets the widget names associated with an id
    #  if necessary, new widgets are created
    #  if that is needed, placeholder values are
    #  also created in the values list.
    #
    # @param id - The key we are looking up.
    #
    # @return list (2 elements)
    # @retval the first list element is the
    #         widget for the id the second for its value.
    #
    method _getWidgets id {

	# If needed create a new widget pair

	set listIndex [lsearch -index 0 -integer -exact \
			   $values $id]
	if {$listIndex == -1} {
	    $self _createWidgets $id
	}
	# Compute the widgets:
	
	return [$self _widgetNames $id]
    }
    ##
    # _updateList id value
    # 
    # Update the 'values' list with a new value for an
    # index.  The caller must ensure that the pair for the
    # id already exists (see _createWigets below).
    #
    # @param id - The id whose value will be modified.
    # @param value - The new value associated with the id. 
    method _updateList {id value} {
	set listIndex [lsearch -index 0 -integer -exact \
			   $values $id]
	set values [lreplace $values $listIndex $listIndex \
			[list $id $value]]
    }
    ##
    # _createWidgets id
    #
    #   - Creates the widgets associated with id,
    #   - Puts a placeholder value for the id
    #     in the appropriate place in the values list.
    #   - Puts them in the container in the appropriate place
    #
    # @param id - The identifier;  the caller must have
    #             ensured this id has neither values
    #             list elements nor widgets.
    #
    method _createWidgets id {
	set widgets [$self _widgetNames $id]; # Just the names

	# Figure out where to put new placeholder in the
	# list. We insert a placeholder just before
	# the first list element that is larger than us
	# else we append it to the list.
	# The index is memorized as that will be the 
	# row number at which the new widgets will be gridded.
	#
	# The place holder just has an empty
	# value:

	set placeholder [list $id ""]
	set insertIndex 0
	foreach item $values {
	    set elementId [lindex $item 0]
	    if {$elementId > $id} {
		break
	    }
	    incr insertIndex
	}
	set values [linsert $values $insertIndex $placeholder]

	# Tell the gridder to forget all of the
	# widgets that are after us in the list:

	foreach item [lrange $values \
			  [expr $insertIndex + 1] end] {
	    set unpasteId [lindex $item 0]
	    set unpasteWids [$self _widgetNames $unpasteId]
	    grid forget [lindex $unpasteWids 0]
	    grid forget [lindex $unpasteWids 1]
	}
	# create the new widgets, grid them and all the
	# others we just ungridded:

	ttk::label [lindex $widgets 0] -text $id \
	    -width 10
	ttk::label [lindex $widgets 1] -text "" \
	    -width 10

	foreach item [lrange $values $insertIndex end] {
	    set itemId [lindex $item 0]
	    set itemWidgets [$self _widgetNames $itemId]

	    grid [lindex $itemWidgets 0] \
		[lindex $itemWidgets 1] -row $insertIndex

	    incr insertIndex
	}


    }
    ##
    # _widgetNames id
    #
    # returns a list of the widget names associated with
    # the specified id.  The widget names are computed
    # but there is no assurance the widgets actually
    # exist yet.  That's a job for the caller to manage.
    # 
    # @param id - the id for which to compute the widget
    #             names.
    #
    # @return 2 element list of id value widget names
    method _widgetNames id {
	set idWidget $container.${id}id
	set valueWidget $container.${id}value
	return [list $idWidget $valueWidget]
    }	
}
