#-------------------- Filter Kit
EXTRA_DIST = SkeletonMain.cpp CTemplateFilter.h \
            CTemplateFilter.cpp \
             Makefile-template.in FilterIncludes.in



noinst_PROGRAMS = testapp

testapp_SOURCES = TestApp.cpp
testapp_CPPFLAGS= -I@top_srcdir@/utilities \
                    -I@top_srcdir@/utilities/filter \
                    -I@top_srcdir@/utilities/filter/core \
                    -I@top_srcdir@/utilities/IO \
                    -I@top_srcdir@/utilities/nscldaq-format/format \
                    -I@top_srcdir@/utilities/nscldaq-format/FormattedIO \
                    -I@top_srcdir@/utilities/nscldaq-format/Buffer \
                    -I@top_srcdir@/base/uri		\
                    -I@top_srcdir@/base/dataflow \
                    -I@top_srcdir@/base/headers		\
                    @LIBTCLPLUS_CFLAGS@

testapp_LDADD = @top_builddir@/utilities/filter/core/libfilter.la	\
                        @top_builddir@/utilities/filter/V11/libfilterv11.la	\
                        @top_builddir@/utilities/filter/V12/libfilterv12.la	\
                        @top_builddir@/utilities/IO/libdaqio.la \
                        @top_builddir@/base/dataflow/libDataFlow.la 	\
                        @top_builddir@/base/uri/liburl.la 	\
                        @top_builddir@/utilities/nscldaq-format/format/V10/libdataformatv10.la \
                        @top_builddir@/utilities/nscldaq-format/format/V11/libdataformatv11.la \
                        @top_builddir@/utilities/nscldaq-format/format/V12/libdataformatv12.la \
                        @top_builddir@/utilities/nscldaq-format/FormattedIO/libdaqformatio.la \
                        @top_builddir@/utilities/nscldaq-format/Buffer/libbuffer.la \
                        @LIBEXCEPTION_LDFLAGS@
testapp_LDFLAGS	= -Wl,"-rpath-link=$(libdir)"


install-data-local:
	$(mkinstalldirs) @prefix@/filterkit
	$(INSTALL_DATA) FilterIncludes \
		@srcdir@/CTemplateFilter.cpp \
		@srcdir@/CTemplateFilter.h \
		@srcdir@/SkeletonMain.cpp @prefix@/filterkit
	$(INSTALL_DATA) Makefile-template @prefix@/filterkit/Makefile

