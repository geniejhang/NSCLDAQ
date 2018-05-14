bin_PROGRAMS=unglom timecheck reglom


common_cxxflags=-I@top_srcdir@/daq/eventbuilder  \
	-I@top_srcdir@/utilities/IO \
	-I@top_srcdir@/utilities/nscldaq-format/format	\
	-I@top_srcdir@/utilities/nscldaq-format/utils \
	-I@top_srcdir@/utilities/nscldaq-format/Buffer \
	-I@top_srcdir@/base/os			\
	-I@top_srcdir@/utilities/nscldaq-format/FormattedIO @LIBTCLPLUS_CFLAGS@

common_ldflags=@top_builddir@/daq/eventbuilder/libFragmentIndex.la \
	@top_builddir@/utilities/nscldaq-format/FormattedIO/libdaqformatio.la \
	@top_builddir@/utilities/nscldaq-format/format/V12/libdataformatv12.la \
	@top_builddir@/utilities/IO/libdaqio.la \
	@top_builddir@/base/dataflow/libDataFlow.la \
	@top_builddir@/base/os/libdaqshm.la \
	@top_builddir@/base/uri/liburl.la @LIBEXCEPTION_LDFLAGS@ \
	-Wl,"-rpath=@libdir@"

unglom_SOURCES=Unglom.cpp CUnglom.cpp CUnglom.h
unglom_CXXFLAGS=$(common_cxxflags)
unglom_LDFLAGS=$(common_ldflags)


timecheck_SOURCES=Timecheck.cpp CTimeChecker.cpp CTimeChecker.h
timecheck_CXXFLAGS=$(common_cxxflags)
timecheck_LDFLAGS=$(common_ldflags)


reglom_SOURCES=reglomMain.cpp CMerge.cpp CMerge.h reglomopts.c reglomopts.h
reglom_CXXFLAGS=$(common_cxxflags) -I@top_srcdir@/base/uri
reglom_LDFLAGS=$(common_ldflags)


BUILT_SOURCES=reglomopts.c 
EXTRA_DIST=reglomopts.ggo offlinereglom.tcl reglom_ref.xml


reglomopts.c: reglomopts.h

reglomopts.h: reglomopts.ggo
	@GENGETOPT@ -i reglomopts.ggo -F reglomopts -u

install-exec-local:
	$(INSTALL_PROGRAM) @srcdir@/offlinereglom.tcl @bindir@/offlinereglom
