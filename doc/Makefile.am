docbook_files =				\
	medit.docbook			\
	license.docbook			\
	editor-editing.docbook		\
	editor-open-save.docbook	\
	editor-plugins.docbook		\
	editor-prefs.docbook		\
	editor-tools.docbook

EXTRA_DIST =			\
	$(docbook_files)	\
	manual.xsl		\
	help.xsl		\
	help.css

all: build-help manual.html

build-help: help/help.css

DISTCLEANFILES = LGPL.xml
# Stupid form feed characters and stupid XML and stupid xsltproc!!!
LGPL.xml: $(top_srcdir)/COPYING
	$(srcdir)/txt2xml $(top_srcdir)/COPYING > LGPL.xml.tmp && \
	mv LGPL.xml.tmp LGPL.xml

stamp-help: $(docbook_files) help.xsl LGPL.xml
	rm -f help/*.html && \
	$(mkdir_p) help && \
	xsltproc --xinclude --path "." --stringparam base.dir $(builddir)/help/ $(srcdir)/help.xsl $(srcdir)/medit.docbook && \
	echo stamp > stamp-help
help/help.css: stamp-help help.css
	cp $(srcdir)/help.css help/

manual.html: $(docbook_files) manual.xsl LGPL.xml
	xsltproc --xinclude --path "." --output manual.html $(srcdir)/manual.xsl $(srcdir)/medit.docbook

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
