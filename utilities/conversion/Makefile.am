lib_LTLIBRARIES = libConversion.la

SUBDIRS = NSCLDAQ10 NSCLDAQ11

libConversion_la_SOURCES = CTransformMediator.h \
                           CTransform10p0to11p0.cpp \
                           CTransform11p0to10p0.cpp

                           #CTransform11p0to11p0.cpp

include_HEADERS = CTransformMediator.h \
                           CTransform10p0to11p0.h

#                           CTransform11p0to11p0.h

libConversion_la_CXXFLAGS = -I@srcdir@ \
                            -I@srcdir@/NSCLDAQ10 \
                            -I@srcdir@/NSCLDAQ11 \
                            @LIBTCLPLUS_CFLAGS@

libConversion_la_LIBADD = @LIBTCLPLUS_LDFLAGS@

noinst_PROGRAMS = unittests

unittests_SOURCES = TestRunner.cpp \
                    ctransform10p0to11p0tests.cpp \
                    ctransform11p0to10p0tests.cpp

unittests_LDADD	= -L$(libdir) $(CPPUNIT_LDFLAGS) 		\
                   @top_builddir@/utilities/conversion/NSCLDAQ10/libdataformatv10.la	\
                   @top_builddir@/utilities/conversion/NSCLDAQ11/libdataformatv11.la	\
                   @top_builddir@/utilities/conversion/libConversion.la	\
                   @LIBEXCEPTION_LDFLAGS@

unittests_CPPFLAGS= -I@srcdir@ \
                    -I@srcdir@/NSCLDAQ10 \
                    -I@srcdir@/NSCLDAQ11 \
                    @CPPUNIT_CFLAGS@ \
                    @LIBTCLPLUS_CFLAGS@ \
                    @TCL_FLAGS@ \
                    -I@top_srcdir@/base/testutils

unittests_LDFLAGS	= @CPPUNIT_LDFLAGS@ \
                            @TCL_LDFLAGS@ \
                          @LIBEXCEPTION_LDFLAGS@ \
                          -Wl,"-rpath-link=$(libdir)"


TESTS=./unittests
