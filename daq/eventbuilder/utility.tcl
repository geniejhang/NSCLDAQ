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
#             created.
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
    option -title      -default ""
    option -lefttitle  -default ""
    option -righttitle -default ""
}