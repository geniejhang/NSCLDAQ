lib_LTLIBRARIES		= libCCUSBConfig.la

libCCUSBConfig_la_SOURCES	= \
				CConfiguration.cpp	\
				CStackCommand.cpp	\
				CPH7xxCommand.cpp	\
				CAD811Command.cpp	\
				CLRS2551Command.cpp	\
				CLRS2249Command.cpp	\
				CLRS2228Command.cpp	\
				CCAEN257Command.cpp	\
				C1205Command.cpp	\
				CCCUSBCommand.cpp	\
				CPH7132Command.cpp	\
				CUserCommand.cpp	\
				CAddTclDriver.cpp 	\
				CMarkerCommand.cpp	\
				@top_srcdir@/usb/common/devices/CCamacBranchException.cpp	 \
			 	CConfigurableObject.cpp	\
				CReadoutModule.cpp	\
				CReadoutHardware.cpp	\
				CPH7xx.cpp	\
				CAD811.cpp	\
				CStack.cpp	\
				CLRS2551.cpp	\
				CLRS2249.cpp	\
				CLRS2228.cpp	\
				CCAEN257.cpp	\
				CC1205.cpp	\
				CCCUSBModule.cpp	\
				CPH7132.cpp	\
				CTclModule.cpp	\
				CLeCroy4434.cpp \
				CCamacCrate.cpp \
				CLeCroy4300B.cpp \
				CULMTrigger.cpp \
				CMarker.cpp




noinst_HEADERS		= \
				CConfiguration.h 	\
				CStackCommand.h		\
				CPH7xxCommand.h		\
				CAD811Command.h		\
				CLRS2551Command.h	\
				CLRS2249Command.h	\
				CLRS2228Command.h	\
				CCAEN257Command.h	\
				C1205Command.h		\
				CCCUSBCommand.h		\
				CPH7132Command.h	\
				CUserCommand.h		\
				CAddTclDriver.h		\
				CMarkerCommand.h
				CConfigurableObject.h	\
				CReadoutModule.h 	\
				CReadoutHardware.h 	\
				CStack.h		\
				CAD811.h		\
				CPH7xx.h		\
				CLRS2551.h		\
				CLRS2249.h		\
				CLRS2228.h		\
				CCAEN257.h		\
				CC1205.h		\
				CCCUSBModule.h		\
				CPH7132.h		\
				CTclModule.h		\
				CMarker.h

#				CScalerCommand.h




libCCUSBConfig_la_CPPFLAGS		= \
				-I@top_srcdir@/usb/ccusb		\
				-I@top_srcdir@/usb/ccusb/ccusb		\
				-I@top_srcdir@/usb/common/devices		\
				-I@top_srcdir@/base/headers		\
				@LIBTCLPLUS_CFLAGS@		\
				@TCL_FLAGS@ \
				@THREADCXX_FLAGS@

libCCUSBConfig_la_LIBADD	=	 \
				@top_builddir@/usb/ccusb/ccusb/libCCUSB.la \
				@LIBTCLPLUS_LDFLAGS@ \
				@TCL_LDFLAGS@ \
				@THREADLD_FLAGS@ 


install-data-local:
	$(mkinstalldirs) @prefix@/ccusbdriver
	$(mkinstalldirs) @prefix@/ccusbdriver/includes
	$(INSTALL_DATA)  @srcdir@/*.h @prefix@/ccusbdriver/includes


EXTRA_DIST = CCamacCompat.hpp
