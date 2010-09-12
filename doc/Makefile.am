if MOO_DEV_MODE

src_texinfo_files =	\
	medit.texi	\
	prefs.texi	\
	prefs-file.texi	\
	user-tools.texi	\
	regex.texi	\
	license.texi	\
	script.texi	\
	mooscript.texi

medit.texi: medit.texi.in $(top_builddir)/config.status
	cd $(top_builddir) && ./config.status --file=doc/medit.texi
	mv medit.texi $(srcdir)/medit.texi

mooscript.texi: $(top_srcdir)/moo/mooscript/mooscript-classes.h $(top_srcdir)/moo/mooscript/gendoc.py
	$(PYTHON) $(top_srcdir)/moo/mooscript/gendoc.py $(top_srcdir)/moo/mooscript/mooscript-classes.h \
		> mooscript.texi.tmp && mv mooscript.texi.tmp $(srcdir)/mooscript.texi

help/index.html: $(src_texinfo_files)
	rm -f $(srcdir)/help/*.html
	$(MKDIR_P) $(srcdir)/help
	cd $(srcdir) && makeinfo --html medit.texi

help-sections.h: help/index.html $(srcdir)/gensections.py
	$(PYTHON) $(srcdir)/gensections.py $(srcdir)/help/*.html > help-sections.h.tmp && \
		mv help-sections.h.tmp $(srcdir)/help-sections.h

all-am: doc
doc: help/index.html help-sections.h medit.1

man-medit.t2t: man-medit.t2t.in $(top_builddir)/config.status
	cd $(top_builddir) && ./config.status --file=doc/man-medit.t2t
	mv man-medit.t2t $(srcdir)/man-medit.t2t

medit.1: man-medit.t2t
	txt2tags --target=man --outfile=- $(srcdir)/man-medit.t2t | grep -v "cmdline: txt2tags" \
		> medit.1.tmp && mv medit.1.tmp $(srcdir)/medit.1

endif

EXTRA_DIST = help help-sections.h medit.1

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(MOO_HELP_DIR)
	cd $(srcdir) && $(INSTALL_DATA) help/*.html $(DESTDIR)$(MOO_HELP_DIR)

uninstall-local:
	rm -f $(DESTDIR)$(MOO_HELP_DIR)/*.html

if MOO_OS_UNIX
dist_man_MANS = medit.1
endif MOO_OS_UNIX
