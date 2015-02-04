#!/bin/bash
#
# Start Wish. \
exec tclsh ${0} ${@}
#
#  Save SEETF control settings to the recoverable database so that future
#  runs of the same beam can re-use them.
#  settings saved are:
#      Shaper settings.
#      CFD    settings.
#      HV     settings (3 file).
#  The settings are saved in:
#     ~see/saved_settings/<beam>
#   where <beam> is of the form AAEEVV
#      AA - The atomic mass of the isotope.
#      EE - The chemical element name of the isotope.
#      VV - The beam energy in MeV/A.
#
# $Log: controlsave.tcl,v $
# Revision 1.3  2004/05/10 13:04:51  see
# Fix case of improperly maintained database for accepted overwrite.
#
# Revision 1.2  2004/05/10 12:41:44  see
# Fix error in database insertion logic.  Comparison went the wrong way and
# there also was no code to handle case where the new entry should go last.
#

#  Global variables (configuration parameters):

set ConfigurationRoot "~see/saved_settings"; # Top of configuration directory tree.
set WorkingDirectory  "~/config";	# Where the config files to save are.

#  The files to save.

set shaperfile "shaper_defaults.shaper_values"
set cfdfile    "seecfd_default.cfd_settings"
set lrhvfile   "scintlr_hv_defaults.hv_settings"
set udhvfile   "scintud_hv_defaults.hv_settings"
set ppachvfile "ppac_hv_defaults.hv_settings"

#  List of file variables.

set ConfigFiles {shaperfile cfdfile lrhvfile udhvfile ppachvfile}

# IsInteger
# Purpose
#	Returns true if the parameter is an integer.
# Inputs
#   -	I - the value to check for integerness.
# Output:
#   -	true  - $I is an integer.
#   -	False - $I is not an integer.
#
proc IsInteger {i} {
    return [expr [scan $i %d a] == 1]
}






#
#   ParseBeam
#  Purpose:
#     Breaks a beam specification apart into N, Element Name and energy.
#  Inputs:
#       Beam Specification.
#   Outputs:
#       List containing {N Element Energy}
#   Errors:
#      Beams that cannot be parsed in this way error:
#    "Beam specification <spec> not valid"
#
proc ParseBeam {Beam} {
    #
    # Check for leading digits:
    #
    if {![regexp -indices {^[0-9]+} $Beam where]} {
	error "ParseBeam - missing leading atomic number. $Beam"
    }
    set A [string range $Beam [lindex $where 0] [lindex $where 1]]
    set remainder [string range $Beam [expr [lindex $where 1]+1] end]

    # Parse the chemical element.

    if {![regexp -indices {^[A-Za-z]+} $remainder where]} {
	error "ParseBeam - missing alphabetical chemical name $Beam"
    }
    if {([lindex $where 1] - [lindex $where 0]) >= 2 } {
	error "ParseBeam - Chemical element name can be at most 2 chars $Beam"
    }
    set Element [string range $remainder [lindex $where 0] [lindex $where 1]]
    set remainder [string range $remainder [expr [lindex $where 1] +1] end]


    # Parse the energy.

    if {![regexp -indices {^[0-9]+} $remainder where]} {
	error "ParseBeam - missing traling beam energy"
    }
    set Energy [string range $remainder  [lindex $where 0] [lindex $where 1]]

    return "$A $Element $Energy"
	   
}

# AssembleBeam
# Purpose:
#	Create a beam specification from component parts.
# Inputs:
# -	N - Atomic mass.
# -	Element - Chemical element name.
# -	Energy  - energy in MeV
# Outputs:
# 	A string of the form NeEEE where N is the  Atomic mass, e is the 
#      Chemical element name, and EEE is the zero filled MeV/nucleon. E.g.
#      48, Ca 140 -> 48Ca140.
#
# Errors:
# 	Throws errors if Atomic mass or energy are not integers. 
#  Note that we don't validity check the chemical element name 
#  in this version.
#
proc AssembleBeam {N Element Energy} {
    #   The mass and energy must be ints:

    if {![IsInteger $N]} {
	error "AssembleBeam - Atomic mass $N is not an integer."
    }
    if {![IsInteger $Energy] } {
	error "AssembleBeam - Beam energy $Energy is not an integer."
    }
    append Beam $N $Element $Energy
    return $Beam
}

