bin_PROGRAMS=statusinjector

statusinjector_SOURCES=statusinjector.cpp


nodist_statusinjector_SOURCES= cmdopts.h cmdopts.c

BUILT_SOURCES= cmdopts.h cmdopts.c

cmdopts.c: cmdopts.h

cmdopts.h: cmdopts.ggo
	$(GENGETOPT) --output-dir=@builddir@ --input=@srcdir@/cmdopts.ggo -F cmdopts
