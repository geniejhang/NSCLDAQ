lib_LTLIBRARIES		= libVMUSBDaqConfig.la
libVMUSBDaqConfig_la_SOURCES	= 	CADCCommand.cpp		\
				CConfiguration.cpp	\
				CScalerCommand.cpp	\
				CCAENChainCommand.cpp	\
				CStackCommand.cpp	\
				CCAENV830Command.cpp	\
				CMarkerCommand.cpp	\
				CSIS3804Command.cpp	\
				CNADC2530Command.cpp	\
				CMADCCommand.cpp	\
				CMADCScalerCommand.cpp	\
				CV1x90Command.cpp	\
				CV977Command.cpp	\
				CMASECommand.cpp	\
				CMADCChainCommand.cpp   \
				CHINPCommand.cpp \
				CPSDCommand.cpp \
				CV1729Command.cpp	\
				CV1495scCommand.cpp	\
				CUserCommand.cpp	\
				CAddTclDriver.cpp	\
				CHiRACommand.cpp	\
				CVMUSBCommand.cpp	\
  			CMTDCCommand.cpp \
	      CReadoutModule.cpp	\
				C3820.cpp		\
				C785.cpp		\
				CCAENChain.cpp		\
				CStack.cpp		\
				C830.cpp		\
				CMarker.cpp		\
				C3804.cpp		\
				CNADC2530.cpp		\
				CMesytecBase.cpp	\
				CMADC32.cpp		\
				CMTDC32.cpp		\
				CMADCScaler.cpp		\
				CV1x90.cpp		\
				CV977.cpp		\
				CXLM.cpp		\
				CMASE.cpp		\
				CMADCChain.cpp	\
				CHINP.cpp \
				CPSD.cpp	\
				CV1729.cpp	\
				CV1495sc.cpp	\
				CTclModule.cpp \
				CHiRA.cpp	\
				CVMUSBControl.cpp \
				CDelay.cpp \
				CCBD8210CamacBranchDriver.cpp \
				@top_srcdir@/usb/common/devices/CCrateController.cpp \
				CCBD8210CrateController.cpp \
				CCBD8210ReadoutList.cpp \
				CCBDCamacBranch.cpp \
				@top_srcdir@/usb/common/devices/CBCNAF.cpp \
				@top_srcdir@/usb/common/devices/CCNAF.cpp \
				@top_srcdir@/usb/common/devices/CNAF.cpp \
				CULMTrigger.cpp \
				CLeCroy4300B.cpp \
				CLeCroy4434.cpp \
				CXLMFERA.cpp \
				CXLMTimestamp.cpp \
				CLeCroy2551.cpp \
				@top_srcdir@/usb/common/devices/CCamacBranchException.cpp \
				CMQDC32StackBuilder.cpp \
				CMQDC32RdoHdwr.cpp



noinst_HEADERS		=	CADCCommand.h  		\
				CConfiguration.h 	\
				CScalerCommand.h	\
				CCAENChainCommand.h	\
				CStackCommand.h		\
				CCAENV830Command.h	\
				CMarkerCommand.h	\
				CSIS3804Command.h	\
				CNADC2530Command.h	\
				CMADCCommand.h		\
				CMADCScalerCommand.h	\
				CV1x90Command.h		\
				CV977Command.h		\
				CMASECommand.h		\
				CMADCChainCommand.h	\
				CHINPCommand.h \
				CPSDCommand.h		\
				CV1729Command.h		\
				CV1495scCommand.h	\
				CUserCommand.h		\
				CAddTclDriver.h		\
				CHiRACommand.h		\
				CVMUSBCommand.h		\
				CMTDCCommand.h \
				CReadoutModule.h	\
				CReadoutHardware.h	\
				C3820.h			\
				C785.h			\
				CCAENChain.h		\
				CStack.h		\
				C830.h			\
				CMarker.h		\
				C3804.h			\
				CNADC2530.h		\
				CMesytecBase.h		\
				CMADC32.h		\
				CMTDC32.h		\
				CMADCScaler.h		\
				CV1x90.h		\
				CV977.h			\
				CXLM.h			\
				CMASE.h			\
				CMADCChain.h		\
				MADC32Registers.h	\
				CHINP.h \
				CPSD.h			\
				CV1729.h		\
				CV1495sc.h		\
				CCAENV1x90Opcodes.h	\
				CCAENV1x90Registers.h	\
				CTclModule.h		\
				CHiRA.h			\
				CVMUSBControl.h \
				CDelay.h \
				CCBD8210CamacBranchDriver.h \
				@top_srcdir@/usb/common/devices/CCrateController.h \
				CCBD8210CrateController.h \
				CCBD8210ReadoutList.h \
				CCBDCamacBranch.h \
				@top_srcdir@/usb/common/devices/CBCNAF.h \
				@top_srcdir@/usb/common/devices/CCNAF.h \
				@top_srcdir@/usb/common/devices/CNAF.h \
				CXLMFERA.h \
				CXLMTimestamp.h \
				@top_srcdir@/usb/common/devices/CCamacBranchException.h  \
				CMQDC32StackBuilder.h \
				CMQDC32RdoHdwr.h \
				MQDC32Registers.h


libVMUSBDaqConfig_la_CXXFLAGS=@THREADCXX_FLAGS@

libVMUSBDaqConfig_la_CPPFLAGS = \
				-I@top_srcdir@/usb/common/configurableobject \
				-I@top_srcdir@/usb/vmusb		\
				-I@top_srcdir@/usb/vmusb/vmusb		\
				-I@top_srcdir@/usb/common/devices		\
				@LIBTCLPLUS_CFLAGS@		\
				-I@top_srcdir@/base/headers		\
				-I@top_srcdir@/base/os	\
				$(TCL_FLAGS) 


libVMUSBDaqConfig_la_LIBADD	= \
				@top_builddir@/usb/vmusb/vmusb/libVMUSB.la \
				@top_builddir@/usb/common/configurableobject/libConfigurableObject.la \
				@LIBTCLPLUS_LDFLAGS@	\
				$(TCL_LDFLAGS) @THREADLD_FLAGS@

install-data-local:
	$(mkinstalldirs) @prefix@/vmusbdriver
	$(mkinstalldirs) @prefix@/vmusbdriver/includes
	$(INSTALL_DATA)  @srcdir@/*.h @prefix@/vmusbdriver/includes

EXTRA_DIST = CCamacCompat.hpp
