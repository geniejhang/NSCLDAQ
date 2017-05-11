lib_LTLIBRARIES=libvardbRingBuffer.la libtclvardbringbuffer.la libPyVardbRingBuffer.la

COMMONLIBS = @top_builddir@/utilities/varmanager/server/libvarmgr.la \
		@top_builddir@/utilities/varmanager/libvardb.la         \
		@top_builddir@/base/os/libdaqshm.la			\
		@top_builddir@/base/sqlite++/libsqlite3pp.la @SQLITE3_LDFLAGS@ \
                @ZMQ_LDFLAGS@
COMMONLDFLAGS = -Wl,"-rpath=@libdir@"
COMMONCPPFLAGS= -I@top_srcdir@/utilities/varmanager/server 	\
	-I@top_srcdir@/utilities/varmanager			\
	-I@top_srcdir@/base/sqlite++

#  C++ API:

libvardbRingBuffer_la_SOURCES = CVardbRingBuffer.cpp 
libvardbRingBuffer_la_CPPFLAGS=$(COMMONCPPFLAGS)
libvardbRingBuffer_la_LIBADD=$(COMMONLIBS)
libvardbRingBuffer_la_LDFLAGS=$(COMMONLDFLAGS)

include_HEADERS=CVardbRingBuffer.h

# TCL API:

libtclvardbringbuffer_la_SOURCES = TCLVardbRingBufferPackage.cpp \
	CTCLVardbRingBuffer.cpp CTCLVardbRingBuffer.h	\
	CTCLVardbRingBufferInstance.cpp CTCLVardbRingBufferInstance.h

libtclvardbringbuffer_la_CPPFLAGS = $(COMMONCPPFLAGS) @LIBTCLPLUS_CFLAGS@ @TCL_CPPFLAGS@
libtclvardbringbuffer_la_LIBADD = $(COMMONLIBS) @builddir@/libvardbRingBuffer.la \
	@LIBTCLPLUS_LDFLAGS@  @TCL_LDFLAGS@
libtclvardbringbuffer_la_LDFLAGS = $(COMMONLDFLAGS)

# Python API:

libPyVardbRingBuffer_la_SOURCES=PyVardbRingbuffer.cpp
libPyVardbRingBuffer_la_CPPFLAGS=$(COMMONCPPFLAGS) @PYTHON_CPPFLAGS@
libPyVardbRingBuffer_la_LIBADD=$(COMMONLIBS) @builddir@/libvardbRingBuffer.la @PYTHON_LDFLAGS@
libPyVardbRingBuffer_la_LDFLAGS=$(COMMONLDFLAGS)


# Installation automake doesn't know how to do:

install-exec-hook:
	$(mkinstalldirs) @prefix@/TclLibs/vardbringbuffer
	rm -f @prefix@/TclLibs/vardbringbuffer/libtclvardbringbuffer.so
	ln -s @libdir@/libtclvardbringbuffer.so \
		@prefix@/TclLibs/vardbringbuffer/libtclvardbringbuffer.so
	echo "pkg_mkIndex -verbose @prefix@/TclLibs/vardbringbuffer *.so" | @TCLSH@
	$(mkinstalldirs) @prefix@/pythonLibs/nscldaq/vardb
	rm -f @prefix@/pythonLibs/nscldaq/vardb/VardbRingbuffer.so
	ln -s @libdir@/libPyVardbRingBuffer.so @prefix@/pythonLibs/nscldaq/vardb/VardbRingbuffer.so

# C++ API tests:

noinst_PROGRAMS=unittests

unittests_SOURCES = TestRunner.cpp Asserts.h apitests.cpp
unittests_CPPFLAGS=$(COMMONCPPFLAGS) @CPPUNIT_CFLAGS@
unittests_LDADD= @builddir@/libvardbRingBuffer.la $(COMMONLIBS) @CPPUNIT_LDFLAGS@
unittests_LDFLAGS= $(COMMONLDFLAGS)

# Testing setup for TCL and Python tests:

TEST_EXTENSIONS=.test .py
TEST_LOG_COMPILER=@TCLSH@
PY_LOG_COMPILER=@PYTHON@
TESTS_ENVIRONMENT=TCLLIBPATH=@prefix@/TclLibs; export TCLLIBPATH; \
	PYTHONPATH=@prefix@/pythonLibs; export PYTHONPATH;

TESTFILES=tclapitest.test pyapitests.py

TESTS=@builddir@/unittests $(TESTFILES)

# Documentation

DOCFILES=man3.xml man5.xml

# Extra files in the distro tarball


EXTRA_DIST=$(DOCFILES)  $(TESTFILES)
