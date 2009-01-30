#
#  This is an automake input file for building the
#  CAEN V1495 loader redistributed under license
#  from CAEN and modified to load via the SBS
#  Bit 3 interface.
#

bin_PROGRAMS=V1495Loader

noinst_HEADERS=CAENVMElib.h  CAENVMEoslib.h  CAENVMEtypes.h  console.h

V1495Loader_SOURCES=console.c    V1495Upgrade.c

INCLUDES=-DSBSVMELIB $(VMEBUILDHEADERS) -DLINUX

V1495Loader_LDFLAGS=$(VMEBUILDLIBS)  -lncurses \
			-Wl,"-rpath-link=$(libdir)"

