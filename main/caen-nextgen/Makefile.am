lib_LTLBRARIES=libCaenVx2750.la

include_HEADERS=CAENVX2750PhaTriger.h \
    Dig2Device.h \
    DynamicMultiTrigger.h \
    TclConifiguredReadout.h \
    VX2750EventSegment.h \
    VX2750MultiModuleEventSegment.h \
    VX2750MultiTrigger.h \
    VX2750PHAConfiguration.h \
    VX2750Pha.h \
    VX2750TclConfig.h

libCAENVx2750_la_CPPFLAGS=@BOOST_CPPFLAGS@ @BOOST_EXTRA_CPPFLAGS@ \
    @TCL_CPPFLAGS@ @CAENNG_CPPFLAGS@ -I@top_srcdir@/usb/common/configurableobject

libCAENVx2750_la_LDFLAGS=@BOOST_LDFLAGS@ @BOST_LOG_LIB@  \
    @TCL_LDFLAGS@ @CAENNG_LDFLAGS@ \
    @top_builddir@/libConfigurableObject.la


libCAENVx2750_la_SOURCES= CAENVX2750PhaTrigger.cpp \
    Deg2Device.cpp \
    DynamicMultiTrigger.cpp \
    TclConfiguredReadout.cpp \
    VX2750EventSegment.cpp \
    VX2750MultiModuleEventSegment.cpp \
    VX2750MultiTrigger.cpp \
    VX2750PHAConfiguration.cpp \
    VX2750Pha.cpp \
    VBX2750TclConfig.cpp 



noinst_HEADERS=Asserts.h


#  These are test programs but can only be run
#  If there is hardware to run them on.

#noinst_PROGRAMS=fejackettests triggertests configtests