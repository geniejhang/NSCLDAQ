# cvs.tcl --
#
#	Handling of various cvs output formats.
#
# Copyright (c) 2003 Andreas Kupries <andreas_kupries@sourceforge.net>
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
# 
# RCS: @(#) $Id$

package require Tcl 8.2
package require textutil

namespace eval ::doctools {}
namespace eval ::doctools::cvs {
    namespace export scanLog toChangeLog
}

# ::doctools::cvs::scanLog --
#
#	Scan a log generated by 'cvs log' and extract the relevant information.
#
# Arguments:
#	text	The text to scan
#
# Results:
#	None.
#
# Sideeffects:
#	None.
#
# Notes:
#	Original location of code:	http://wiki.tcl.tk/3638
#				aka	http://wiki.tcl.tk/log2changelog
#	Original author unknown.
#	Bugfix by TR / Torsten Reincke

proc ::doctools::cvs::scanLog {text evar cvar fvar} {

    set text [split $text \n]
    set n    [llength $text]

    upvar $evar entries  ;    #set       entries  [list]
    upvar $cvar comments ;    #array set comments {}
    upvar $fvar files    ;    #array set files    {}

    for {set i 0} {$i < $n} {incr i} {
	set line [lindex $text $i]
	switch -glob -- $line {
	    "*Working file:*" {
		regexp {Working file: (.*)} $line -> filename
	    }
	    "date:*" {
		scan $line "date: %s %s author: %s" date time author
		set author [string trim $author ";"]

		# read the comment lines following date
		set comment ""
		incr i
		set line [lindex $text $i]
		# [TR]: use regexp here to see if log ends:
		while {(![regexp "(-----*)|(=====*)" $line]) && ($i < $n)} {
		    append comment $line "\n"
		    incr i
		    set line [lindex $text $i]
		}

		#  Store this date/author/comment
		lappend entries [list $date $author]
		lappend comments($date,$author) $comment
		lappend files($date,$author,$comment) $filename
	    }
	}
    }

    return
}


# ::doctools::cvs::toChangeLog --

#	Convert a preprocessed cvs log (see scanLog) into a Changelog
#	suitable for emacs.
#
# Arguments:
#	evar, cvar, fvar: Name of the variables containing the preprocessed log.
#
# Results:
#	A string containing a properly formatted ChangeLog.
#
# Sideeffects:
#	None.
#
# Notes:
#	Original location of code:	http://wiki.tcl.tk/3638
#				aka	http://wiki.tcl.tk/log2changelog
#	Original author unknown.

proc ::doctools::cvs::toChangeLog {evar cvar fvar} {
    upvar $evar entries
    upvar $cvar comments
    upvar $fvar files

    set linebuffer [list]

    foreach e [lsort -unique -decreasing $entries] {

	#  print the date/author
	foreach {date author} $e {break}
	lappend linebuffer "$date $author"
	lappend linebuffer ""

	#  Find all the comments submitted this date/author

	set clist [lsort -unique $comments($date,$author)]

	foreach c $clist {
	    #  Print all files for a given comment
	    foreach f [lsort -unique $files($date,$author,$c)] {
		lappend linebuffer "\t* $f:"
	    }

	    #  Format and print the comment

	    lappend linebuffer [textutil::indent [textutil::undent $c] "\t  "]
	    lappend linebuffer ""
	    continue
	}
    }

    return [join $linebuffer \n]
}

#------------------------------------
# Module initialization

package provide doctools::cvs 0.1
