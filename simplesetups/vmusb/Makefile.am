DOCS    = @srcdir@/vmusbsetup.xml @srcdir@/vmusbbookinfo.xml
#FIGURES = 


CHAPTER2BOOK=@top_srcdir@/docbuild/chapter2book.py

BOOK = @builddir@/vmusbsetup_autobook.pdf

install-exec-local:
	$(mkinstalldirs) @prefix@/share/html
	$(mkinstalldirs) @prefix@/share/html/figures
	$(mkinstalldirs) @prefix@/share/pdfs
	$(INSTALL_DATA) $(FIGURES) @prefix@/share/html/figures
	$(CHAPTER2BOOK) -s @srcdir@/vmusbsetup.xml -b @srcdir@/vmusbbookinfo.xml -o @builddir@
	$(INSTALL_DATA) $(BOOK) @prefix@/share/pdfs

clean-local:
	rm -f $(BOOK)

EXTRA_DIST=$(DOCS) $(BOOK)
