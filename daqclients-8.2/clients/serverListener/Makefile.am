bin_PROGRAMS=serverListener

serverListener_SOURCES=serverListener.cpp

INCLUDES= -I@top_srcdir@/Exception -I@top_srcdir@/Headers  -I@top_srcdir@/framework $(SERVER_DEBUG)

serverListener_LDADD= @top_srcdir@/framework/libEventFramework.la	\
	@top_srcdir@/Exception/libException.la			\
	@top_srcdir@/TCL/libtclPlus.la				\
	-lrt $(SPECTRODAQ_LDFLAGS) $(TCL_LDFLAGS) $(X11LIBS)

serverListener_LDFLAGS = -Wl,"-rpath=$(libdir)"