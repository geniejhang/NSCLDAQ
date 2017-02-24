TCL_PACKAGES= LogModel.tcl LogView.tcl RingStatModel.tcl \
   RingStatController.tcl RingStatView.tcl \
   ReadoutStatModel.tcl ReadoutStatView.tcl ReadoutStatController.tcl \
   dashobjects.tcl RingStatusHighlight.tcl



TCL_APPS=LogMessageViewer.tcl ringstatviewer.tcl readoutstatviewer.tcl \
	StatusViewer.tcl Dashboard.tcl

IMAGES=folder.png program.png iconcredit.txt run.png

DOCS=logviewer.xml

tclpkgInstall:
	for f in $(TCL_PACKAGES); do $(INSTALL_SCRIPT) @srcdir@/$$f @prefix@/TclLibs/status; done
	echo pkg_mkIndex -verbose "@prefix@/TclLibs/status *.so *.tcl" |@TCLSH@

tclappInstall:
	$(mkinstalldirs) @bindir@
	for f in $(TCL_APPS); do $(INSTALL_SCRIPT) @srcdir@/$$f @bindir@/`basename $$f .tcl`; done
	for f in $(IMAGES); do $(INSTALL_DATA) @srcdir@/$$f @prefix@/TclLibs/status; done

install-exec-hook: tclpkgInstall tclappInstall

EXTRA_DIST=$(TCL_PACKAGES) $(TCL_APPS) $(DOCS) $(IMAGES)
