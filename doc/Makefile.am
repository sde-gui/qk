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

# man-medit.t2t: man-medit.t2t.in $(top_builddir)/config.status
# 	cd $(top_builddir) && ./config.status --file=doc/man-medit.t2t
# 	mv man-medit.t2t $(srcdir)/man-medit.t2t

# medit.1: man-medit.t2t
# 	cd $(srcdir) && $(PYTHON) txt2tags/txt2tags.py --target=man --outfile=medit.1.tmp man-medit.t2t && \
# 		mv medit.1.tmp medit.1

medit.texi: medit.texi.in $(top_builddir)/config.status
	cd $(top_builddir) && ./config.status --file=doc/medit.texi
	mv medit.texi $(srcdir)/medit.texi

mooscript.texi: $(top_srcdir)/moo/mooscript/mooscript-classes.h $(top_srcdir)/moo/mooscript/gendoc.py
	$(PYTHON) $(top_srcdir)/moo/mooscript/gendoc.py $(top_srcdir)/moo/mooscript/mooscript-classes.h \
		> mooscript.texi.tmp && mv mooscript.texi.tmp $(srcdir)/mooscript.texi

# help.html: $(MOO_HELP_T2T_FILES)
# 	cd $(srcdir) && $(PYTHON) txt2tags/txt2tags.py --outfile=help.html.tmp medit.t2t && \
# 		mv help.html.tmp help.html

help/index.html: $(src_texinfo_files)
	rm -f $(srcdir)/help/*.html
	$(MKDIR_P) $(srcdir)/help
	cd $(srcdir) && makeinfo --html medit.texi

help-sections.h: help/index.html $(srcdir)/gensections.py
	$(PYTHON) $(srcdir)/gensections.py $(srcdir)/help/*.html > help-sections.h.tmp && \
		mv help-sections.h.tmp $(srcdir)/help-sections.h

# help-sections.h: help.html genhelpsections.py
# 	cd $(srcdir) && $(PYTHON) genhelpsections.py help.html > help-sections.h.tmp && mv help-sections.h.tmp help-sections.h
# 	rm -f $(srcdir)/help-sections.h.tmp

all-am: doc
# doc: medit.1 help/index.html help-sections.h
doc: help/index.html help-sections.h

endif

# EXTRA_DIST = medit.1 help.html help-sections.h help
EXTRA_DIST = help help-sections.h

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(MOO_HELP_DIR)
	cd $(srcdir) && $(INSTALL_DATA) help/*.html $(DESTDIR)$(MOO_HELP_DIR)

uninstall-local:
	rm -f $(DESTDIR)$(MOO_HELP_DIR)/*.html

# if MOO_OS_UNIX
# dist_man_MANS = medit.1
# endif MOO_OS_UNIX
