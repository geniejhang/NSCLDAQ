
nscldaq11dir=@includedir@/V11

lib_LTLIBRARIES = libfilterv11.la

nscldaq11_HEADERS = CFilter.h \
                    CCompositeFilter.h \
                    CTransparentFilter.h \
                    CNullFilter.h \
                    CFilterAbstraction.h \
                CAbnormalEndRunFilterHandler.h  \
                COneShotLogicFilter.h



libfilterv11_la_SOURCES = CCompositeFilter.cpp \
                            CFilterAbstraction.cpp \
                        CAbnormalEndRunFilterHandler.cpp \
                        COneShotLogicFilter.cpp

libfilterv11_la_CPPFLAGS = -I@srcdir@/.. \
                            -I@top_srcdir@/utilities/filter/core \
                            -I@top_srcdir@/utilities/IO \
                            -I@top_srcdir@/utilities/nscldaq-format/format \
                            -I@top_srcdir@/utilities/nscldaq-format/FormattedIO


libfilterv11_la_LIBADD = @top_builddir@/utilities/IO/libdaqio.la \
                            @top_builddir@/utilities/filter/core/libfilter.la \
                            @top_builddir@/utilities/nscldaq-format/format/V11/libdataformatv11.la \
                            @top_builddir@/utilities/nscldaq-format/FormattedIO/libdaqformatio.la


#------------------- Tests:

noinst_PROGRAMS = unittests

unittests_SOURCES	= TestRunner.cpp  \
                                                compositefiltertests.cpp \
                                                transparentfiltertests.cpp \
                                                cfilterabstractiontests.cpp \
                                                abnormalendrunfiltertests.cpp \
                                                coneshotlogicfiltertests.cpp \
                                                CTestFilter.h \
                                                CTestFilter.cpp


unittests_LDADD		= \
                        @builddir@/libfilterv11.la	\
                        @top_builddir@/utilities/IO/libdaqio.la \
                        @top_builddir@/utilities/filter/core/libfilter.la \
                        @top_builddir@/utilities/nscldaq-format/format/V11/libdataformatv11.la \
                        @top_builddir@/utilities/nscldaq-format/Buffer/libbuffer.la \
                        $(CPPUNIT_LDFLAGS) 		\
                        @LIBEXCEPTION_LDFLAGS@

unittests_CPPFLAGS= -I@top_srcdir@/utilities/filter \
                    -I@top_srcdir@/utilities/filter/core \
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
