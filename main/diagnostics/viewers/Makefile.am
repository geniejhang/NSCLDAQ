TCL_PACKAGES= LogModel.tcl LogView.tcl

TCL_APPS=LogMessageViewer.tcl


DOCS=logviewer.xml

tclpkgInstall:
	for f in $(TCL_PACKAGES); do $(INSTALL_SCRIPT) @srcdir@/$$f @prefix@/TclLibs/status; done
	echo pkg_mkIndex -verbose "@prefix@/TclLibs/status *.so *.tcl" |@TCLSH@

tclappInstall:
	$(mkinstalldirs) @bindir@
	for f in $(TCL_APPS); do $(INSTALL_SCRIPT) @srcdir@/$$f @bindir@/`basename $$f .tcl`; done

install-exec-hook: tclpkgInstall tclappInstall

EXTRA_DIST=$(TCL_PACKAGES) $(TCL_APPS) $(DOCS)
