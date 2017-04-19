bin_PROGRAMS=unglom
noinst_PROGRAMS = unittests


noinst_HEADERS = CUnglom.h
unglom_SOURCES=unglom.cpp @srcdir@/cmdline.c CUnglom.cpp

unglom_CPPFLAGS=-I@top_srcdir@/daq/eventbuilder \
        -I@top_srcdir@/utilities/nscldaq-format/format       \
        -I@top_srcdir@/utilities/nscldaq-format/Buffer       \
        -I@top_srcdir@/utilities/nscldaq-format/FormattedIO       \
        -I@top_srcdir@/utilities/IO       \
	@LIBTCLPLUS_CFLAGS@	\
	-I@top_srcdir@/base/os		\
	-I@top_srcdir@/base/headers  

unglom_LDADD = @top_builddir@/utilities/nscldaq-format/format/V12/libdataformatv12.la  \
        @top_builddir@/utilities/nscldaq-format/Buffer/libbuffer.la \
        @top_builddir@/utilities/nscldaq-format/FormattedIO/libdaqformatio.la \
        @top_builddir@/utilities/IO/libdaqio.la \
        @top_builddir@/daq/eventbuilder/libEventBuilder.la \
	@top_builddir@/daq/eventbuilder/libEventBuilderClient.la \
	@top_builddir@/base/os/libdaqshm.la	\
	@LIBTCLPLUS_LDFLAGS@

BUILT_SOURCES = @srcdir@/cmdline.c @srcdir@/cmdline.h

@srcdir@/cmdline.c: @srcdir@/cmdline.h

@srcdir@/cmdline.h: @srcdir@/cmdline.ggo
	@GENGETOPT@ --input=@srcdir@/cmdline.ggo \
		--output-dir=@srcdir@


unittests_SOURCES = TestRunner.cpp unglomtest.cpp CUnglom.cpp cmdline.c
unittests_CPPFLAGS = $(unglom_CPPFLAGS) -I@top_srcdir@/base/testutils @CPPUNIT_CFLAGS@
unittests_LDADD = $(unglom_LDADD) @CPPUNIT_LDFLAGS@

TESTS = unittests

EXTRA_DIST=cmdline.ggo unglom.xml
