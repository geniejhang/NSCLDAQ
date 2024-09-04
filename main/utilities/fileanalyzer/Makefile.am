
bin_PROGRAMS = FileAnalyzer

FileAnalyzer_SOURCES = FileAnalyzer.cpp CSourceCounterFilter.cpp CSourceCounterFilter.h 
FileAnalyzer_CXXFLAGS = -I@top_srcdir@/utilities/filter \
			-I@top_srcdir@/daq/format \
			-I@top_srcdir@/daq/eventbuilder \
			@LIBTCLPLUS_CFLAGS@ @UFMT_CPPFLAGS@

FileAnalyzer_LDADD = @top_builddir@/utilities/filter/libfilter.la \
			 @top_builddir@/daq/format/libdataformat.la \
			 @UFMT_LDFLAGS@ \
			 @top_builddir@/base/dataflow/libDataFlow.la

