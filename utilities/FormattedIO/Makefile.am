#-------------- The library libdataformat

lib_LTLIBRARIES	=	libdaqformatio.la

libdaqformatio_la_SOURCES = RingIOV10.cpp \
											RingIOV11.cpp
include_HEADERS	= RingIOV10.h \
									RingIOV11.h

libdaqformatio_la_CPPFLAGS	=  \
                -I@top_srcdir@/utilities/IO \
                -I@top_srcdir@/utilities/conversion \
		-I@top_srcdir@/base/headers \
    @LIBTCLPLUS_CFLAGS@	

libdaqformatio_la_LDFLAGS	= -version-info $(SOVERSION)	\
				-Wl,"-rpath-link=$(libdir)" -lrt \
				$(THREADLD_FLAGS)

libdaqformatio_la_LIBADD	= \
			@top_builddir@/utilities/IO/libdaqio.la \
			@top_builddir@/utilities/conversion/NSCLDAQ10/libdataformatv10.la \
			@top_builddir@/utilities/conversion/NSCLDAQ11/libdataformatv11.la \
			@LIBEXCEPTION_LDFLAGS@


libdaqformatio_la_CXXFLAGS = $(THREADCXX_FLAGS) $(AM_CXXFLAGS)


------------------- Tests:

noinst_PROGRAMS = unittests

unittests_SOURCES	= TestRunner.cpp  \
										daq10test.cpp \
										daq11test.cpp

unittests_LDADD		= \
			@builddir@/libdaqformatio.la \
			@top_builddir@/utilities/FormattedIO/libdaqformatio.la \
			@top_builddir@/utilities/IO/libdaqio.la \
			@top_builddir@/utilities/conversion/NSCLDAQ10/libdataformatv10.la \
			@top_builddir@/utilities/conversion/NSCLDAQ11/libdataformatv11.la \
			$(CPPUNIT_LDFLAGS) 		\
			@LIBEXCEPTION_LDFLAGS@

unittests_CPPFLAGS=  \
										 -I@top_srcdir@/utilities/FormattedIO \
										 -I@top_srcdir@/utilities/IO \
										 -I@top_srcdir@/utilities/conversion \
										 @LIBTCLPLUS_CFLAGS@	
unittests_CXXFLAGS = $(AM_CXXFLAGS)

unittests_LDFLAGS	= -Wl,"-rpath-link=$(libdir)"

TESTS=./unittests


