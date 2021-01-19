bin_PROGRAMS		=	evtCreator
BUILT_SOURCES		= 	evtCreatorargs.c evtCreatorargs.h

evtCreator_SOURCES	=	evtCreator.cpp evtCreatorMain.cpp
nodist_evtCreator_SOURCES =       evtCreatorargs.c evtCreatorargs.h

noinst_HEADERS		=	evtCreatorMain.h

evtCreator_DEPENDENCIES	=	evtCreatorargs.o


evtCreator_CPPFLAGS       =	-I@top_srcdir@/base/headers		\
				@LIBTCLPLUS_CFLAGS@			\
				-I@top_srcdir@/daq/format		\
			        -I@top_srcdir@/base/dataflow	\
				-I@top_srcdir@/base/os		\
				-I@top_srcdir@/utilities/IO \
				@OPENSSL_INCLUDES@

evtCreator_LDADD		=	@top_builddir@/daq/format/libdataformat.la	\
				@top_builddir@/base/dataflow/libDataFlow.la	\
				@LIBEXCEPTION_LDFLAGS@			\
				@top_builddir@/base/os/libdaqshm.la		\
				@top_builddir@/utilities/IO/libdaqio.la \
				$(THREADLD_FLAGS) @OPENSSL_LDFLAGS@ @OPENSSL_LIBS@

evtCreator_CXXFLAGS	=	$(THREADCXX_FLAGS) $(AM_CXXFLAGS)

evtCreatorargs.c: evtCreatorargs.h
evtCreatorargs.h: evtCreatorargs.ggo
	$(GENGETOPT) <@srcdir@/evtCreatorargs.ggo --file=evtCreatorargs \
			--set-version=@VERSION@

EXTRA_DIST		=	evtCreatorargs.ggo evtCreator.xml 

clean-local:
	rm -f evtCreatorargs.h evtCreatorargs.c

#-------------------------------------------------------------
#
# Test programs.
#
noinst_PROGRAMS         =       evtCreatorTests
evtCreatorTests_SOURCES = TestRunner.cpp evtCreatorTests.cpp
evtCreatorTests_CPPFLAGS=@CPPUNIT_CFLAGS@ -I@top_srcdir@/base/headers             \
	@LIBTCLPLUS_CFLAGS@                     \
	-I@top_srcdir@/daq/format               \
	-I@top_srcdir@/base/dataflow    \
	-I@top_srcdir@/base/os          \
	-I@top_srcdir@/utilities/IO \
	@OPENSSL_INCLUDES@

evtCreatorTests_LDADD   = @CPPUNIT_LDFLAGS@ \
	@top_builddir@/daq/format/libdataformat.la      \
	@top_builddir@/base/dataflow/libDataFlow.la     \
	@LIBEXCEPTION_LDFLAGS@                  \
	@top_builddir@/base/os/libdaqshm.la             \
	@top_builddir@/utilities/IO/libdaqio.la \
	$(THREADLD_FLAGS)

TESTS=evtCreatorTests
