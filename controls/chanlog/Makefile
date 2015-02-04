
OS=$(shell uname)
LinuxLIBID=linux-x86_64

COMPILE=g++ -I/usr/lib/epics/include  -I/usr/lib/epics/include/os/$(OS) \
	-g -c
LINK=g++ -g
LIBRARIES=-L/usr/lib/epics/lib/$($(OS)LIBID) -lca  -Wl,"-rpath=/usr/lib/epics/lib/$($(OS)LIBID)"

SOURCES=chanlog.cpp getinfo.cpp
OBJECTS=chanlog.o getinfo.o


.cpp.o:
	$(COMPILE) $<

chanlog: $(OBJECTS)
	$(LINK)   -o chanlog $(OBJECTS) $(LIBRARIES)


clean:
	rm $(OBJECTS)
	rm chanlog
