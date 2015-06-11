lib_LTLIBRARIES = libdataformatv10.la

libdataformatv10_la_SOURCES = CRingItem.cpp \
                           CRingScalerItem.cpp \
                           CRingStateChangeItem.cpp \
                           CPhysicsEventItem.cpp \
                           CRingPhysicsEventCountItem.cpp \
                           CRingTextItem.cpp \
                           ringitem.cpp


include_HEADERS = CRingItem.h \
                  CRingScalerItem.h \
                  CRingStateChangeItem.h \
                  CPhysicsEventItem.h \
                  CRingPhysicsEventCountItem.h \
                  CRingTextItem.h \
                  DataFormatV10.h

libdataformatv10_la_CXXFLAGS = -I@srcdir@ \
                               @LIBTCLPLUS_CFLAGS@

libdataformatv10_la_LDFLAGS = @LIBTCLPLUS_LDFLAGS@

noinst_PROGRAMS = unittests

unittests_SOURCES	= TestRunner.cpp \
                                                                                ringitemtests.cpp teststate.cpp	\
                                                                                scalertests.cpp texttest.cpp physcounttests.cpp		\
                                                                                formatoutputtests.cpp

unittests_LDADD	= -L$(libdir) $(CPPUNIT_LDFLAGS) 		\
                        @top_builddir@/utilities/conversion/NSCLDAQ10/libdataformatv10.la	\
                        @LIBEXCEPTION_LDFLAGS@

unittests_CPPFLAGS= @CPPUNIT_CFLAGS@ \
                    @LIBTCLPLUS_CFLAGS@ \
                    @TCL_FLAGS@ \
                    -I@top_srcdir@/base/testutils

unittests_LDFLAGS	= @CPPUNIT_LDFLAGS@ \
                            @TCL_LDFLAGS@ \
                          @LIBEXCEPTION_LDFLAGS@ \
                          -Wl,"-rpath-link=$(libdir)"


TESTS=./unittests
