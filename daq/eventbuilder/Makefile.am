# bin_PROGRAMS=eventorderer

lib_LTLIBRARIES=libEventBuilder.la  libEventBuilderClient.la

#eventorderer_LDFLAGS= @TCL_LDFLAGS@ \
#		-Wl,"-rpath=@libdir@" \
#		@THREAD_LDFLAGS@


libEventBuilder_la_SOURCES = OrdererPackage.cpp CFragmentHandlerCommand.cpp CFragmentHandler.cpp \
	CInputStatsCommand.cpp COrdererOutput.cpp fragment.c


libEventBuilderClient_la_SOURCES=CEventOrderClient.cpp fragment.c \
	CEVBClientApp.cpp  CEVBFrameworkApp.cpp \
	EVBFramework.cpp


FRAMEWORK_SOURCEFILES =  GetOpt.cpp Main.cpp options.ggo

FRAMEWORK_SKELETON = Makefile_Skeleton $(FRAMEWORK_SOURCEFILES)


include_HEADERS = CEventOrderClient.h fragment.h CEVBClientApp.h EVBFramework.h \
	CEVBFrameworkApp.h  GetOpt.h


noinst_HEADERS = CFragmentHandlerCommand.h CFragmentHandler.h cmdline.h CInputStatsCommand.h COrdererOutput.h


INCLUDES = -D__STDC_LIMIT_MACROS \
	-I@top_srcdir@/base/tcpip \
	-I@top_srcdir@/base/exception  \
	-I@top_srcdir@/base/headers	\
	-I@top_srcdir@/base/tcpip        \
	-I@top_srcdir@/servers/portmanager \
	-I@top_srcdir@/base/os \
	-I@top_srcdir@/base/tclplus	\
	@THREADCXX_FLAGS@ @TCL_FLAGS@


libEventBuilderClient_la_LIBADD = @top_builddir@/base/tcpip/libTcp.la \
	@top_builddir@/servers/portmanager/libPortManager.la   \
	@top_builddir@/base/tcpip/libTcp.la	\
	@top_builddir@/base/os/libdaqshm.la		\
	@top_builddir@/base/exception/libException.la

libEventBuilder_la_LIBADD = @top_builddir@/base/tclplus/libtclPlus.la \
	@top_builddir@/base/exception/libException.la \
	@TCL_LDFLAGS@ @THREADLD_FLAGS@

TCL_PACKAGE_FILES=ConnectionManager.tcl callbackManager.tcl eventOrderer.tcl \
	connectionList.tcl

TCL_TEST_FILES=tclTests.tcl managerTests.test

EXTRA_DIST=protocol.xml api.xml Asserts.h \
	$(TCL_PACKAGE_FILE) \
	$(TCL_TEST_FILES) options.ggo \
	$(FRAMEWORK_SOURCEFILES) \
	Makefile_Skeleton.in


SKELDIR=@prefix@/skeletons/eventbuilder

install-exec-hook:
	$(mkinstalldirs) $(prefix)/TclLibs/EventBuilder
	$(INSTALL_SCRIPT) $(TCL_PACKAGE_FILES) $(prefix)/TclLibs/EventBuilder
	rm -f $(prefix)/TclLibs/EventBuilder/libEventBuilder.so
	$(LN_S) @libdir@/libEventBuilder.so $(prefix)/TclLibs/EventBuilder/libEventBuilder.so
	echo "pkg_mkIndex -verbose $(prefix)/TclLibs/EventBuilder *.tcl *.so" |@TCLSH_CMD@ 
	$(mkinstalldirs) $(SKELDIR)
	$(INSTALL_DATA) $(FRAMEWORK_SKELETON) $(SKELDIR)
	mv $(SKELDIR)/Makefile_Skeleton $(SKELDIR)/Makefile


GetOpt.o: GetOpt.cpp cmdline.h


#-------------------------------------------
#
# Tests:

noinst_PROGRAMS = unittests
unittests_SOURCES = TestRunner.cpp lookupTest.cpp  \
	connectTest.cpp GetOpt.cpp cmdline.c


unittests_LDADD = libEventBuilderClient.la \
	$(libEventBuilder_la_LIBADD) \
	@CPPUNIT_LDFLAGS@ @THREADLD_FLAGS@


unittests_DEPENDENCIES = cmdline.o cmdline.h

check-TESTS:
	TCLLIBPATH=@prefix@/TclLibs @TCLSH_CMD@ tclTests.tcl
#	./unittests 


cmdline.h: options.ggo
	$(GENGETOPT) --input=options.ggo

cmdline.c: options.ggo
	$(GENGETOPT) --input=options.ggo

GetOpt.o: cmdline.c cmdline.h GetOpt.cpp