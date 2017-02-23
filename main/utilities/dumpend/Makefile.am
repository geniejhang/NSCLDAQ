bin_PROGRAMS =dumpend

COMMON_CFLAGS = -I@top_srcdir@/utilities/nscldaq-format/format \
                -I@top_srcdir@/utilities/nscldaq-format/FormattedIO \
                -I@top_srcdir@/base/os \
                -I@top_srcdir@/utilities/IO  \
                @LIBTCLPLUS_CFLAGS@
COMMON_LOADFLAGS= @top_builddir@/utilities/nscldaq-format/format/V10/libdataformatv10.la \
                    @top_builddir@/utilities/nscldaq-format/format/V11/libdataformatv11.la \
                    @top_builddir@/utilities/nscldaq-format/format/V12/libdataformatv12.la \
                    @top_builddir@/utilities/nscldaq-format/FormattedIO/libdaqformatio.la \
                    @top_builddir@/base/dataflow/libDataFlow.la \
                    @top_builddir@/base/os/libdaqshm.la \
                    @top_builddir@/utilities/IO/libdaqio.la	\
	@LIBEXCEPTION_LDFLAGS@

dumpend_SOURCES = CFileIterator.cpp CFileIterator.h main.cpp CEndRunInfo.cpp CEndRunInfo.h \
	CEndRunInfo11.cpp CEndRunInfo11.h CEndRunInfo10.cpp CEndRunInfo10.h \
        CEndRunInfo12.h CEndRunInfo12.cpp CEndRunInfoFactory.cpp CEndRunInfoFactory.h  \
	Application.cpp Application.h

nodist_dumpend_SOURCES=options.c options.h

dumpend_CXXFLAGS=$(COMMON_CFLAGS)
dumpend_LDADD=$(COMMON_LOADFLAGS)


# gengetopt stuff:

BUILT_SOURCES=options.c options.h

CLEAN_FILES=options.c options.h

options.c: options.h


options.h: options.ggo
	$(GENGETOPT) < @srcdir@/options.ggo \
	--output-dir=@builddir@ --file options --unamed-opts=FILE 


noinst_PROGRAMS=unittests

unittests_SOURCES=TestRunner.cpp fileitertest.cpp \
	endinfo11test.cpp endrun10test.cpp	\
	erinfofactorytest.cpp			\
	CFileIterator.cpp CFileIterator.h \
	CEndRunInfo.cpp CEndRunInfo.h	\
	CEndRunInfo11.cpp CEndRunInfo11.h \
	CEndRunInfo10.cpp CEndRunInfo10.h \
        CEndRunInfo12.cpp CEndRunInfo12.h \
	CEndRunInfoFactory.cpp CEndRunInfoFactory.h

unittests_CXXFLAGS=$(CPPUNIT_CFLAGS) $(COMMON_CFLAGS) -I@top_srcdir@/base/os
unittests_LDADD=$(CPPUNIT_LDFLAGS) @top_builddir@/base/os/libdaqshm.la $(COMMON_LOADFLAGS)

EXTRA_DIST=options.ggo

TESTS=unittests
