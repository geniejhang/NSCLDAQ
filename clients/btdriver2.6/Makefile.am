SUBDIRS=lib include
all-local:
	./DriverBuild.sh  $(KERNEL_SRC)

install-data-local:
	$(INSTALL_DATA)   dd/btp.ko /lib/modules
	$(INSTALL_SCRIPT) sys/mkbtp  /lib/modules
	$(INSTALL_DATA)  dd/*.h     $(prefix)/include


EXTRA_DIST=DriverBuild.sh
