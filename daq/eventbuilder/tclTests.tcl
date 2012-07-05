#!/bin/sh
# \
exec tclsh "$0" "$@"

package require tcltest
::tcltest::configure -testdir [file dirname [file normalize [info script]]]
eval ::tcltest::configure $argv
::tcltest::runAllTests
