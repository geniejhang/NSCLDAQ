lib_LTLIBRARIES = libsqlite3pp.la

TCL_PACKAGE_FILES = where.tcl
TCL_PACKAGE_DIR   = @prefix@/TclLibs/sqlite

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
			$(libsqlite3pp_la_SOURCES)

unittests_LDFLAGS = @SQLITE3_LDFLAGS@ @CPPUNIT_LDFLAGS@
unittests_CPPFLAGS= @SQLITE3_CFLAGS@ @CPPUNIT_CFLAGS@

install-exec-hook: $(TCL_PACKAGE_FILES)
	$(mkinstalldirs) $(TCL_PACKAGE_DIR)
	for f in $(TCL_PACKAGE_FILES);  do $(INSTALL_SCRIPT) @srcdir@/$$f $(TCL_PACKAGE_DIR); done
	echo pkg_mkIndex -verbose $(TCL_PACKAGE_DIR) "*.tcl" | @TCLSH@


TESTS=@builddir@/unittests


EXTRA_DIST = sqlite++.xml