# SourceDatabase
# Purpose:
#	Read the current beam settings database into an internal format.
# Inputs:
#	Database filename.
# Outputs:
#	A global array: ConfigurationDatabase, indexed by Atomic mass/element
#  name  Each element contains a list of lists.  The first element of each
#  innermost list is the beam energy in MeV/A.  The remaining elements of 
#  the innermost list are:
# -	shaper settings filename (full path)
# -	cfd settings filename
# -	left/right scintillator HV settings.
# -	Up/down scintillator HV settings.
# -	PPAC HV settings.
# Errors:
#   Failures to open the database file.
#
proc SourceDatabase {file} {
    global ConfigurationDatabase

    if {[catch "source $file"]} {
	error "SourceDatabase - Could not source database file: $file"
    }
}
# HaveBeam
# Purpose:
#	Interrogates the ConfigurationDatabase to determine if a 
#  specified beam is already known.
# Inputs:
# -	ConfigurationDatabase (global)
# -	N - beam Atomic mass.
# -	Element - chemical element name
# -	Energy   - Beam energy in MeV/A.
# Outputs:
# -	true - Database has selected beam.
# -	False - Database does not have selected beam.
# Errors:
#	Errors if N/Energy are not integers.
#
proc HaveBeam {N Element Energy} {
    global ConfigurationDatabase

    if {![IsInteger $N]} {
	error "HaveBeam: Atomic mass $N is not an integer"
    }
    if {![IsInteger $Energy]} {
	error "HaveBeam: Beam energy $Energy is not an integer"
    }

    #   The beams are stored as lists in the array elements
    #  of ConfigurationDatabase indexec by $N$Element:
    #
    append Isotope $N $Element
    if {[array names ConfigurationDatabase $Isotope] == ""} {
	return 0;			# No array element for the isotope.
    }
    set BeamList $ConfigurationDatabase($Isotope); # Beam list for the isotope.
    foreach beam $BeamList {
	if {[lindex $beam 0] == $Energy} {
	    return 1;			# Found a match!
	}
    }
    return 0;				# No matches.
}

# MakeSaveDirectory
# Purpose:
# 	Create a directory for the settings associated with a beam.
# Inputs:
# 	SubdirectoryName - The name of the directory to create.  This is
#  assumed to be relative to the database root (~see/saved_settings).  
# By convention, the directory will be named AEV where A - The Atomic 
# mass E the chemical element name, V the beam energy.
# 
# Outputs:	
# 	The Full path to created directory.
# Errors:
#	Error thrown if directory could not be made.

proc MakeSaveDirectory {name} {
    global ConfigurationRoot

    append Directory $ConfigurationRoot "/" $name
    
    if {[catch "file mkdir $Directory"]} {
	error "MakeDirectory - Unable to make directory $Directory"
    }

    file attributes  $Directory -permissions "u=rwx,g=rwx,o=rx"

    return $Directory
}



# CopyFiles
# Purpose:
# 	To copy the saved settings file to the target directory (created by 
#       MakeSaveDirectory)
# Inputs:
# -     target - the target directory to which the files get copied.
#	The files are assumed to be in the default files at ~/config saved 
# as the defaults:
# -	shaper_defaults.shaper_values - the shaper data file.
# -	seecfd_default.cfd_settings -The CFD data file.
# -	scintlr_hv_defaults.hv_settings- The scintillator left/right HV
#        settings.
# -	scintud_hv_defaults.hv_settings - The scintillator Up/Down HV Settings.
# -	ppac_hv_defaults.hv_settings - The PPAC HV settings.
# Outputs:
# 	List of the full paths to the files created (in the order above 
#       [how they appear in the database too].

