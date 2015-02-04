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


#  This file contains a simple manager for the beam saved settings.
#  Functions supported are:
#     - delete
#     - rename

set version 1.0

# Add the canonicalized path to the script to
# the front of auto-Path
#

set here [file dirname [info script]]
set wd [pwd]
cd  $here
set here [pwd]
cd $wd
if {[lsearch -exact $auto_path $here] == -1} {
    set auto_path [concat $here $auto_path]
}


# Packages:

package require snit
package require BWidget
package require renameDialog

#  Top level definitions:

set oldDatabaseDir    "~see/saved_settings";      # for testing only.
# set databaseDir       "./saved_settings";         # testing
set databaseDir       $oldDatabaseDir;            #production

set descriptionFile   [file join $databaseDir database.tcl]

#  This stocks the ConfigurationDatabase variable.
#  This is an array indexed by beam species.
#  Each element of the array is a list of lists.
#  Element 0 of each list in the list of lists is the energy
#  All the other information is the set of files in the database.
#

source $descriptionFile

#---------------------------------------------------------------
# compareSpecies a b
#    Does a sort-worthy comparison of two species.
#    Species are of the form Aname where A is the isotopic number
#    and name is the element name e.g. 172Xe
#    The primary sort is by isotope, the secondarry sort
#    is by isotopic number.
#
proc compareSpecies {a b} {
    set ascan [scan $a %d%s anum aele]
    set bscan [scan $b %d%s bnum bele]

    # If an element is not in this form,
    # just do a string compare.

    if {($ascan != 2) || ($bscan != 2)} {
        return [string compare $a $b]
    }

    # same element:

    if {$aele eq $bele} {
        if {$anum < $anum} {
            return -1
        }
        if {$anum == $bnum} {
            return 0
        }
        if {$anum > $bnum} {
            return 1
        }
    }
    # Different element:

    return [string compare $aele $bele]

}
#---------------------------------------------------------------
# fillTree  widget
#      Fill a bwidget tree widget with the ConfigurationDatabase
#      The tree produced is a 2-level hierarchy of species, energy
#      with the data associated with each node is
#      The contents of each list element.
#
proc fillTree widget {
    global ConfigurationDatabase
    foreach species [lsort -command compareSpecies [array names ConfigurationDatabase]] {
        set top [$widget insert end  root $species -text $species \
                                    -helptext {Click + to look at energies}]
        foreach item $ConfigurationDatabase($species) {
            set energy [lindex $item 0]
            $widget insert end $top $top.$energy \
                    -text "$energy MeV" -data $item
        }
    }
}
#----------------------------------------------------------------
# writeDatabase
#     Write the ConfigurationDatabase back out to file.
#
proc writeDatabase {} {
    global ConfigurationDatabase
    global descriptionFile
    global databaseDir

    append backupDescription [file join $databaseDir \
                              [file tail [file rootname $descriptionFile]]] \
                            .backup
    if {[file exists $backupDescription]} {
        file delete $backupDescription
    }
    file rename $descriptionFile $backupDescription
    set fd [open $descriptionFile w]


    foreach species [array names ConfigurationDatabase] {
        if {[llength $ConfigurationDatabase($species)] != 0} {
            puts $fd "set ConfigurationDatabase($species) [list $ConfigurationDatabase($species)]"
        }
    }

    close $fd
}

