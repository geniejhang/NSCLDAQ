lib_LTLIBRARIES = libUDPBrokerBase.la

libUDPBrokerBase_la_SOURCES = UDPBrokerBase.cpp

libUDPBrokerBase_la_CXXFLAGS = -I@top_srcdir@/base/udp \
                -I@top_srcdir@/daq/format \
                -I@top_srcdir@/daq/IO \
                @LIBTCLPLUS_CFLAGS@

libUDPBrokerBase_la_LIBADD = @top_builddir@/base/udp/libUdp.la \
                   @top_builddir@/daq/IO/libdaqio.la \
                   @top_builddir@/daq/format/libdataformat.la \
                   @LIBEXCEPTION_LDFLAGS@

include_HEADERS = UDPBrokerBase.h datagramHeader.h pcapHeader.h


