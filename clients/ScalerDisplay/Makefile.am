install-exec-local:
	$(mkinstalldirs) $(prefix)/Scripts
	$(mkinstalldirs) $(bindir)
	$(INSTALL_SCRIPT) *.tcl  $(prefix)/Scripts
	rm -f $(bindir)/ScalerDisplay
	$(LN_S) $(prefix)/Scripts/ScalerDisplay.tcl $(bindir)/ScalerDisplay

EXTRA_DIST = ScalerDisplay.tcl mwutil.tcl notebook.tcl scaler.tcl \
	tablelist.tcl tablelistSortByColumn.tcl tablelistWidget.tcl \
	tabnbook.tcl
