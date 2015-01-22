install-exec-local:
	$(mkinstalldirs) $(bindir)
	$(mkinstalldirs) $(prefix)/TclLibs/ScalerDisplay
	$(INSTALL_SCRIPT) $(TCLPACKAGES) $(prefix)/TclLibs/ScalerDisplay
	$(INSTALL_SCRIPT) ScalerDisplay ScalerDisplay.tcl $(bindir)
	$(INSTALL_SCRIPT) logchannels.tcl $(bindir)
	$(INSTALL_SCRIPT) logstripchart.tcl $(bindir)


TCLPACKAGES=scaler.tcl 
TCLAPPS=ScalerDisplay ScalerDisplay.tcl

EXTRA_DIST = ScalerDisplay.tcl scaler.tcl \
 ScalerDisplay.in scalerdisplay.1 logchannels.tcl logstripchart.tcl

man_MANS = scalerdisplay.1