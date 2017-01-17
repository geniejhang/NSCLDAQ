bin_PROGRAMS = teering

<<<<<<< HEAD:main/daq/evbtools/teering/Makefile.am
teering_SOURCES = teering.cpp @srcdir@/tcmdline.c # @top_srcdir@/daq/eventbuilder/fragment.c
=======
teering_SOURCES = teering.cpp @srcdir@/tcmdline.c 
>>>>>>> master:main/daq/evbtools/teering/Makefile.am

BUILT_SOURCES=@srcdir@/tcmdline.c

@srcdir@/tcmdline.c: @srcdir@/tcmdline.h


@srcdir@/tcmdline.h: @srcdir@/options.ggo
	$(GENGETOPT) --input=@srcdir@/options.ggo \
		--output-dir=@srcdir@ -F tcmdline

teering_CPPFLAGS = -I@top_srcdir@/daq/eventbuilder  \
	-I@top_srcdir@/daq/format \
	-I@top_srcdir@/base/dataflow \
	-I@top_srcdir@/base/os


teering_LDADD = @top_builddir@/daq/eventbuilder/libEventBuilderClient.la \
	@top_builddir@/daq/format/libdataformat.la \
	@top_builddir@/base/dataflow/libRingBuffer.la     \
	@top_builddir@/base/dataflow/libDataFlow.la	\
	@top_builddir@/base/os/libdaqshm.la		 \
	@LIBTCLPLUS_LDFLAGS@	\
	@THREADLD_FLAGS@

teering_LDFLAGS= -Wl,"-rpath=@libdir@"

EXTRA_DIST=options.ggo teering.xml
