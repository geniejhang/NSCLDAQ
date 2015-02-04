#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#             Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321
#


package provide renameDialog 1.0
package require snit


#     renameDialog is a beam renaming dialog box.
#     It describes the 'from' name and provides
#     entries for the particle species and
#     energy:
#
# +---------------------------------------------+
# | Rename settings for $species at $energy MeV |
# |   [         ] New species                   |
# |   [         ] New energy                    |
# +---------------------------------------------+
# |   [Ok]              [Cancel]                |
# +---------------------------------------------+
#
# Options:
#    -command        - Called when Ok is clicked.
#    -cancelcommand  - Called when Cancel is clicked.
#    -species        - Initial species
#    -energy         - Initial energy
#    -renamedspecies - Value of species entry box.
#    -renamedenergy  - Value of eneryg entry box.
# If the client does not provide values for the -renamed*
# items, they are initialized from the corresponding initial
# values.
#
# Methods:
#    modal    - makes the dialog modal.
#               note that cancel/ok remove the modality.
#
snit::widget renameDialog {
    hulltype toplevel
    option -command          {}
    option -cancelcommand    {}
    option -species          {}
    option -energy           {}
    option -renamedspecies   {}
    option -renamedenergy    {}

    # Early versions of snit... myvar does not work.

    method Myvar name {
        append result  $selfns :: $name
        return $result
    }

    constructor args {
        $self configurelist $args

        #   Default the renamed values.


        frame $win.work -relief groove -borderwidth 4
        label $win.work.old          -text "Rename settings for $options(-species) at $options(-energy)"
        entry $win.work.species      -textvariable [$self Myvar options(-renamedspecies)]
        label $win.work.specieslbl   -text "New species"
        entry $win.work.energy       -textvariable [$self Myvar options(-renamedenergy)]
        label $win.work.energylbl    -text "New energy"

        frame $win.action
        button $win.action.ok        -text Ok      -command [mymethod onOk]
        button $win.action.cancel    -text Cancel  -command [mymethod onCancel]

        # Layout the dialog widgets:

        grid $win.work.old                -
        grid $win.work.species      $win.work.specieslbl
        grid $win.work.energy       $win.work.energylbl

        grid $win.action.ok         $win.action.cancel

        pack $win.work $win.action -side top -fill x -expand 1

       # Create the dialog widgets..

        if {$options(-renamedspecies) eq {} } {
            set options(-renamedspecies) $options(-species)
        }
        if {$options(-renamedenergy) eq {} } {
            set options(-renamedenergy) $options(-energy)
        }


    }
    ####
    # Dispatch option
    #     Dispatches a script in an option.
    # Parameters:
    # option   - name of the option to dispatch (e.g. -command).
    #
    method Dispatch option {
        set script $options($option)
        if {$script ne {}} {
            eval $script
        }
    }
    ####
    #  onOk
    #      Responds to clicks on the Ok button.
    #
    method onOk {} {
        $self Dispatch -command

        if {[winfo exists $win.hidden]} {
            destroy $win.hidden
        }
    }
    ####
    #  onCancel
    #      Responds to click on the Cancel button.
    #
    method onCancel {} {
        $self Dispatch -cancelcommand

        if {[winfo exists $win.hidden]} {
            destroy $win.hidden
        }
    }
    ####
    # modal
    #     Forces the dialog to be app modal.
    #
    method modal {} {
        frame $win.hidden

        wm deiconify $win
        focus -force $win
        grab $win
        tkwait window $win.hidden
        grab release $win


    }
}
