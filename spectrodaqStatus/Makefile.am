PKGDIR=$(prefix)/TclLibs/spdaqstat

PKGFILES=spdaqstat.tcl	\
	spdaqwidgets.tcl

install-exec-local:
	$(mkinstalldirs) $(PKGDIR)
	chmod a+rx $(PKGDIR)
	$(INSTALL_SCRIPT) $(PKGFILES) $(PKGDIR)
	echo "pkg_mkIndex $(PKGDIR) *.tcl" >makeindex.tcl
	$(TCLSH_CMD) <makeindex.tcl




EXTRA_DIST=$(PKGFILES)