proc CopyFiles {target} { 
    global WorkingDirectory;		# Source of the config files.
    global ConfigFiles;			# List of globals with filenames.

    foreach Filevar $ConfigFiles {
	global $Filevar;		# These list elements are global vars.
	set    SourceFile $WorkingDirectory
	append SourceFile "/" [set $Filevar]
	set    DestFile   $target
	append DestFile  "/"  [set $Filevar]

	if {[catch "file copy -force $SourceFile $DestFile"]} {
	    error "CopyFiles: Failed to copy $SourceFile -> $DestFile"
	}
	file attributes $DestFile -permissions "u=rw,g=rw,o=r"
	lappend OutputFiles $DestFile
    }
    return $OutputFiles
}

# EnterBeam
# Purpose:
#	Add a beam specification to the database in the ConfigurationDatabase 
# global array.
# Inputs:
# -	N  - Atomic mass of the beam.
# -	E  - Chemical element number of the beam.
# -	V  - The energy in MeV/A of the beam.
# -	List of configuration files as returned from CopyFiles.
# -	
# Outputs: 
#	None, ConfigurationDatabase (global) does get modified.
# Errors:
# 	Message if N,V are not integer.
# 	Message if beam already exists.
#
proc EnterBeam {N E V FileList} {
    global ConfigurationDatabase

    if {[HaveBeam $N $E $V]} {
	error "EnterBeam: Duplicate entry for $N$E$V"
    }

    # Validate the parameters that must be integers:

    if {![IsInteger $N]} {
	error "EnterBeam: Atomic mass $N must be an integer"
    }
    if {![IsInteger $V]} {
	error "EnterBeam: Beam energy $V must be an integer"
    }
    #  Create the array index, and the new database entry:

    append Index $N $E
    lappend NewEntry $V
    foreach file $FileList {
	lappend NewEntry $file
    }

    #  If the element does not exist, this is simple... create it and
    #  add the single list entry to it:
    if {[array names ConfigurationDatabase $Index ] == ""} {
    	puts "new entry"
	lappend ConfigurationDatabase($Index) $NewEntry
    } else {
	#  If it does exist, we must insert this in the 'proper place'
	# in the sorted list.. or declare an error if there's a dup.

	set OldList $ConfigurationDatabase($Index)
	set done 0
	foreach entry $OldList {
	    set Energy [lindex $entry 0]
	    if {$Energy == $V} {
		error "EnterBeam: Dup entry for $N$E$V"
	    }
	    if {($Energy > $V) && (!$done)} { # Insert before this!
		lappend NewList $NewEntry
		set done 1
	    }
	    lappend NewList $entry
	}
	if {! $done} {
	    lappend NewList $NewEntry
	}
	set ConfigurationDatabase($Index) $NewList
    }
    
}
#
# ReplaceBeam
# Purpose
#	To replace an existing beam specification in the 
#   ConfigurationDatabase global.
# Inputs:
# -	N  - Atomic mass of the beam.
# -	E  - Chemical element number of the beam.
# -	V  - The energy in MeV/A of the beam.
# -	FileList of configuration files as returned from CopyFiles.
# Outputs: 
# 	None, ConfigurationDatabase (global) does get modified.
#
proc ReplaceBeam {N E V FileList} {
    global ConfigurationDatabase

   
    # Validate the parameters that must be integers:

    if {![IsInteger $N]} {
	error "EnterBeam: Atomic mass $N must be an integer"
    }
    if {![IsInteger $V]} {
	error "EnterBeam: Beam energy $V must be an integer"
    }
    # Replace requires existence:

    if {![HaveBeam $N $E $V] } {
	error "ReplaceBeam: $N$E$V Does not exist"
    }

    #  Create the array index, and the new database entry:

    append Index $N $E
    lappend NewEntry $V
    foreach file $FileList {
	lappend NewEntry $file
    }

    #  Locate and replace the appropriate element:

    set OldList $ConfigurationDatabase($Index)
    foreach beam $OldList {
	if {[lindex $beam 0] == $V} {
	    lappend NewList $NewEntry
	} else {
	    lappend NewList $beam
	}
    }
    set ConfigurationDatabase($Index) $NewList

    
}

