lib_LTLIBRARIES = libdataformatv10.la

libdataformatv10_la_SOURCES = CRingItem.cpp \
                           CRingScalerItem.cpp \
                           CRingStateChangeItem.cpp \
                           CPhysicsEventItem.cpp \
                           CRingPhysicsEventCountItem.cpp \
                           CRingTextItem.cpp

include_HEADERS = CRingItem.h \
                  CRingScalerItem.h \
                  CRingStateChangeItem.h \
                  CPhysicsEventItem.h \
                  CRingPhysicsEventCountItem.h \
                  CRingTextItem.h \
                  DataFormatV10.h

libdataformatv10_la_CXXFLAGS = -I@srcdir@ \
                               @LIBTCLPLUS_CFLAGS@

libdataformatv10_la_LDFLAGS = @LIBTCLPLUS_LDFLAGS@

