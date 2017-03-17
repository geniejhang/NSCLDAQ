
bin_PROGRAMS = FileAnalyzer

FileAnalyzer_SOURCES = FileAnalyzer.cpp \
                    CV11SourceCounterFilter.cpp \
                    CV11SourceCounterFilter.h \
                    FragmentIndex.cpp \
                    FragmentIndex.h \
                    CV12SourceCounterFilter.cpp \
                    CV12SourceCounterFilter.h

FileAnalyzer_CXXFLAGS = -I@top_srcdir@/utilities/filter \
                        -I@top_srcdir@/utilities/filter/core \
                        -I@top_srcdir@/utilities/IO \
                        -I@top_srcdir@/utilities/nscldaq-format/format \
                        -I@top_srcdir@/utilities/nscldaq-format/FormattedIO \
                        -I@top_srcdir@/daq/eventbuilder \
                        @LIBTCLPLUS_CFLAGS@

FileAnalyzer_LDADD = @top_builddir@/utilities/filter/core/libfilter.la \
                    @top_builddir@/utilities/filter/V11/libfilterv11.la \
                    @top_builddir@/utilities/nscldaq-format/format/V11/libdataformatv11.la \
                    @top_builddir@/utilities/filter/V12/libfilterv12.la \
                    @top_builddir@/utilities/nscldaq-format/format/V12/libdataformatv12.la \
                    @top_builddir@/utilities/nscldaq-format/FormattedIO/libdaqformatio.la \
                    @top_builddir@/utilities/IO/libdaqio.la \
                    @top_builddir@/base/dataflow/libDataFlow.la

