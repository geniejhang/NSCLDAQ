bin_PROGRAMS= frag2ring

frag2ring_SOURCES=frag2ring.cpp CFragReader.cpp CFragWriter.cpp \
	CFragReader.h CFragWriter.h

frag2ring_CPPFLAGS=$(COMPILATION_FLAGS) \
                    -I@top_srcdir@/utilities/nscldaq-format/Buffer \
		-I@top_srcdir@/daq/eventbuilder			\
		-I@top_srcdir@/base/os

frag2ring_LDADD = @top_builddir@/utilities/nscldaq-format/Buffer/libbuffer.la \
                    @top_builddir@/base/os/libdaqshm.la $(THREADLD_FLAGS)