#----------------------------------------------------------------
# removeSetting species energy
#      Removes the indicated species/energy from the
#      ConfigurationDatabase, along with associated data files
#      and write the resulting configuration database back to disk.
#
proc removeSetting {species energy} {
    global ConfigurationDatabase
    global databaseDir
    global oldDatabaseDir
    set settings $ConfigurationDatabase($species)
    set elementNumber 0
    foreach setting $settings {
        if {$energy eq [lindex $setting 0]} {
            set ConfigurationDatabase($species) \
                [lreplace $settings $elementNumber $elementNumber]
            set files [lreplace $setting 0 0]
            foreach file $files {
                regsub $oldDatabaseDir $file $databaseDir filename
            }
            writeDatabase
            return
        } else {
            incr elementNumber
        }
    }
    error "Deleting nonexistent database element."
}
#----------------------------------------------------------------
# Delete widget node
#      Delete the settings described by node (pending user confirmation).
# Parameters:
#  widget  - the tree containing the settings.
#  node    - The node describing the settings.
#            The node name is of the form species.energy
#
proc Delete {widget node} {
    set info [split $node .]
    set species [lindex $info 0]
    set energy  [lindex $info 1]

    set message "You are about to delete the settings for $species\
at $energy MeV.  If you are certain this is what you want to do, click
Ok below, otherwise, click Cancel"
    set answer [tk_messageBox -icon warning -type okcancel \
                              -default cancel -title {Confirm Delete?} \
            -message $message]
    if {$answer eq "ok"} {
        removeSetting $species $energy
        $widget delete $node
    }
}
#------------------------------------------------------------------
# addSetting tree species settings
#     Add a database entry to the ConfigurationDatabase and the
#     tree.
# Parameters:
#   tree    - The tree widget.
#   species - The species of the database entry
#   settings- Value  of the ConfigurationDatabaseList entry for this
#             item.
#
proc addSetting {tree species settings} {
    global ConfigurationDatabase

    #  The configuration database lists for each species
    #  are sorted by energy... but entries may have leading
    #  0's so alpha sorts are what are appropriate (some are invalid octals
    #  and numbers with leading 0's are interpreted as octal).
    #
    set energy [lindex $settings 0]
    # There may not be any of this species yet:
    # In that case create the entry, and create the
    # species and energy in the tree:
    # For now at the end.
    #
    if {[array names ConfigurationDatabase $species] eq ""} {
        $tree insert end root $species -text $species
        $tree insert end $species $species.$energy -text "$energy MeV" \
            -data $settings
        lappend ConfigurationDatabase($species) $settings
        return
    }
    #  ConfigurationDatabase($species) may or may not
    #  have an entry for this item.. if it does, we just replace
    #  the entry with $settings, and update the -data option on the
    #  tree entry.
    #
    set node $species.$energy

    set oldsettings $ConfigurationDatabase($species)
    set index 0
    set newenergy [lindex $settings 0]
    foreach setting $oldsettings {
        if {$newenergy eq [lindex $setting 0]} {
            $tree itemconfigure $node -data $settings
            set ConfigurationDatabase($species) \
                [lreplace $oldsettings $index $index $settings]
            return
        }
        incr index
    }

    # The settings energy is new for this species and must be inserted
    # in the right part of the tree and database.
    #
    set index 0
    foreach setting $oldsettings {
        # Insert at this index..
        if {[string compare $newenergy [lindex $setting 0]] == -1} {
            set ConfigurationDatabase($species) \
                [linsert $oldsettings $index $settings]
            $tree insert $index $species $node -text "$newenergy MeV" \
                    -data $settings
            return
        }
        incr index
    }
    #  New setting is at the end of the list:

    lappend ConfigurationDatabase($species) $settings
    $tree insert end $species $node -text "$newenergy MeV" \
            -data $settings

}
#
#------------------------------------------------------------------
# onRename tree dialog node
#      DO the actual rename of a setting in response to an ok
#      from the dialog.
# Parameters:
#    tree   - Tree widget.
#    dialog - The renameDialog.
#    node   - Tree node being renamed.
#
proc onRename {tree dialog node} {
    global ConfigurationDatabase
    global databaseDir
    global oldDatabaseDir

    set oldSpecies [$dialog cget -species]
    set oldEnergy  [$dialog cget -energy]
    set newSpecies [$dialog cget -renamedspecies]
    set newEnergy  [$dialog cget -renamedenergy]

    if {($oldSpecies eq $newSpecies) && ($oldEnergy eq $newEnergy) } {
        return
    }

    # Locate the exact old configuration:

    set oldsettings $ConfigurationDatabase($oldSpecies)
    set newsetting $newEnergy
    set newdir [file join $databaseDir $newSpecies$newEnergy]
    file mkdir $newdir
    foreach setting $oldsettings {
        if {[lindex $setting 0] == $oldEnergy} {
            set files [lrange $setting 1 end]
            foreach file $files {
                set base [file tail $file]
                regsub $oldDatabaseDir $file $databaseDir $file
                set destfile [file join $newdir $base]
                file copy -force $file $destfile
                lappend newsetting $destfile
            }
        }
    }

    #
    #  Adjust the settings database and tree

    addSetting $tree $newSpecies $newsetting

    removeSetting $oldSpecies $oldEnergy;    # Remove old setting.
    $tree delete $oldSpecies.$oldEnergy;   # Remove from tree too.

}
#------------------------------------------------------------------
#  Rename widget node
#       Prompt for the rename of a setting.
#  Parameters:
#      widget   - the tree widget.
#      node     - name of the node to rename.
#                 This has the form species.energy
#
proc Rename {widget node} {
    set info     [split $node .]
    set species  [lindex $info 0]
    set energy   [lindex $info 1]

    renameDialog .dialog -energy $energy -species $species \
        -command [list onRename $widget .dialog $node]
    .dialog modal

    destroy .dialog

}

