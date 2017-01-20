lib_LTLIBRARIES = libsqlite3pp.la

TCL_PACKAGE_FILES = where.tcl
TCL_PACKAGE_DIR   = @prefix@/TclLibs/sqlite
TCL_TESTS         = where.test


PYTHON_MODULE_FILES = where.py
PYTHON_PACKAGE_DIR  = @prefix@/pythonLibs/nscldaq/sqlite
PYTHON_TESTS        = wheretests.py

include_HEADERS=CSqlite.h CSqliteException.h CSqliteStatement.h\
	 CSqliteTransaction.h CSqliteWhere.h

libsqlite3pp_la_SOURCES=CSqlite.cpp CSqliteException.cpp CSqliteStatement.cpp \
	CSqliteTransaction.cpp CSqliteWhere.cpp


libsqlite3pp_la_CPPFLAGS = @SQLITE3_CFLAGS@
libsqlite3pp_la_LDFLAGS  = @SQLITE3_LDFLAGS@

noinst_PROGRAMS=unittests
noinst_HEADERS=Asserts.h

unittests_SOURCES = TestRunner.cpp exceptionTest.cpp connectionTest.cpp \
			statementTest.cpp transactionTest.cpp filtertests.cpp \
			savepttests.cpp \
			$(libsqlite3pp_la_SOURCES)

unittests_LDFLAGS = @SQLITE3_LDFLAGS@ @CPPUNIT_LDFLAGS@
unittests_CPPFLAGS= @SQLITE3_CFLAGS@ @CPPUNIT_CFLAGS@

tcl-install:  $(TCL_PACKAGE_FILES)
	$(mkinstalldirs) $(TCL_PACKAGE_DIR)
	for f in $(TCL_PACKAGE_FILES);  do $(INSTALL_SCRIPT) @srcdir@/$$f $(TCL_PACKAGE_DIR); done
	echo pkg_mkIndex -verbose $(TCL_PACKAGE_DIR) "*.tcl" | @TCLSH@

python-install:	$(PYTHON_PACKAGE_FILES) 
	$(mkinstalldirs) $(PYTHON_PACKAGE_DIR)
	rm -f $(PYTHON_PACKAGE_DIR)/*.pyc
	touch $(PYTHON_PACKAGE_DIR)/__init__.py
	for f in $(PYTHON_MODULE_FILES); do $(INSTALL_SCRIPT) @srcdir@/$$f $(PYTHON_PACKAGE_DIR); done

install-exec-hook: tcl-install python-install




TEST_EXTENSIONS=.py .test
PY_LOG_COMPILER=@PYTHON@
TEST_LOG_COMPILER=@TCLSH@
TESTS_ENVIRONMENT=PYTHONPATH=@prefix@/pythonLibs; export PYTHONPATH;\
	TCLLIBPATH=@prefix@/TclLibs; export TCLLIBPATH;

TESTS=@builddir@/unittests  $(TCL_TESTS) $(PYTHON_TESTS)


EXTRA_DIST = sqlite++.xml $(TCL_PACKAGE_FILES) $(TCL_TESTS) \
	$(PYTHON_MODULE_FILES) $(PYTHON_TESTS)


