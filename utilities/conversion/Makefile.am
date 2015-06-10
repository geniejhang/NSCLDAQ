lib_LTLIBRARIES = libConversion.la

libConversion_la_SOURCES = CTransformMediator.h \
                           CTransform11p0to11p0.cpp \
                           CTransform10p0to11p0.cpp

include_HEADERS = CTransformMediator.h \
                           CTransform11p0to11p0.h \
                           CTransform10p0to11p0.h

libConversion_la_CXXFLAGS = -I@srcdir@ \
                            -I@top_srcdir@/utilities/filter \
                            -I@top_srcdir@/daq/format \
                            @LIBTCLPLUS_CFLAGS@

libConversion_la_LIBADD = @top_builddir@/utilities/libfilter.la \
                          @top_builddir@/daq/format/libdataformat.la \
                          @LIBTCLPLUS_LDFLAGS@
