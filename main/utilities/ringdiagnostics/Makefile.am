install-exec-local:
	$(mkinstalldirs) @bindir@
	$(INSTALL_PROGRAM) @srcdir@/pidtocommand.py @bindir@/pidtocommand
	$(INSTALL_SCRIPT) @srcdir@/*.py @prefix@/pythonLibs/nscldaq

EXTRA_DIST=*.py
