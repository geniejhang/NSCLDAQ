lib_LTLIBRARIES = libdataformatv8.la

libdataformatv8_la_SOURCES = bheader.cpp \
                                CV8Buffer.cpp \
                                CRawBuffer.cpp \
                                CPhysicsEventBuffer.cpp \
                                CScalerBuffer.cpp \
                                CPhysicsEventBodyParser.cpp \
                                CStandardBodyParser.cpp \
                                CControlBuffer.cpp \
                                CTextBuffer.cpp

nscldaq8dir = @includedir@/NSCLDAQ8

nscldaq8_HEADERS = DataFormatV8.h \
                    bheader.h \
                    CV8Buffer.h \
                    CRawBuffer.h \
                    CPhysicsEventBuffer.h \
                    CScalerBuffer.h \
                    CPhysicsEventBodyParser.h \
                    CStandardBodyParser.h \
                    CControlBuffer.h \
                    CTextBuffer.h \
										format_cast.h

libdataformatv8_la_CXXFLAGS = -I@srcdir@ \
                              -I@top_srcdir@/utilities/Buffer \
                               @LIBTCLPLUS_CFLAGS@

libdataformatv8_la_LDFLAGS = @LIBTCLPLUS_LDFLAGS@ \
                             @top_builddir@/utilities/Buffer/libbuffer.la

noinst_PROGRAMS = unittests

unittests_SOURCES	= TestRunner.cpp \
                            ccontrolbuffertest.cpp \
                            physicseventtest.cpp \
                            standardbodyparsertest.cpp \
                            scalerbuffertest.cpp \
                            format_casttest.cpp \
                            rawbuffertest.cpp \
                            ctextbuffertest.cpp

unittests_LDADD	= $(CPPUNIT_LDFLAGS) 		\
                  @builddir@/libdataformatv8.la	\
                  @top_builddir@/utilities/Buffer/libbuffer.la \
                  @LIBEXCEPTION_LDFLAGS@

unittests_CPPFLAGS= -I@srcdir@ \
                    -I@top_srcdir@/utilities/Buffer \
                    @CPPUNIT_CFLAGS@ \
                    @LIBTCLPLUS_CFLAGS@ \
                    @TCL_FLAGS@ \
                    -I@top_srcdir@/base/testutils \
                    -I@top_srcdir@/base/headers

unittests_LDFLAGS	= @CPPUNIT_LDFLAGS@ \
                          @TCL_LDFLAGS@ \
                          @LIBEXCEPTION_LDFLAGS@ \
                          -Wl,"-rpath-link=$(libdir)"


TESTS=./unittests