#-----------------------------------------------------------
# menupopup node x y
#     Called in response to a right click on an item in
#     the tree.  If this is a 'terminal' node, the
#     menu is popped up (posted) to allow the user
#     to delete or rename files.
#
# node  Node which caused the popup.
# x,y   Where to pop up the menu.
#
proc menupopup {x y node} {
    set info [split $node .]
    if {[llength $info] == 2} {
        set species [lindex $info 0]
        set energy  [lindex $info 1]
        set list [.tree itemcget $node -data]
        .popup entryconfigure 0 -command [list Delete .tree $node]
        .popup entryconfigure 1 -command [list Rename .tree $node]
        tk_popup .popup $x $y

    }
}

#---------------------------------------------------------------
# showAbout
#     Shows the about dialog for the program.
#
proc showAbout {} {
    tk_messageBox -type ok -icon info -title {About} \
     -message "See settings manager\n\
(c) Michigan State University 2005\n\
All rights reserved\n\
Licensed for redistribution under the GPL \
See: \n\
 http://www.gnu.org/licenses/gpl.txt\n\
for terms and conditions\n\
Author: Ron Fox"


}
#---------------------------------------------------------------
# showHelp
#    Give some useful info about how to operate the program.
#
#
proc showHelp {} {
    toplevel .help
    wm title .help "Using the program"
    set text [text     .help.info -wrap word -height 6]
    button   .help.dismiss -text Dismiss -command [list destroy .help]

    $text insert end "\n    The program displays the tree of settings values.  "
    $text insert end "The top levels of the tree are isotopes, click on "
    $text insert end "the '+' to the left of each isotope to view the "
    $text insert end "energies for which there are settings.  "
    $text insert end "Hold down the right mouse button over a specific setting to bring up the "
    $text insert end "popup menu of operations available for that setting."
    $text configure -state disable

    pack .help.info .help.dismiss
}


# Create the GUI:

menu .popup -tearoff 0
.popup add command -label "Delete..."
.popup add command -label "Rename..."

scrollbar .sb -orient vertical -command [list .tree yview]
Tree .tree    -yscrollcommand [list .sb set]
pack .tree .sb -side left -fill both -expand 1
fillTree .tree

.tree bindText <Button-3> [list menupopup %X %Y]

menu .menu
menu .menu.help -tearoff 0
menu .menu.file -tearoff 0

.menu add cascade -label File -menu .menu.file
.menu add cascade -label Help -menu .menu.help


.menu.file add command -label Exit    -command exit
.menu.help add command -label About   -command showAbout
.menu.help add command -label {How to use} -command showHelp

. configure -menu .menu