# WriteIsotope
# Purpose:
#	Writes the beam descriptions associated with an isotope 
# (Atomic mass/chemical element) [element of the ConfigurationDatabase 
# global array], to file in database format.
#
# Inputs:
# -	TCL file descriptor open on the database file
# -	Isotope - the name of the isotope.
# -	ConfigurationDatabase (global).
# Outputs:
# 	NONE
#
proc WriteIsotope {Fd Isotope} {
    global ConfigurationDatabase

    set BeamList $ConfigurationDatabase($Isotope)

    puts $Fd "set ConfigurationDatabase($Isotope) {     \\"
    foreach beam $BeamList {
	puts $Fd "{ $beam }   \\"
    }
    puts $Fd "}"
}

# WriteDatabase 
# Purpose:
# 	Write the database in ConfigurationDatabase back to file.  
#  In order to ensure integrity:
# O The database is written to target.tmp
# O The database is then mv'd to target.
# 
# Inputs:
# -	ConfigurationDatabase (global array)
# -	Target - Full path to database file.
# Outputs:
# 	None
# Errors:
# 	If unable to write file, or rename it, or set appropriate permissions.
#
proc WriteDatabase {Target} {
    global ConfigurationDatabase

    if {[catch {set f [open $Target.tmp w]}]} {
	error "WriteDatabase: Unable to open the database file $Target"
    }

    set Isotopes [lsort [array names ConfigurationDatabase]]

    foreach Isotope $Isotopes {
	WriteIsotope $f $Isotope
    }
    close $f
    file rename -force $Target.tmp $Target
    file attributes $Target -permissions "u=rw,g=rw,o=r"


}
#  SaveSettings
#    This procedures is intended to be called when the user wants to 
#    save a bunch of SEE control settings for a beam. 
#    separating this into a proc makes possible interactive testing.
# Inputs:
#    Beam   - The beam to save for.
#
#
proc SaveSettings {Beam} {
    global ConfigurationRoot
    #
    #  Read in the database.

    SourceDatabase $ConfigurationRoot/database.tcl; # Read in existing database.
    # Break the beam into its consitutents and check to see if there
    # are already settings for it:

    set BeamInfo [ParseBeam $Beam]
    set Mass    [lindex $BeamInfo 0]
    set Element [lindex $BeamInfo 1]
    set Energy  [lindex $BeamInfo 2]
    set Exists  [HaveBeam $Mass $Element $Energy]
    # If the beam exists, require confirmation before overwriting:

    if {$Exists} {
	puts -nonewline "$Beam already has saved settings, Ovewrite? (y/N): "
	flush stdout
	gets stdin      ok
	set ok [string index $ok 0]
	puts "Response to prompt was $ok"
	if { ($ok == "N")  || ($ok == "n") } {
	    error "$Beam will not be ovewritten -- exiting"
	}
    }
    #  Put the files in place:

    append dirname $Mass $Element $Energy
    set TargetDir [MakeSaveDirectory $dirname]
    set Files     [CopyFiles $TargetDir]
    if {$Exists} {
	ReplaceBeam $Mass $Element $Energy $Files
    } else {
	EnterBeam $Mass $Element $Energy $Files
    }

    WriteDatabase $ConfigurationRoot/database.tcl

}


if {[llength $argv] != 1} {
    puts "Usage: "
    puts "   controlsave beam"
    puts " where beam is a beam specification of the form AIE"
    puts "    A  - the mass of the isotope."
    puts "    I  - The chemical name of the isotope."
    puts "    E  - The beam energy."
    puts " Example:"
    puts "    controlsave 48Ca140 "
    puts "Saves the current defaults as the settings for 48Ca 140MeV/a"
    exit 1
}

SaveSettings [lindex $argv 0]
