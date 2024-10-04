install-exec-local:
	$(mkinstalldirs) @bindir@
	$(INSTALL_PROGRAM) @srcdir@/pidtocommand.py @bindir@/pidtocommand
	rm -rf @prefix@/pythonLibs/nscldaq/__pycache__
	$(INSTALL_SCRIPT) @srcdir@/*.py @prefix@/pythonLibs/nscldaq
	$(INSTALL_PROGRAM) @srcdir@/ringdiagnostics.py @bindir@/ringdiagnostics

EXTRA_DIST=*.py
