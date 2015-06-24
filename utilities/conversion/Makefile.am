lib_LTLIBRARIES = libConversion.la

libConversion_la_SOURCES = CTransformMediator.h \
                           CTransformMediator.hpp \
                           CTransform10p0to11p0.cpp \
                           CTransform11p0to10p0.cpp \
                           CTransformFactory.cpp \
													CCompositePredicate.cpp 

                           #CTransform11p0to11p0.cpp

include_HEADERS = CTransformMediator.h \
                           CTransform10p0to11p0.h \
                           CTransform11p0to10p0.h \
                           CTransformFactory.h \
                                                                                                         CPredicate.h  \
													 CCompositePredicate.h 

#                           CTransform11p0to11p0.h

libConversion_la_CXXFLAGS = -I@srcdir@ \
                            -I@top_srcdir@/utilities/format/NSCLDAQ10 \
                            -I@top_srcdir@/utilities/format/NSCLDAQ11 \
                            -I@top_srcdir@/utilities/filter \
                            -I@top_srcdir@/utilities/IO \
                            -I@top_srcdir@/utilities/FormattedIO \
                            -I@top_srcdir@/utilities/format \
                            @LIBTCLPLUS_CFLAGS@

libConversion_la_LIBADD = @LIBTCLPLUS_LDFLAGS@

noinst_PROGRAMS = unittests testApp

unittests_SOURCES = TestRunner.cpp \
                    ctransform10p0to11p0tests.cpp \
                    ctransform11p0to10p0tests.cpp \
                    ctransformfactorytests.cpp

unittests_LDADD	= -L$(libdir) $(CPPUNIT_LDFLAGS) 		\
                   @top_builddir@/utilities/conversion/libConversion.la	\
                   @top_builddir@/utilities/format/NSCLDAQ10/libdataformatv10.la	\
                   @top_builddir@/utilities/format/NSCLDAQ11/libdataformatv11.la	\
                   @top_builddir@/utilities/IO/libdaqio.la	\
                   @top_builddir@/utilities/FormattedIO/libdaqformatio.la	\
                   @top_builddir@/utilities/filter/libfilter.la	\
                   @LIBEXCEPTION_LDFLAGS@

unittests_CPPFLAGS= -I@srcdir@ \
                    -I@top_srcdir@/utilities/format/NSCLDAQ10 \
                    -I@top_srcdir@/utilities/format/NSCLDAQ11 \
                    -I@top_srcdir@/utilities/filter \
                    -I@top_srcdir@/utilities/IO \
                    -I@top_srcdir@/utilities/FormattedIO \
                    -I@top_srcdir@/utilities/format \
                    @CPPUNIT_CFLAGS@ \
                    @LIBTCLPLUS_CFLAGS@ \
                    @TCL_FLAGS@ \
                    -I@top_srcdir@/base/testutils

unittests_LDFLAGS	= @CPPUNIT_LDFLAGS@ \
                            @TCL_LDFLAGS@ \
                          @LIBEXCEPTION_LDFLAGS@ \
                          -Wl,"-rpath-link=$(libdir)"

TESTS=./unittests

testApp_SOURCES = Main.cpp

testApp_LDADD = @builddir@/libConversion.la \
								@top_builddir@/utilities/format/NSCLDAQ10/libdataformatv10.la \
								@top_builddir@/utilities/format/NSCLDAQ11/libdataformatv11.la \
								@top_builddir@/utilities/filter/libfilter.la \
								@top_builddir@/utilities/IO/libdaqio.la \
								@top_builddir@/utilities/FormattedIO/libdaqformatio.la

testApp_CPPFLAGS = -I@srcdir@ \
									 -I@top_srcdir@/utilities/format/NSCLDAQ10 \
									 -I@top_srcdir@/utilities/format/NSCLDAQ11 \
									 -I@top_srcdir@/utilities/filter \
									 -I@top_srcdir@/utilities/IO \
                                                                         -I@top_srcdir@/utilities/FormattedIO \
                                                                         -I@top_srcdir@/utilities/format
