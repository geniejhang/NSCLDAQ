bin_PROGRAMS = teering

teering_SOURCES = teering.cpp @srcdir@/tcmdline.c

BUILT_SOURCES=@srcdir@/tcmdline.c

@srcdir@/tcmdline.c: @srcdir@/tcmdline.h


@srcdir@/tcmdline.h: @srcdir@/options.ggo
	$(GENGETOPT) --input=@srcdir@/options.ggo \
		--output-dir=@srcdir@ -F tcmdline

teering_CPPFLAGS = -I@top_srcdir@/daq/eventbuilder  \
	-I@top_srcdir@/base/dataflow \
	-I@top_srcdir@/base/os	\
	-I@top_srcdir@/utilities/IO	\
	-I@top_srcdir@/utilities/nscldaq-format/Buffer \
	-I@top_srcdir@/utilities/nscldaq-format/format \
	-I@top_srcdir@/utilities/nscldaq-format/FormattedIO \
	-I@top_srcdir@/utilities/nscldaq-format/utils


teering_LDADD = @top_builddir@/daq/eventbuilder/libEventBuilderClient.la \
	@top_builddir@/utilities/nscldaq-format/FormattedIO/libdaqformatio.la \
	@top_builddir@/utilities/nscldaq-format/Buffer/libbuffer.la	\
	@top_builddir@/utilities/nscldaq-format/format/V12/libdataformatv12.la \
	@top_builddir@/utilities/IO/libdaqio.la \
	@top_builddir@/base/dataflow/libRingBuffer.la     \
	@top_builddir@/base/os/libdaqshm.la		 \
	@LIBTCLPLUS_LDFLAGS@	\
	@THREADLD_FLAGS@

teering_LDFLAGS= -Wl,"-rpath=@libdir@"

EXTRA_DIST=options.ggo teering.xml
