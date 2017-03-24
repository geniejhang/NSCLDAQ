#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#    Jeromy Tompkins
#	   NSCL
#	   Michigan State University
#	   East Lansing, MI 48824-1321

package provide Utils 1.0

namespace eval Utils {

  ## @brief Test that a value falls with a certain range
  #
  # The range specified is inclusive such that the condition to be tested is
  # low <= val <= high.
  #
  # @param low  lower bound of range 
  # @param high upper bound of range 
  # @param val  value to test 
  #
  # @returns boolean indicating whether value is within the defined range
  proc isInRange {low high val} {
    return [expr {($val >= $low) && ($val <= $high)}]
  }

  ## @brief Check that all of the elements in a list fall within a range
  #
  # This iterates through the list and checks each element for the following
  # condition: low <= element <= high. The algorithm begins at the beginning of
  # the list and keeps checking until either an element is identified that does
  # not satisfy the condition or the end of the list is reached.
  #
  # @param low  lower bound
  # @param high upper bound
  # @param list list of values
  #
  # @returns boolean 
  # @retval 0 - at least one element in list is outside of range
  # @retval 1 - all elements fall within range
  proc listElementsInRange {low high list} {

  # this is innocent until proven guilty
    set result 1 

    # if an element is out of range, flag it and stop looking.
    foreach element $list {
      if {($element<$low) || ($element>$high)} {
        set result 0
        break
      }
    }

    return $result
  }

  ##
  #
  #
  proc sequence {start n {inc 1}} {
    set res [list]
    for {set i 0} {$i<$n} {incr i} {
      lappend res [expr $start+$i*$inc]
    }
    return $res
  }
}
##
# fqdn
#    Given a DNS hostname which is, potentially not fully qualified,
#    returns its fully qualified hostname.  The hsotname must be
#    resolvable with nslookup which must be installed in the user's path on
#    the host system.
#
# @param host  - Name of the host to fully qualify.
# @return string - the hosts fqdn.
# @throw Errors if:
#        *  nslookup is not installed.
#        *  The host parameter is not resolveable.
#  @note acknowledgements to Tom Wilkason who produced a script
#        very much like this in
#        http://computer-programming-forum.com/57-tcl/e186ce2a504ea1b8.htm
#
# @note nslookup is used because that's available on windows as well as unix
#       like systems while host is available on unix like systems only(?).
#

proc fqdn host {
   foreach line [split [exec nslookup $host] \n] { 
      foreach {title host} [split $line :] { 
         if {$title == "Name"} { 
            return [string trim $host] 
         } 
      } 
   } 
   error "$host cannot be looked up by nslookup command."
  
}