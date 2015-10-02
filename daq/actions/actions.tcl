# actions.tcl
#
# A tcl package to handle the parsing of a basic messaging protocol
# to allow communication between devices in a pipeline and the ReadoutGUI.
#
# Things that need to be fixed. 
#   - Need to fix the leaking of the process.
#   - sometimes a newline is prepended to an output and that needs to be 
#     fixed.
#


package provide Actions 1.0
package require snit


snit::type Actions {
  
  variable accumulatedInput "" 
  variable accumulatedOutMsg "" 
  variable incomplete 0 
  variable errors [dict create 0 " unable to parse directive"]
  variable legalDirectives {ERRMSG LOGMSG WRNMSG TCLCMD OUTPUT DBGMSG}
  variable directiveMap { ERRMSG 0 LOGMSG 1 WRNMSG 2 
                          TCLCMD 3 OUTPUT 4 DBGMSG 5} 

  option -actionbundle -default DefaultActions

  constructor {args} {
    $self configurelist $args
  }

  method onReadable {fd} {

    if { [eof $fd] } {
      # unregister itself
      chan event $fd readable ""

      # we have reached an end of file
      #
      # convert to blocking to retrieve the exit status
      chan configure $fd -blocking 1

      if {[catch {close $fd} msg]} {
        if {[lindex $::errorCode 0] eq "CHILDSTATUS"} {
          puts stderr "Child process exited abnormally with status: $::errorCode"
        }
      }
    } else {
      $self handleReadable $fd 
    }
  }

  method getLine {} { return $accumulatedInput }
  method setLine {str} { set accumulatedInput $str }

  method handleReadable {fd} {

    # read what the channel has to give us
    set input [chan read $fd ]

    return [$self processInput $input]
  }


  #
  # We need to be careful about not processing the last line of the message
  # in case it did not end in a newline character. By not ending in a newline
  # character, that means we do not have a complete message and should
  # adjust the number of lines to process so that the last line is excluded.
  method computeNLinesToProcess {nLines strippedNewline firstLine} {
    if {(! $strippedNewline)} {
      #puts "did not strip newline"
      #puts "nLines=$nLines, firstLine=\"$firstLine\", isLegalDir=[$self isLegalDirective [$self extractFirstWord $firstLine]]"
      if {($nLines != 1) 
          || !([$self isLegalDirective [$self extractFirstWord $firstLine]]) 
          || ([string length $firstLine]<6)} {
        #puts "adjusting number of lines"
        incr nLines -1
      }
    }

    return $nLines
  }


  # This has become embarrassingly complicated...
  method processInput input {
    #puts "processing input: \"$input\""

    if {$incomplete} {
      #puts "last was incomplete and we start with : \"$accumulatedInput\""
      set input "[lindex $accumulatedInput end]$input"
      set accumulatedInput {}
    }

    set strippedNewline 0
    set result {}

    # pop a single newline character off of the end if it exists to avoid 
    # an empty line at the end caused by the split command
    if {[string index $input end] eq "\n"} {
      set input [string range $input 0 end-1]
      set strippedNewline 1
      #puts "stripped the newline!"
    }

    set accumulatedInput [concat $accumulatedInput [split $input "\n"]]
    #puts "accumulatedInput: \"$accumulatedInput\""

    # we need to iterate over all lines that are present at the start
    # so we will iterate over a copy of accumulatedInput because 
    # the accumulatedInput list gets manipulated as we go along
    set linesToProcess $accumulatedInput
    set nLines [llength $linesToProcess]
    if {$nLines == 0} {incr nLines}
    set firstLine [lindex $linesToProcess 0]

    # adjust the number of lines to process in case we believe the last line is incomplete
    set nLinesToProcess [$self computeNLinesToProcess $nLines $strippedNewline $firstLine]
    #puts "nLines: $nLines, nLinesToProcess: $nLinesToProcess"

    # process the lines of data
    for {set index 0} {$index<$nLinesToProcess} {incr index} {

      set line [lindex $linesToProcess $index]


      # correct for the removal of newlines during the split command
      if {($index == [expr $nLines-1]) && !$incomplete} {
        # if we are processing all of the original lines, then that implies the
        # message we are working with is considered a complete message
        append accumulatedOutMsg "$line"
      } else {
        # because we are here, we are in the middle of a message, and it is possible
        # that we had newlines in the middle of the message. Make sure to add back the newline character
        #that 
        append accumulatedOutMsg "$line\n"
      }

      # pop off the current line from accumulatedInput
      set accumulatedInput [lreplace $accumulatedInput 0 0]

      #puts "line: \"$line\", accumulatedOutMsg: \"$accumulatedOutMsg\", input: \"$accumulatedInput\", incomplete=$incomplete"

      # if the first word is a legal directive then we will handle the 
      # message as a packet, otherwise it is some other entity 
      set firstWord [$self extractFirstWord $accumulatedOutMsg]
      if {[$self isLegalDirective $firstWord]} {
        #puts "Found a directive"
        set parsedLine [$self buildPacket $accumulatedOutMsg]
        if {$parsedLine ne ""} {
          set incomplete 0
          lappend result $parsedLine
          set accumulatedOutMsg {}
        } ;# else move to next line and handle it
      } else {
        #puts "Found a non packet"
        set parsedLine [$self handleNonPacket $accumulatedOutMsg]
        if {$parsedLine ne {}} {
          lappend result $parsedLine
          if {[lindex $parsedLine 2] == [string length $accumulatedOutMsg]} {
            set accumulatedOutMsg {}
          } else {
            set accumulatedOutMsg [string range $accumulatedOutMsg [lindex $parsedLine 2] end]
          }
        }
      } 
    } ;# end of for loop


    # handle the messages!
    set incomplete [expr !$strippedNewline]
    #puts "result: \"$result\""
    set retval {}
    foreach msg $result {
      lappend retval [$self handleMessage $msg]
    }
    return $retval
  }

  method extractFirstWord {sentence} {
    return [string range $sentence 0 5]
  }

  # The first word was detected as a legal directive so 
  # we expect that there is a well formed packet. Try to
  # read the whole thing. If the full packet isn't there,
  # return a null string and move on. 
  # If the packet is found, truncate "line" so that the 
  # packet is no longer being outputted.
  method buildPacket {content} {

    set incomplete 1

    # find first and second word boundaries 
    set b1 [string first { } $content 0]
    if {$b1 == -1} return
    set b2 [string first { } $content [expr $b1+1]]
    if {$b2 == -1} return
    
    set pktSize [string trim [string range $content $b1 $b2] { \n}]
    set totalLength [string length $content]
    set remChars [expr $totalLength - ($b2+1)]

    if {$remChars >= $pktSize} {
       set b3 [expr $b2+$pktSize]
       lappend parsedLine [$self extractFirstWord $content] 
       lappend parsedLine [string range $content [expr $b2+1] $b3] 
       lappend parsedLine [string range $content [expr $b1+1] [expr $b2-1]] 

       set incomplete 0
       set content [string range $content [expr $b3+1] end] 
      
       return $parsedLine
    } else {
       return ""
    }

  }

  # Deal with non packet output... we simply output
  # everything we have up until a valid directive is 
  # found 
  # if we find a directive, output everything up to that
  # directive, pop the outputted msg from the front of line,
  # return. 
  method handleNonPacket {content} {
  
    # handle the scenario when the line contains a directive somewhere
    # in it
    foreach dir $legalDirectives {
      set index [string first $dir $content]
      if {$index != -1} {
        # we found a directive, output everything up to that directive
        set msg [string range $content 0 [expr $index-1]]
        set content [string range $content $index end]

        return  [list OUTPUT $content [string length $content]]
      }
    }

    # if we are here then we didn't find any directives, output the whole line
    set result [list OUTPUT $content [string length $content]]
    return $result
  } 

  # Determine if there word is a legal directive
  method isLegalDirective {word} {
    return [expr {$word in $legalDirectives}]
  }

  # Jump-table of sorts for passing various 
  # handlers to their handlers
  method handleMessage {parsedLine} {

    set directive [lindex $parsedLine 0]
    set msg [lindex $parsedLine 1]
    set dirId [dict get $directiveMap $directive]
    set result {}
    switch $dirId {
      0 { set result [$self handleError $msg ] }
      1 { set result [$self handleLog $msg ]}
      2 { set result [$self handleWarning $msg ] }
      3 { set result [$self handleTclCommand $msg ]}
      4 { set result [$self handleOutput $msg] }
      5 { set result [$self handleDebug $msg] }
    } 

    return $result
  }

  method handleError {str} {
    return [$options(-actionbundle) handleError $str]
  }

  method handleLog {str} {
   return [$options(-actionbundle) handleLog $str]
  }

  method handleWarning {str} {
    return [$options(-actionbundle) handleWarning $str]
  }

  method handleDebug {str} {
    return [$options(-actionbundle) handleDebug $str]
  }

  method handleOutput {str} {
    return [$options(-actionbundle) handleOutput $str]
  }

  method handleTclCommand {str} {
    return [$options(-actionbundle) handleTclCommand $str]
  }
}

