lib_LTLIBRARIES = libDDASDataSimulator.la
include_HEADERS = DDASDataSimulator.h
libDDASDataSimulator_la_SOURCES = DDASDataSimulator.cpp
libDDASDataSimulator_la_CPPFLAGS = @UFMT_CPPFLAGS@ @LIBTCLPLUS_CFLAGS@ @DDASFMT_CPPFLAGS@
libDDASDataSimulator_la_LIBADD = @UFMT_LDFLAGS@ @LIBEXCEPTION_LDFLAGS@ @DDASFMT_LDFLAGS@

##
# Test suite
#

noinst_PROGRAMS = DDASDataSimulatorTests
noinst_HEADERS = Asserts.h
DDASDataSimulatorTests_SOURCES = TestRunner.cpp DDASDataSimulatorTests.cpp
DDASDataSimulatorTests_CPPFLAGS = ${libDDASDataSimulator_la_CPPFLAGS} @CPPUNIT_CFLAGS@ 
DDASDataSimulatorTests_LDFLAGS = ${libDDASDataSimulator_la_LIBADD} @CPPUNIT_LDFLAGS@
TESTS = DDASDataSimulatorTests
