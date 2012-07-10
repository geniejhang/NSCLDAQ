# bin_PROGRAMS=eventorderer

lib_LTLIBRARIES=libEventBuilder.la

#eventorderer_LDFLAGS= @TCL_LDFLAGS@ \
#		-Wl,"-rpath=@libdir@" \
#		@THREAD_LDFLAGS@


libEventBuilder_la_SOURCES=CEventOrderClient.cpp fragment.c

include_HEADERS = CEventOrderClient.h fragment.h 

INCLUDES = -I@top_srcdir@/base/tcpip \
	-I@top_srcdir@/base/exception  \
	-I@top_srcdir@/base/headers	\
	-I@top_srcdir@/base/tcpip        \
	-I@top_srcdir@/servers/portmanager \
	-I@top_srcdir@/base/os
	@THREADCXX_FLAGS@


libEventBuilder_la_LIBADD = @top_builddir@/base/tcpip/libTcp.la \
	@top_builddir@/servers/portmanager/libPortManager.la   \
	@top_builddir@/base/tcpip/libTcp.la	\
	@top_builddir@/base/os/libdaqshm.la		\
	@top_builddir@/base/exception/libException.la

TCL_PACKAGE_FILES=ConnectionManager.tcl callbackManager.tcl eventOrderer.tcl \
	connectionList.tcl

TCL_TEST_FILES=tclTests.tcl managerTests.test

EXTRA_DIST=protocol.xml api.xml Asserts.h \
	$(TCL_PACKAGE_FILE) \
	$(TCL_TEST_FILES)


install-exec-local:
	$(mkinstalldirs) $(prefix)/TclLibs/EventBuilder
	$(INSTALL_SCRIPT) $(TCL_PACKAGE_FILES) $(prefix)/TclLibs/EventBuilder
	echo "pkg_mkIndex -verbose $(prefix)/TclLibs/EventBuilder *.tcl" |@TCLSH_CMD@



#-------------------------------------------
#
# Tests:

noinst_PROGRAMS = unittests
unittests_SOURCES = TestRunner.cpp lookupTest.cpp  \
	connectTest.cpp 

unittests_LDADD = libEventBuilder.la \
	$(libEventBuilder_la_LIBADD) \
	@CPPUNIT_LDFLAGS@ @THREADLD_FLAGS@

check-TESTS:
	TCLLIBPATH=@prefix@/TclLibs @TCLSH_CMD@ tclTests.tcl
#	./unittests 
