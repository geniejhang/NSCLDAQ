install-exec-local:
	$(mkinstalldirs) $(prefix)/contrib
	$(mkinstalldirs) $(prefix)/contrib/N568Panel
	$(INSTALL_PROGRAM) $(srcdir)/*.tcl      $(prefix)/contrib/N568Panel
	$(INSTALL_PROGRAM) $(srcdir)/goShaper $(srcdir)/goShaper16 \
						$(prefix)/contrib/N568Panel
	$(INSTALL_DATA)   $(srcdir)/*.cfg       $(prefix)/contrib/N568Panel

EXTRA_DIST = loadshaper.tcl n568b.tcl shaper.ui.tcl \
	shaperapp.tcl shaperapp16.tcl shaper16.ui.tcl myshaper.cfg shaper.cfg \
	goShaper goShaper16