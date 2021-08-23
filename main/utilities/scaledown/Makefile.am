bin_PROGRAMS = scaledown

scaledown_SOURCES=   scaledown.cpp scaledownFunctions.cpp \
	scaledownFunctions.h
# nodist_scaledown_SOURCES=options.c options.h

scaledown_CPPFLAGS=-I@top_srcdir@/utilities/IO 			\
		    -I@top_srcdir@/daq/format			\
		    @LIBTCLPLUS_CFLAGS@
scaledown_LDFLAGS = @top_builddir@/utilities/IO/libdaqio.la 	\
    @top_builddir@/daq/format/libdataformat.la			\
	@top_builddir@/base/dataflow/libDataFlow.la		\
    @LIBEXCEPTION_LDFLAGS@

# BUILT_SOURCES	= options.c options.h

noinst_PROGRAMS=unittests

unittests_SOURCES=scaledownTests.cpp TestRunner.cpp \
	scaledownFunctions.cpp

unittests_CPPFLAGS=$(scaledown_CPPFLAGS) @CPPUNIT_CFLAGS@
unittests_LDFLAGS=$(scaledown_LDFLAGS)   @CPPUNIT_LDFLAGS@

TESTS = unittests

# EXTRA_DIST = makeNameFile.tcl options.ggo
