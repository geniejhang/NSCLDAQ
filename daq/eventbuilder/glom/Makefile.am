bin_PROGRAMS=glom

glom_SOURCES= glom.c

noinst_HEADERS = glom.h

BUILT_SOURCES = glom.c glom.h

glom.c:  glom.h

glom.h: glom.ggo
	@GENGETOPT@ --input=glom.ggo --file-name=glom