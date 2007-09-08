EXTRA_DIST =		\
	manual.docbook	\
	manual.xsl	\
	help.xsl	\
	help.css

all: build-help manual.html

build-help: help/help.css

stamp-help: manual.docbook help.xsl
	rm -f help/*.html && \
	$(mkdir_p) help && \
	xsltproc --stringparam base.dir $(builddir)/help/ $(srcdir)/help.xsl $(srcdir)/manual.docbook && \
	echo stamp > stamp-help
help/help.css: stamp-help help.css
	cp $(srcdir)/help.css help/

manual.html: manual.docbook manual.xsl
	xsltproc --output manual.html $(srcdir)/manual.xsl $(srcdir)/manual.docbook

#############################################################################
# Standard automake stuff
#
install: install-data
install-data: build-help manual.html
	$(mkdir_p) $(DESTDIR)$(htmldir)/help
	$(INSTALL_DATA) manual.html $(DESTDIR)$(htmldir)/
	$(INSTALL_DATA) help/* $(DESTDIR)$(htmldir)/help/
uninstall:
	rm -fr $(DESTDIR)$(htmldir)/help $(htmldir)/manual.html

clean: mostlyclean
mostlyclean:
	rm -fr help stamp-help manual.html
distclean: clean
	rm -f Makefile Makefile.in

# all: chunks single
#
# clean:
# 	rm -fr manual/ *.html stamp-chunks

# single: manual.html
# manual.html: manual.docbook manual.xsl
# 	xsltproc --output manual.html manual.xsl manual.docbook
