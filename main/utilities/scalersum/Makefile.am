bin_PROGRAMS = sumscaler

sumscaler_SOURCES=   main.cpp App.cpp App.h  CRun.h CRun.cpp \
	CChannel.h CIncrementalChannel.h CIncrementalChannel.cpp \
	CCumulativeChannel.h CCumulativeChannel.cpp

nodist_sumscaler_SOURCES=options.c options.h

sumscaler_CPPFLAGS=-I@top_srcdir@/utilities/IO 			\
		    -I@top_srcdir@/utilities/nscldaq-format/format			\
		    -I@top_srcdir@/utilities/nscldaq-format/Buffer			\
		    -I@top_srcdir@/utilities/nscldaq-format/FormattedIO			\
		    -I@top_srcdir@/utilities/IO			\
		    -I@top_srcdir@/base/os			\
                    -I@top_srcdir@/base/sfinae_tools \
		    @LIBTCLPLUS_CFLAGS@

sumscaler_LDFLAGS = @top_builddir@/utilities/IO/libdaqio.la 	\
    @top_builddir@/utilities/nscldaq-format/format/V12/libdataformatv12.la			\
    @top_builddir@/utilities/nscldaq-format/Buffer/libbuffer.la \
    @top_builddir@/utilities/nscldaq-format/FormattedIO/libdaqformatio.la \
    @top_builddir@/base/os/libdaqshm.la \
    @LIBEXCEPTION_LDFLAGS@

BUILT_SOURCES	= options.c options.h

options.c: options.h

options.h: options.ggo
	$(GENGETOPT) < @srcdir@/options.ggo --file=options \
	    --unamed-opts

install-exec-hook:
	$(INSTALL_SCRIPT) @srcdir@/makeNameFile.tcl @bindir@/makeNameFile

clean-local:
	-rm -rf options.h options.c

noinst_PROGRAMS=unittests

unittests_SOURCES=TestRunner.cpp channelTests.cpp runTests.cpp \
									CRun.cpp CCumulativeChannel.cpp CIncrementalChannel.cpp 

unittests_CPPFLAGS=$(sumscaler_CPPFLAGS) -I@top_srcdir@/base/testutils @CPPUNIT_CFLAGS@
unittests_LDFLAGS=$(sumscaler_LDFLAGS)   @CPPUNIT_LDFLAGS@

TESTS = unittests

EXTRA_DIST = makeNameFile.tcl options.ggo
