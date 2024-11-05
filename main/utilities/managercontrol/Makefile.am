MODULE_FILES=config.py
MODULE_DEST=@prefix@/pythonLibs/nscldaq/manager_control

PROGRAM_FILE=

install-data-local:
	$(MKDIR_P) $(MODULE_DEST)
	for f in $(MODULE_FILES); do $(INSTALL_SCRIPT) @srcdir@/$$f $(MODULE_DEST); done

# $(INSTALL_PROGRAM) @srcdir@/$(PROGRAM_FILE) @bindir@/`basename $(PROGRAMFILE) .py`
