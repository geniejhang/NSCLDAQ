bin_PROGRAMS=glom
noinst_PROGRAMS=unittests

noinst_HEADERS = CGlom.h

glom_SOURCES= glomMain.cpp CGlom.cpp
nodist_glom_SOURCES=glom.c glom.h

BUILT_SOURCES = glom.c glom.h


glom_CPPFLAGS = -I@top_srcdir@/daq/eventbuilder \
        -I@top_srcdir@/utilities/nscldaq-format/format          \
        -I@top_srcdir@/utilities/nscldaq-format/Buffer          \
        -I@top_srcdir@/utilities/nscldaq-format/FormattedIO          \
        -I@top_srcdir@/utilities/IO \
        -I@top_srcdir@/base/os \
        -I@top_srcdir@/base/sfinae_tools

glom_LDADD = @top_builddir@/daq/eventbuilder/libEventBuilderClient.la  \
        @top_builddir@/utilities/nscldaq-format/format/V12/libdataformatv12.la                     \
        @top_builddir@/utilities/nscldaq-format/Buffer/libbuffer.la                     \
        @top_builddir@/utilities/nscldaq-format/FormattedIO/libdaqformatio.la                     \
        @top_builddir@/utilities/IO/libdaqio.la                     \
        @top_builddir@/base/os/libdaqshm.la


glom.c:  glom.h

glom.h: @srcdir@/glom.ggo
	@GENGETOPT@ --input=@srcdir@/glom.ggo \
		--output-dir=@builddir@ --file-name=glom



unittests_SOURCES = TestRunner.cpp glomtest.cpp CGlom.cpp glom.c
unittests_CPPFLAGS = $(glom_CPPFLAGS) \
                    -I@top_srcdir@/base/testutils \
                    @CPPUNIT_CFLAGS@

unittests_LDADD = $(glom_LDADD) @CPPUNIT_LDFLAGS@

TESTS = unittests

EXTRA_DIST=glom.ggo glom.xml
