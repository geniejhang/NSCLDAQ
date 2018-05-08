lib_LTLIBRARIES		= libCCUSBCore.la

libCCUSBCore_la_SOURCES	= Globals.cpp CAcquisitionThread.cpp 	\
				CControlQueues.cpp \
				COutputThread.cpp \
				CRunState.cpp \
				DataBuffer.cpp  \
				CSystemControl.cpp \
				CBeginRun.cpp \
				CEndRun.cpp \
				CPauseRun.cpp \
				CResumeRun.cpp \
				CInit.cpp \
				CExit.cpp  \
				TclServer.cpp \
				Server.cpp \
			serverinstance.cpp \
				CGetCommand.cpp  \
				CSetCommand.cpp  \
				CUpdateCommand.cpp  \
					tclUtil.cpp	\
					CCCUSBHighLevelController.cpp \
					CPreBeginCommand.cpp CPreEndCommand.cpp \
					CPrePauseCommand.cpp CPreResumeCommand.cpp

noinst_HEADERS		= CAcquisitionThread.h		\
				CControlQueues.h \
				COutputThread.h \
				CRunState.h \
				DataBuffer.h \
			CSystemControl.h \
				CBeginRun.h \
				CEndRun.h \
				CPauseRun.h \
				CResumeRun.h \
				CInit.h \
				CExit.h  \
				TclServer.h \
				server.h \
			serverinstance.h \
				CGetCommand.h  \
				CSetCommand.h  \
				CUpdateCommand.h  \
				tclUtil.h \
				Events.h	\
				CCCUSBHighLevelController.h  \
				CPreBeginCommand.h CPreEndCommand.h \
				CPrePauseCommand.h CPreResumeCommand.h	\
				Events.h				\
				Globals.h

libCCUSBCore_la_CPPFLAGS = \
		  @THREADCXX_FLAGS@ \
			-I@top_srcdir@/usb/common/configurableobject \
			-I@top_srcdir@/usb/ccusb/ccusb		\
			-I@top_srcdir@/usb/ccusb/daqconfig	\
			-I@top_srcdir@/usb/ccusb/ctlconfig	\
			-I@top_srcdir@/usb/common/slowcontrols	\
			-I@top_srcdir@/usb/ccusb	\
			@TCL_FLAGS@ \
			@LIBTCLPLUS_CFLAGS@		\
			-I@top_srcdir@/base/headers	\
			-I@top_srcdir@/base/dataflow	\
			-I@top_srcdir@/base/thread	\
			-I@top_srcdir@/base/os \
			-I@top_srcdir@/utilities/nscldaq-format/format	\
			-I@top_srcdir@/utilities/nscldaq-format/FormattedIO \
			-I@top_srcdir@/utilities/nscldaq-format/Buffer \
			-I@top_srcdir@/utilities/IO \
                        -I@top_srcdir@/diagnostics/status  \
                        -I@top_srcdir@/base/sfinae_tools

# add -DREPORT_ERRORS to INCLUDES below for verbose
# error reporting info.



libCCUSBCore_la_LIBADD	= \
			@top_builddir@/usb/ccusb/daqconfig/libCCUSBDaqConfig.la	\
			@top_builddir@/usb/ccusb/ctlconfig/libCCUSBCtlConfig.la	\
			@top_builddir@/usb/common/configurableobject/libConfigurableObject.la \
		 	 @top_builddir@/usb/ccusb/ccusb/libCCUSB.la		\
			@LIBTCLPLUS_LDFLAGS@					\
			@top_builddir@/base/thread/libdaqthreads.la \
			@top_builddir@/base/os/libdaqshm.la	\
      @top_builddir@/utilities/nscldaq-format/format/V12/libdataformatv12.la	\
      @top_builddir@/utilities/nscldaq-format/FormattedIO/libdaqformatio.la	\
      @top_builddir@/utilities/nscldaq-format/Buffer/libbuffer.la	\
      @top_builddir@/utilities/IO/libdaqio.la	\
			@top_builddir@/diagnostics/status/libStatusMessages.la \
      @TCL_LDFLAGS@ \
      @THREADLD_FLAGS@ \
			@ZMQ_LDFLAGS@	


