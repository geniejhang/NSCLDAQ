#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#            NSCL
#            Michigan State University
#        

#  Test various possibilities for the tolerance 

package require tcltest
source displayepics.tcl

tcltest::test asymtol-1 {Test asymmetric tolerances} \
    -body {
	set a [parseAsymmetricRange ".1:+.5-.25"]	
    }                                                \
    -result {-0.15 0.6 .1}
	
tcltest::test aymtol-2 {Test asymmetric tolerances} \
    -body {
	set a [parseAsymmetricRange ".1:-.25+.5"]
    }                                                \
    -result {-0.15 0.6 .1}

tcltest::test asymtol-3 {Test old-style asymm tolerances} \
    -body {
	parseAsymmetricRange ".1+.5-.25"
    }                                                    \
    -result {-0.15 0.6 0.1}

tcltest::test asymtol-4 {test old-style sym tolerances ..} \
    -body {
	parseAsymmetricRange ".1-.25+.5"
    }                                                  \
    -result {-0.15 0.6 0.1}

tcltest::test nominal-1 {Test symmetric nominal range} \
    -body {
	set a [getNominalRange 1:.5]
    }                                                  \
    -result {0.5 1.5 1.0}

tcltest::test nominal-2 {Test asymmetric range via getNominal} \
    -body {
	set a [getNominalRange 1:+.5-.25]
    }                                                          \
    -result {0.75 1.5 1}

tcltest::test nominal-3 {Test asymmetric range 2 via getNominal} \
    -body {
	set a [getNominalRange 1:-.25+.5]
    }                                                             \
    -result {0.75 1.5 1}

tcltest::test nominal-4 {Test asym range via getNominal old style} \
    -body {
	getNominalRange  ".1+.5-.25"
    }                                         \
    -result {-0.15 0.6 0.1}

tcltest::test exponential-1 {Test symmetric range with exponents} \
    -body {
	getNominalRange 1.0e2:1.0e-1
    }                                     \
    -result {99.9 100.1 100.0}
tcltest::test exponential-2 {Test sym range with exps that have signs} \
    -body {
	getNominalRange 1.0e-1+1.0e-1-1
    }                                                 \
    -result {-0.9 0.2 0.1}


tcltest::cleanupTests