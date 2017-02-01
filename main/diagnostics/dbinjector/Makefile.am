bin_PROGRAMS=statusinjector

statusinjector_SOURCES=statusinjector.cpp CParameters.cpp CParameters.h
statusinjector_CPPFLAGS=-I@top_srcdir@/base/sqlite++ -I@top_srcdir@/diagnostics/status
statusinjector_LDFLAGS=@top_builddir@/diagnostics/status/libStatusMessages.la \
	@top_builddir@/base/sqlite++/libsqlite3pp.la


nodist_statusinjector_SOURCES= cmdopts.h cmdopts.c

BUILT_SOURCES= cmdopts.h cmdopts.c

cmdopts.c: cmdopts.h

cmdopts.h: cmdopts.ggo
	$(GENGETOPT) --output-dir=@builddir@ --input=@srcdir@/cmdopts.ggo -F cmdopts
