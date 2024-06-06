lib_LTLIBRARIES = libDDASException.la

libDDASException_la_SOURCES = CXIAException.cpp
include_HEADERS = CXIAException.h
libDDASException_la_CPPFLAGS = @LIBTCLPLUS_CFLAGS@ \
	@PIXIE_CPPFLAGS@ \
	-I@top_builddir@/ddas
libDDASException_la_LIBADD = @LIBEXCEPTION_LDFLAGS@ \
	@PIXIE_LDFLAGS@ \
	@PLX_LDFLAGS@

noinst_PROGRAMS = unittests
unittests_SOURCES = TestRunner.cpp Asserts.h ExceptionTests.cpp
unittests_CPPFLAGS = @CPPUNIT_CFLAGS@ @LIBTCLPLUS_CFLAGS@ @PIXIE_CPPFLAGS@ -I@top_builddir@/ddas
unittests_LDADD = @CPPUNIT_LIBS@ @LIBEXCEPTION_LDFLAGS@ @PIXIE_LDFLAGS@ @PLX_LDFLAGS@ @builddir@/libDDASException.la

TESTS = unittests
