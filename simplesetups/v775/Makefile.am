DOCS    = @srcdir@/v775setup.xml @srcdir@/v775book.xml
FIGURES = @srcdir@/v775ediagram.jpeg @srcdir@/v775ediagram.eps
FIGSRC  = @srcdir@/v775ediagram.dia

EXTRA_DIST=$(DOCS)

install-exec-local: v775book.pdf
	$(mkinstalldirs) @prefix/share/html
	$(mkinstalldirs) @prefix/share/pdfs
	$(INSTALL_DATA) $(FIGURES) @prefix@/share/html
	$(INSTALL_DATA) v775book.pdf @prefix@/share/pdfs

v775book.pdf: @srcdir@/v775book.xml @srcdir@/v775setup.xml
	docbook2pdf @srcdir@/v775book.xml

clean-local:
	rm -f v775book.pdf v775setup.pdf
