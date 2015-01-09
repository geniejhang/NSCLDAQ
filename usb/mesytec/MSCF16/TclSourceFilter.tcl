

package provide TclSourceFilter 1.0
package require BlockCompleter

snit::type TclSourceFilter {

  variable _validPatterns


  ## @brief Construct and set up the state
  #
  # @param presenter    an MCFD16ControlPanel instance
  # @param args         option-value pairs
  #
  constructor {args} {
    set _validPatterns [list]
  }
  
  method SetValidPatterns {patterns} {
    set _validPatterns $patterns
  }

  ##
  # @param  path    the path to a file containing state 
  method Filter {script} {
    # split the file into complete chunks that can be passed to eval
    set rawLines [$self Tokenize $script]

    # find the lines we can safely execute
    set executableLines [$self FindPatternMatches $rawLines]

    return $executableLines
  }

  ## @brief Split the file into fully executable chunks
  #
  # This uses the BlockCompleter snit::type to find all complete blocks (i.e. at
  # the end of a line the number of left and right curly braces are equal). The
  # list of complete blocks are returned.
  #
  # @param path   the path to the file to parse
  #
  # @returns  the list of complete blocks 
  method Tokenize {contents} {

    set blocks [list]
    BlockCompleter bc -left "{" -right "}"
 
    set lines [split $contents "\n"]

    set index 0
    set nLines [llength $lines]
    while {$index < $nLines} {
      bc appendText [lindex $lines $index]
      incr index
      while {![bc isComplete] && ($index < $nLines)} {
        bc appendText "\n[lindex $lines $index]"
        incr index
      }
      lappend blocks [bc getText]
      bc Reset
    }

    bc destroy
    return $blocks
  }

  ## @brief Check to see if second element is an API call
  #
  # In a well-formed call to a device driver, the first element will be the name
  # of the device driver instance and the second element of the line will be the
  # actual method name. If the second element is not a string that is understood
  # to be a valid method name, then we return false. That is only if that line
  # is also not recognized as a command to manipulate the names.
  # 
  # @param  lines   the list of lines to filter
  #
  # @returns a list of lines that only contain  valid api calls or name
  # manipulation code
  method FindPatternMatches blocks {
    set validLines [list]
    foreach line $blocks {
      if {[$self IsPatternMatch $line]} {
        lappend validLines $line
      }
    }
    return $validLines
  }

  ## @brief Check whether the second element of a line is a valid api call
  #
  # This snit::type provides a list of strings that it considers valid API
  # calls. If the second element of the line is in this list, then it is
  # considered an api call.
  #
  # @param line   line of code to check
  #
  # @returns boolean
  # @retval 0 - second element is not in list of valid calls
  # @retval 1 - otherwise
  method IsPatternMatch {line} {
    set found 0
    foreach patt $_validPatterns {
      set found [regexp $patt $line]
      if {$found} break
    }

    return $found
  }

}
