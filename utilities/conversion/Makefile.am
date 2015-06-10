lib_LTLIBRARIES = libConversion.la

SUBDIRS = NSCLDAQ10 NSCLDAQ11

libConversion_la_SOURCES = CTransformMediator.h \
                           CTransform10p0to11p0.cpp

                           #CTransform11p0to11p0.cpp

include_HEADERS = CTransformMediator.h \
                           CTransform10p0to11p0.h

#                           CTransform11p0to11p0.h

libConversion_la_CXXFLAGS = -I@srcdir@ \
                            -I@top_srcdir@/daq/format \
                            @LIBTCLPLUS_CFLAGS@

libConversion_la_LIBADD = @LIBTCLPLUS_LDFLAGS@
