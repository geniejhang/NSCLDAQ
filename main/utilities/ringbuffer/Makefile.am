TCL_PACKAGES=dbringstatus.tcl
TCL_TESTS  = dbringbuffer.test
TCL_PKGDIR = @prefix@/TclLibs/vardbringbuffer

install-exec-local:
	$(INSTALL_SCRIPT) @srcdir@/ringbuffer.tcl @bindir@/ringbuffer
	$(mkinstalldirs) $(TCL_PKGDIR)
	$(INSTALL_SCRIPT) $(TCL_PACKAGES) $(TCL_PKGDIR)
	echo "pkg_mkIndex -verbose  $(TCL_PKGDIR) *.tcl *.so" | @TCLSH@



TEST_EXTENSIONS=.test .py
TEST_LOG_COMPILER=@TCLSH@
PY_LOG_COMPILER=@PYTHON@
TESTS_ENVIRONMENT=TCLLIBPATH=@prefix@/TclLibs; export TCLLIBPATH; \
	PYTHONPATH=@prefix@/pythonLibs; export PYTHONPATH;

TESTFILES=dbringbuffer.test


TESTS=dbringbuffer.test

EXTRA_DIST=ringbuffer.tcl $(TCL_PACKAGES) $(TESTFILES)
