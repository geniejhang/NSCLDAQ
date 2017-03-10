
nscldaq12dir=@includedir@/V12

lib_LTLIBRARIES = libfilterv12.la

nscldaq12_HEADERS = CFilter.h \
                    CCompositeFilter.h \
                    CNullFilter.h \
                    CFilterAbstraction.h \
                    CAbnormalEndRunFilterHandler.h



libfilterv12_la_SOURCES = CCompositeFilter.cpp \
                            CFilterAbstraction.cpp \
                            CAbnormalEndRunFilterHandler.cpp

libfilterv12_la_CPPFLAGS = -I@srcdir@/.. \
                            -I@top_srcdir@/utilities/IO \
                            -I@top_srcdir@/utilities/nscldaq-format/format \
                            -I@top_srcdir@/utilities/nscldaq-format/FormattedIO \
                            -I@top_srcdir@/utilities/nscldaq-format/Buffer \
                            -I@top_srcdir@/base/headers

libfilterv12_la_LIBADD = @top_builddir@/utilities/IO/libdaqio.la \
                            @top_builddir@/utilities/IO/libdaqio.la \
                            @top_builddir@/utilities/nscldaq-format/format/V12/libdataformatv12.la \
                            @top_builddir@/utilities/nscldaq-format/Buffer/libbuffer.la



#------------------- Tests:

noinst_PROGRAMS = unittests

unittests_SOURCES	= TestRunner.cpp  \
                                                compositefiltertests.cpp \
                                                abnormalendrunfiltertests.cpp \
                                                CTestFilter.h \
                                                CTestFilter.cpp


unittests_LDADD		= \
                        @builddir@/libfilterv12.la	\
                        @top_builddir@/utilities/IO/libdaqio.la \
                        @top_builddir@/utilities/nscldaq-format/format/V12/libdataformatv12.la \
                        @top_builddir@/utilities/nscldaq-format/FormattedIO/libdaqformatio.la \
                        @top_builddir@/utilities/nscldaq-format/Buffer/libbuffer.la \
                        $(CPPUNIT_LDFLAGS) 		\
                        @LIBEXCEPTION_LDFLAGS@

unittests_CPPFLAGS= -I@top_srcdir@/utilities/filter \
                    -I@top_srcdir@/utilities/nscldaq-format/format \
                    -I@top_srcdir@/utilities/nscldaq-format/FormattedIO \
                    -I@top_srcdir@/utilities/nscldaq-format/Buffer \
                    -I@top_srcdir@/utilities/IO \
                    @LIBTCLPLUS_CFLAGS@	\
                    -I@top_srcdir@/base/uri		\
                    -I@top_srcdir@/base/headers		\
                    -I@top_srcdir@/base/dataflow \
                    -I@top_srcdir@/base/testutils

unittests_CXXFLAGS = $(AM_CXXFLAGS)

unittests_LDFLAGS	= -Wl,"-rpath-link=$(libdir)"

TESTS = unittests
