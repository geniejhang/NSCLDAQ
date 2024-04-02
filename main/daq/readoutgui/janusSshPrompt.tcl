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
    Configuration::Set JanusSSHPipeHost     "localhost"
    Configuration::Set JanusSSHPipeProgram  ""
}
##
#  ::JanusSSHPipe::paramEnvironmentOverrides
#
#  Apply any environment variables that override the defaults.
#
proc ::JanusSSHPipe::paramEnvironmentOverrides {} {
    Configuration::readEnvironment JanusSSHPipeHost DAQHOST
    Configuration::readEnvironment JanusSSHPipeProgram  RDOFILE
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
#    | Host:   [ entry widget ]                                           |
#    | Readout:[ entry widget ]  <Browse...>                              |
#    | ConfigFile: [ entry widget ] <Browse...>                           |
#    +--------------------------------------------------------------------+
#    | <Ok>   <Cancel>                                                    |
#    +--------------------------------------------------------------------+
#
# OPTIONS:
#   -host        - Name of the host the program runs on.
#   -program     - Path to the readout program that is run on the ssh pipe.
#   -configfile  - Janus_Config.txt with the absolute path
#
# METHODS
#   modal    - Block until either OK or Cancel has been clicked (or the dialog
#              destroyed).  [delegated to a DialogWrapper object].
#
snit::widgetadaptor ::JanusSSHPipe::ParameterPromptDialog {
    component wrapper
    component form
    
		option -janussourceid -default 0
    option -host       
    option -program    
    option -configfile
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
        set options(-janussourceid) "0"
        set options(-host)          [Configuration::get JanusSSHPipeHost]
        set options(-program)       [Configuration::get JanusSSHPipeProgram]
        set options(-configfile)    ""
        set options(-ring)          "janus"
        
        install wrapper using DialogWrapper $win.wrapper
        install form using $wrapper controlarea
        set f $form
        
        textprompt $f.janussourceid -text {Source ID:} -textvariable [myvar options(-janussourceid)]
        textprompt $f.host -text {Host name:} -textvariable [myvar options(-host)]
        
        textprompt $f.program -text {Readout program:} \
            -textvariable [myvar options(-program)]
        ttk::button $f.findprogram  -text Browse... -command [mymethod _browseProgram]
        
        textprompt $f.configfile -text {Janus config file:} \
            -textvariable  [myvar options(-configfile)]
        ttk::button $f.findconfigfile  -text Browse... -command [mymethod _browseConfigFile]

        textprompt $f.ring -text {Output Ring:} -textvariable [myvar options(-ring)]

				grid $f.janussourceid
        grid $f.host
        grid $f.program $f.findprogram
        grid $f.configfile $f.findconfigfile
        grid $f.ring
        
        pack $wrapper
        
        # Configure
        $self configurelist $args
        
    }
    #--------------------------------------------------------------------------
    # Private methods.
    #
    
    ##
    # _browseWD
    #
    #   Browse for a directory to use as the cwd.
    #   The result is set in the $form.dir field.
    #   That field is assumed to  not be disbled because the browse button
    #   -state is coupled to that of the entry.
    #
    method _browseWD      {} {
        #
        #  Initial dir is by priority
        #  - The current value.
        #  - The program's directory if it has one.
        #  - The current working directory if not
        
        set programPath [$form.program get]
        if {[$form.dir get] ne ""} {
            set initialdir [file normalize [$form.dir get]]
        } elseif {[$form.program get] ne ""} {
            set initialdir [file dirname [$form.program get]]
        } else {
            set initialdir [pwd]
        }
        
        set wd [tk_chooseDirectory -initialdir $initialdir -mustexist true \
            -parent $win -title "Choose working directory"]
        if {$wd ne ""} {
            $form.dir delete 0 end
            $form.dir insert end $wd
        }
    }

    ##
    # _browseProgram
    #    Use tk_getSaveFile to select a readout program.
    #
    method _browseProgram {} {
        set filename [tk_getOpenFile -title {Choose Readout} -parent $win \
            -defaultextension "" -filetypes [list \
                {{Exectuable files} *}            \
            ]]
        if {$filename ne ""} {
            set f [$wrapper controlarea]
            $f.program delete 0 end
            $f.program insert end $filename
        }
    }

    ##
    # _browseConfigFile
    #    Use tk_getSaveFile to select a Janus_Config file.
    #
    method _browseConfigFile {} {
        set filename [tk_getOpenFile -title {Choose Config File} -parent $win \
            -defaultextension "" -filetypes [list \
                {{Config files} .txt}            \
            ]]
        if {$filename ne ""} {
            set f [$wrapper controlarea]
            $f.configfile delete 0 end
            $f.configfile insert end $filename
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
        array set optionlookup [list janussourceid -janussourceid host -host path -program \
            configfile -configfile ring -ring]
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
