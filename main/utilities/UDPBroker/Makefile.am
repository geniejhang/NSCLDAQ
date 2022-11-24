bin_PROGRAMS=UDPBroker

UDPBroker_CXXFLAGS=-I@top_srcdir@/base/udp \
									 -I@top_srcdir@/daq/format \
									 -I@top_srcdir@/daq/IO \
									@LIBTCLPLUS_CFLAGS@

UDPBroker_LDFLAGS=@top_builddir@/base/udp/libUdp.la \
									@top_builddir@/daq/IO/libdaqio.la \
									@top_builddir@/daq/format/libdataformat.la \
	                @LIBEXCEPTION_LDFLAGS@ -Wl,"-rpath=@libdir@"

UDPBroker_SOURCES=UDPBroker.cpp udpbrokeropts.c 
include_HEADERS=datagramHeader.h pcapHeader.h udpbrokeropts.h

BUILT_SOURCES=udpbrokeropts.c 
EXTRA_DIST=udpbrokeropts.ggo

udpbrokeropts.c: udpbrokeropts.h

udpbrokeropts.h: udpbrokeropts.ggo
	@GENGETOPT@ -i @srcdir@/udpbrokeropts.ggo -F udpbrokeropts -u
