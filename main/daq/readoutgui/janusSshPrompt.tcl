#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2024.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Original Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321
#
#    Author:
#            Genie Jhang
#            FRIB
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file janusSshPrompt.tcl
# @brief Prompter for parameters for the SSHPipe provder for JanusC.
# @author Genie Jhang <changj@frib.msu.edu>

package provide JanusSSHPipe_Prompter 1.0

##
#  The prompter is going to get defaults from a set of configuration parameters
#  which have some defaults as well as environment variables as follows:
#
#   | variable          | Env variable   |  Default  |  Meaning               |
#   ------------------- | -------------- | --------- | ---------------------- |
#   | JanusSSHPipeHost  | DAQHOST        | localhost | Host readout runs on   |
#   | JanusSSHProgram   | RDOFILE        | ""        | Readout executable     |
#
#
#  These definitions are consistent with the 10.x definitions.
#
package require JanusSSHPipe_Provider
package require Configuration
package require Tk
package require DataSourceUI
package require snit
package require dialogwrapper
package require textprompter

# Make sure the provider namespace has been established:

namespace eval ::JanusSSHPipe {}

##
# ::JanusSSHPipe::setParamDefaults
#
#   Sets the default parameter value.  See the table above.
#
proc ::JanusSSHPipe::setParamDefaults {} {
    Configuration::Set JanusSSHPipeSourceID "0"
    Configuration::Set JanusSSHPipeHost     "localhost"
    Configuration::Set JanusSSHPipePort     "41234"
}
##
#  ::JanusSSHPipe::paramEnvironmentOverrides
#
#  Apply any environment variables that override the defaults.
#
proc ::JanusSSHPipe::paramEnvironmentOverrides {} {
}

#
#  Initialize the configuration parameters when the package is loaded:

::JanusSSHPipe::setParamDefaults
::JanusSSHPipe::paramEnvironmentOverrides

#----------------------------------------------------------------------------
#
#  Graphical prompting


##
# @class ::JanusSSHPipe::ParameterPromptDialog
#
# Dialog that prompts for the parameters needed for an SSHPipe data source.
# The parameters we provide are the host, the program path and optional
# parameters passed to the program.
#
# LAYOUT:
#    +--------------------------------------------------------------------+
#    | Client Script: [ entry widget ]  <Browse...>                       |
#    |   Source ID:   [ entry widget ]                                    |
#    | Server Host:   [ entry widget ]                                    |
#    | Server Port:   [ entry widget ]                                    |
#    | Output Ring:   [ entry widget ]                                    |
#    +--------------------------------------------------------------------+
#    | <Ok>   <Cancel>                                                    |
#    +--------------------------------------------------------------------+
#
# OPTIONS:
#   -program       - Absolute path of client python script
#   -janussourceid - Source ID to use for the Janus datasource
#   -host          - Host name the client script connects to
#   -port          - Port number the client script connects to
#   -ring          - Output RingBuffer name
#
# METHODS
#   modal    - Block until either OK or Cancel has been clicked (or the dialog
#              destroyed).  [delegated to a DialogWrapper object].
#
snit::widgetadaptor ::JanusSSHPipe::ParameterPromptDialog {
    component wrapper
    component form
    
    option -program
		option -janussourceid -default 0
    option -host          -default localhost
    option -port          -default 41234
		option -ring
    
    delegate method modal to wrapper
    
    ##
    # constructor
    #   *  Install a top level as the hull.
    #   *  set default values for -host, -program  -parameters.
    #   *  Install a DialogWrapper as wrapper
    #   *  Ask the wrapper for the frame in which we build the prompt elements
    #   *  Build and layout the prompt areas
    #   *  Process configuration options.
    #
    # @param args - Configuration options.
    #
    constructor args {
        installhull using toplevel
        set options(-program)       ""
        set options(-janussourceid) "0"
        set options(-host)          [Configuration::get JanusSSHPipeHost]
        set options(-port)          [Configuration::get JanusSSHPipePort]
        set options(-ring)          ""
        
        install wrapper using DialogWrapper $win.wrapper
        install form using $wrapper controlarea
        set f $form
        
        textprompt $f.program -text {Client script:} \
            -textvariable [myvar options(-program)]
        ttk::button $f.findprogram  -text Browse... -command [mymethod _browseProgram]
        
        textprompt $f.janussourceid -text {Source ID:} -textvariable [myvar options(-janussourceid)]
        textprompt $f.host -text {Server Host:} -textvariable [myvar options(-host)]
        textprompt $f.port -text {Server Port:} -textvariable [myvar options(-port)]
        textprompt $f.ring -text {Output Ring:} -textvariable [myvar options(-ring)]

        grid $f.program $f.findprogram
				grid $f.janussourceid
        grid $f.host
        grid $f.port
        grid $f.ring
        
        pack $wrapper
        
        # Configure
        $self configurelist $args
        
    }
    #--------------------------------------------------------------------------
    # Private methods.
    #
    
    ##
    # _browseProgram
    #    Use tk_getSaveFile to select a readout program.
    #
    method _browseProgram {} {
        set filename [tk_getOpenFile -title {Choose Readout} -parent $win \
            -defaultextension "" -filetypes [list \
                {{Python scripts} .py}            \
            ]]
        if {$filename ne ""} {
            set f [$wrapper controlarea]
            $f.program delete 0 end
            $f.program insert end $filename
        }
    }
}
##
# ::JanusSSHPipe::promptParameters
#
#   Prompt for the parameters and turn them into the dict that
#   DataSourceUI::getParameters normally returns.
#
# @return dict Keys are parameter names, values are three element lists of
#              long prompt, dummy widget name, parameter value.
#
proc ::JanusSSHPipe::promptParameters {} {
    ::JanusSSHPipe::ParameterPromptDialog .janussshpipeprompt
    
    set action [.janussshpipeprompt modal]
    if {$action eq "Ok"} {
        set result [::JanusSSHPipe::parameters]
        array set optionlookup [list janussourceid -janussourceid host -host port -port \
				    program -program ring -ring]
        dict for {key value} $result {
          set val [.janussshpipeprompt cget $optionlookup($key)]
            dict lappend result $key [list] [string trim $val]
        }
        
    } else {
        set result ""
        
    }
    destroy .janussshpipeprompt
    return $result
}
