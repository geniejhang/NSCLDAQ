scripts = loadshaper.tcl n568b.tcl \
	shaper.ui.tcl shaper16.ui.tcl shaperapp.tcl \
	shaperapp16.tcl

samples = goShaper goShaper16 \
	myshaper.cfg shaper.cfg


EXTRA_DIST = $(scripts) $(samples)


install-exec-local:
	$(mkinstalldirs) $(prefix)/contrib $(prefix)/contrib/N568panel
	$(mkinstalldirs) $(prefix)/contrib/N568panel/examples
	for f in $(scripts);  \
	do \
	   $(INSTALL_PROGRAM) $(srcdir)/$$f $(prefix)/contrib/N568panel; \
        done
	for f in $(samples); \
	do			\
	   $(INSTALL_PROGRAM) $(srcdir)/$$f $(prefix)/contrib/N568panel/examples ; \
	done