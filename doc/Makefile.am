docbook_files =				\
	medit.docbook			\
	license.docbook			\
	prefs.docbook			\
	prefs-file.docbook		\
	user-tools.docbook

EXTRA_DIST =			\
	$(docbook_files)	\
	help-sections.h		\
	manual.xsl		\
	help.xsl		\
	help.css		\
	txt2xml

all: build-help manual.html

build-help: help/help.css

DISTCLEANFILES = LGPL.xml
# Stupid form feed characters and stupid XML and stupid xsltproc!!!
LGPL.xml: $(top_srcdir)/COPYING
	$(srcdir)/txt2xml $(top_srcdir)/COPYING > LGPL.xml.tmp && \
	mv LGPL.xml.tmp LGPL.xml

stamp-help: $(docbook_files) help.xsl
	$(MAKE) $(AM_MAKEFLAGS) LGPL.xml && \
	rm -f help/*.html && \
	$(mkdir_p) help && \
	xsltproc --xinclude --path "." --stringparam base.dir $(builddir)/help/ $(srcdir)/help.xsl $(srcdir)/medit.docbook && \
	echo stamp > stamp-help
help/help.css: stamp-help help.css
	cp $(srcdir)/help.css help/

manual.html: $(docbook_files) manual.xsl
	$(MAKE) $(AM_MAKEFLAGS) LGPL.xml && \
	xsltproc --xinclude --path "." --output manual.html $(srcdir)/manual.xsl $(srcdir)/medit.docbook

medit.lo: build-help
prefs.lo: build-help
prefs-file.lo: build-help
license.lo: build-help

#############################################################################
# Standard automake stuff
#
install: install-data
install-data: build-help manual.html
	$(mkdir_p) $(DESTDIR)$(htmldir)/help
	if test -f stamp-help; then \
	  $(INSTALL_DATA) manual.html $(DESTDIR)$(htmldir)/ && \
	  $(INSTALL_DATA) help/* $(DESTDIR)$(htmldir)/help/ ; \
	else \
	  $(INSTALL_DATA) $(srcdir)/manual.html $(DESTDIR)$(htmldir)/ && \
	  $(INSTALL_DATA) $(srcdir)/help/* $(DESTDIR)$(htmldir)/help/ ; \
	fi
uninstall:
	rm -fr $(DESTDIR)$(htmldir)/help $(htmldir)/manual.html

if MOO_BUILD_DOCS
dist-hook:
	$(MAKE) $(AM_MAKEFLAGS) all || exit 1
	if test -f stamp-help; then \
	  cp -p stamp-help manual.html $(distdir)/ && \
	  cp -pR help/ $(distdir)/ ; \
	else \
	  cp -p $(srcdir)/stamp-help $(srcdir)/manual.html $(distdir)/ && \
	  cp -pR $(srcdir)/help/ $(distdir)/ ; \
	fi
endif

clean: mostlyclean
mostlyclean:
	rm -fr help stamp-help manual.html
distclean: clean
	rm -f Makefile Makefile.in
