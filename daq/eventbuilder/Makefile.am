# bin_PROGRAMS=eventorderer

lib_LTLIBRARIES=libEventBuilder.la

#eventorderer_LDFLAGS= $(TCL_LDFLAGS) \
#		-Wl,"-rpath=@libdir@" \
#		@THREAD_LDFLAGS@


libEventBuilder_la_SOURCES=CEventOrderClient.cpp

include_HEADERS = CEventOrderClient.h

INCLUDES = -I@top_srcdir@/base/tcpip \
	-I@top_srcdir@/base/exception  \
	-I@top_srcdir@/base/headers	\
	@THREADCXX_FLAGS@

EXTRA_DIST=protocol.xml api.xml

libEventBuilder_la_LIBADD = @top_builddir@/base/tcpip/libTcp.la \
	@top_builddir@/base/exception/libException.la