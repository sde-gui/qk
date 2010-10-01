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
	$(AM_V_GEN)cd $(top_builddir) && ./config.status --silent --file=doc/medit.texi
	$(AM_V_at)mv medit.texi $(srcdir)/medit.texi

mooscript.texi: $(top_srcdir)/moo/mooscript/mooscript-classes.h $(top_srcdir)/moo/mooscript/gendoc.py
	$(AM_V_GEN)$(PYTHON) $(top_srcdir)/moo/mooscript/gendoc.py $(top_srcdir)/moo/mooscript/mooscript-classes.h \
		> mooscript.texi.tmp && mv mooscript.texi.tmp $(srcdir)/mooscript.texi

help/index.html: $(src_texinfo_files)
	$(AM_V_at)rm -f $(srcdir)/help/*.html
	$(AM_V_at)$(MKDIR_P) $(srcdir)/help
	$(AM_V_GEN)cd $(srcdir) && makeinfo --html medit.texi

help-sections.h.stamp: help/index.html $(srcdir)/gensections.py
	$(AM_V_GEN)$(PYTHON) $(srcdir)/gensections.py $(srcdir)/help/*.html > help-sections.h.tmp && \
		(cmp -s help-sections.h.tmp $(srcdir)/help-sections.h || mv help-sections.h.tmp $(srcdir)/help-sections.h)
	$(AM_V_at)rm -f help-sections.h.tmp
	$(AM_V_at)echo stamp > help-sections.h.stamp

all-am: doc
doc: help/index.html help-sections.h.stamp medit.1

man-medit.t2t: man-medit.t2t.in $(top_builddir)/config.status
	$(AM_V_GEN)cd $(top_builddir) && ./config.status --silent --file=doc/man-medit.t2t
	$(AM_V_at)mv man-medit.t2t $(srcdir)/man-medit.t2t

medit.1: man-medit.t2t
	$(AM_V_GEN)txt2tags --target=man --outfile=- $(srcdir)/man-medit.t2t | grep -v "cmdline: txt2tags" \
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
