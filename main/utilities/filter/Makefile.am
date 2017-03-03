#-------------- The library libdataformat

SUBDIRS = V10 V11 V12

lib_LTLIBRARIES	=	libfilter.la

libfilter_la_SOURCES = CFilterMain.cpp \
                       CBaseMediator.cpp \
                       CMediator.cpp \
                       CFakeMediator.cpp \
                       CInfiniteMediator.cpp \
                       COneShotMediator.cpp \
                       COneShotHandler.cpp \
                       COneShotException.cpp \
                       CMediatorException.cpp \
                       CAbnormalEndRunFilterHandler.cpp \
			CRunStatistics.cpp  \
                        CPredicatedMediator.cpp \
                        CCompositePredicate.cpp \
                        CTestPredicate.cpp \
                         SatisfyTclPlus.cpp  \
                         CFilterVersionAbstractionFactory.cpp

nodist_libfilter_la_SOURCES=filterargs.c filterargs.h

#                       COneShotMediator.cpp 

BUILT_SOURCES = filterargs.c filterargs.h

include_HEADERS	=  CFilterMain.h \
		CBaseMediator.h \
                   CMediator.h \
		 CFakeMediator.h \
                   CInfiniteMediator.h \
		 COneShotMediator.h \
                   COneShotHandler.h \
                   COneShotException.h \
                   CMediatorException.h \
                   CFatalException.h \
                   CAbnormalEndRunFilterHandler.h \
                 CRunStatistics.h \
                 CPredicatedMediator.h \
                 CPredicate.h \
                 CCompositePredicate.h \
                 CTestPredicate.h \
                 CFilterVersionAbstraction.h \
                 CFilterVersionAbstractionFactory.h



#                   COneShotMediator.h 

libfilter_la_CPPFLAGS	= -I@top_srcdir@/utilities/common \
		-I@top_srcdir@/utilities/IO \
                -I@top_srcdir@/base/uri		\
                -I@top_srcdir@/base/os \
                -I@top_srcdir@/base/headers \
                @LIBTCLPLUS_CFLAGS@



libfilter_la_LDFLAGS	= -version-info $(SOVERSION)	\
				-Wl,"-rpath-link=$(libdir)" -lrt \
				$(THREADLD_FLAGS)

libfilter_la_LIBADD	= @top_builddir@/utilities/common/libUtilCommon.la \
			@top_builddir@/utilities/IO/libdaqio.la \
			@top_builddir@/base/dataflow/libDataFlow.la \
			@top_builddir@/base/uri/liburl.la		\
			@top_builddir@/base/os/libdaqshm.la		\
			@LIBEXCEPTION_LDFLAGS@


libfilter_la_CXXFLAGS = $(THREADCXX_FLAGS) $(AM_CXXFLAGS)



filterargs.c: filterargs.h

filterargs.h: filterargs.ggo 
	$(GENGETOPT) < @srcdir@/filterargs.ggo \
		--output-dir=@builddir@ --file=filterargs --unamed-opts="OPTIONS"




#------------------- Tests:

noinst_PROGRAMS = unittests testapp

unittests_SOURCES	= TestRunner.cpp  \
						filtermaintests.cpp  \
                                                abnormalendrunfiltertests.cpp \
                                                cfilterversionabstractionfactorytests.cpp


#						oneshothandlertests.cpp \
#                                                infinitemediatortests.cpp

unittests_LDADD		= \
			@builddir@/libfilter.la	\
			@top_builddir@/utilities/IO/libdaqio.la \
                        @top_builddir@/utilities/nscldaq-format/format/V10/libdataformatv10.la \
                        @top_builddir@/utilities/nscldaq-format/format/V11/libdataformatv11.la \
                        @top_builddir@/utilities/nscldaq-format/format/V12/libdataformatv12.la \
                        @top_builddir@/utilities/nscldaq-format/FormattedIO/libdaqformatio.la \
                        @top_builddir@/utilities/nscldaq-format/Buffer/libbuffer.la \
                        @top_builddir@/base/dataflow/libDataFlow.la 	\
			@top_builddir@/base/uri/liburl.la 	\
                        $(CPPUNIT_LDFLAGS) 		\
			@LIBEXCEPTION_LDFLAGS@

unittests_CPPFLAGS=  -I@top_srcdir@/utilities/nscldaq-format/format \
                    -I@top_srcdir@/utilities/nscldaq-format/FormattedIO \
                    -I@top_srcdir@/utilities/nscldaq-format/Buffer \
                    -I@top_srcdir@/utilities/IO \
                    -I@top_srcdir@/base/uri		\
                    -I@top_srcdir@/base/headers		\
                    -I@top_srcdir@/base/dataflow \
                    -I@top_srcdir@/base/testutils \
                    @LIBTCLPLUS_CFLAGS@

unittests_CXXFLAGS = $(AM_CXXFLAGS)

unittests_LDFLAGS	= -Wl,"-rpath-link=$(libdir)"


testapp_SOURCES = TestApp.cpp
testapp_CPPFLAGS= -I@top_srcdir@/utilities \
                    -I@top_srcdir@/utilities/nscldaq-format/format \
                    -I@top_srcdir@/utilities/nscldaq-format/FormattedIO \
                    -I@top_srcdir@/utilities/nscldaq-format/Buffer \
                    -I@top_srcdir@/base/uri		\
                    -I@top_srcdir@/base/dataflow \
                    @LIBTCLPLUS_CFLAGS@

testapp_LDADD = -L$(libdir) $(CPPUNIT_LDFLAGS) 		\
                        @top_builddir@/utilities/IO/libdaqio.la \
			@top_builddir@/base/dataflow/libDataFlow.la 	\
			@top_builddir@/base/uri/liburl.la 	\
                        @top_builddir@/utilities/nscldaq-format/format/V10/libdataformatv10.la \
                        @top_builddir@/utilities/nscldaq-format/format/V11/libdataformatv11.la \
                        @top_builddir@/utilities/nscldaq-format/format/V12/libdataformatv12.la \
                        @top_builddir@/utilities/nscldaq-format/FormattedIO/libdaqformatio.la \
                        @top_builddir@/utilities/nscldaq-format/Buffer/libbuffer.la \
			@builddir@/libfilter.la	\
			@LIBEXCEPTION_LDFLAGS@
testapp_LDFLAGS	= -Wl,"-rpath-link=$(libdir)"


TESTS=./unittests

#-------------------- Filter Kit

install-data-local:
	$(mkinstalldirs) @prefix@/filterkit
	$(INSTALL_DATA) FilterIncludes \
		@srcdir@/CTemplateFilter.cpp \
		@srcdir@/SkeletonMain.cpp @prefix@/filterkit
	$(INSTALL_DATA) Makefile-template @prefix@/filterkit/Makefile

EXTRA_DIST = filterargs.ggo SkeletonMain.cpp CTemplateFilter.cpp \
             Makefile-template.in FilterIncludes.in
