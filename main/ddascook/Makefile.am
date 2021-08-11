install-exec-local:
	$(mkinstalldirs) @prefix@/share/recipes_ddas
	@DOCBOOK2HTML@ -o @builddir@/html @srcdir@/ddascook.xml
	(cd @srcdir@; tar czf - analyzer) \
		| (cd @prefix@/share/recipes; tar xzf -)
	(cd @builddir@; tar czf - html) | (cd @prefix@/share/recipes_ddas; tar xzf -)

EXTRA_DIST=ddascook.xml analyzer
