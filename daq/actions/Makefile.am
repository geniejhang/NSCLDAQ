lib_LTLIBRARIES=libActions.la

libActions_la_SOURCES=Actions.cpp
libActions_la_CPPFLAGS = -I@top_srcdir@/base/thread @LIBTCLPLUS_CFLAGS@
libActions_la_LDFLAGS  = @top_builddir@/base/thread/libdaqthreads.la @LIBEXCEPTION_LDFLAGS@
include_HEADERS = Actions.h

install-exec-local:
	$(mkinstalldirs) $(prefix)/bin
	$(mkinstalldirs) $(prefix)/TclLibs/Stager
	$(mkinstalldirs) @prefix@/share/html
	for f in  $(TCLPACKAGES); do install @srcdir@/$$f $(prefix)/TclLibs/Stager; done
	echo "pkg_mkIndex -verbose $(prefix)/TclLibs/Stager *.tcl" | $(TCLSH_CMD)

TCLPACKAGES=actions.tcl defaultActions.tcl

TCLTESTS =    actions.test tclTests.tcl


check-TESTS:
	TCLLIBPATH=@prefix@/TclLibs HERE=@srcdir@ tcl=@TCLSH_CMD@ @TCLSH_CMD@ @srcdir@/tclTests.tcl

EXTRA_DIST = $(TCLPACKAGES) $(TCLTESTS) $(CXXPACKAGES)
