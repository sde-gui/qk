if MOO_DEV_MODE

MOO_HELP_T2T_FILES =	\
	license.t2t	\
	prefs-file.t2t	\
	prefs.t2t	\
	user-tools.t2t	\
	regex.t2t	\
	medit.t2t

man-medit.t2t: man-medit.t2t.in $(top_builddir)/config.status
	cd $(top_builddir) && ./config.status --file=doc/man-medit.t2t
	mv man-medit.t2t $(srcdir)/man-medit.t2t

medit.1: man-medit.t2t
	cd $(srcdir) && $(PYTHON) txt2tags/txt2tags.py --target=man --outfile=medit.1.tmp man-medit.t2t && \
		mv medit.1.tmp medit.1

medit.t2t: medit.t2t.in $(top_builddir)/config.status
	cd $(top_builddir) && ./config.status --file=doc/medit.t2t
	mv medit.t2t $(srcdir)/medit.t2t

help.html: $(MOO_HELP_T2T_FILES)
	cd $(srcdir) && $(PYTHON) txt2tags/txt2tags.py --outfile=help.html.tmp medit.t2t && \
		mv help.html.tmp help.html

help/index.html: help.html
	$(MKDIR_P) $(srcdir)/help
	cd $(srcdir)/help && $(PERL) -I.. ../splity/splity.pl -index ../splity/index-template.html -page ../splity/page-template.html ../help.html

help-sections.h: help.html genhelpsections.py
	cd $(srcdir) && $(PYTHON) genhelpsections.py help.html > help-sections.h.tmp && mv help-sections.h.tmp help-sections.h
	rm -f $(srcdir)/help-sections.h.tmp

all-am: doc
doc: medit.1 help/index.html help-sections.h

endif

EXTRA_DIST = medit.1 help.html help-sections.h help

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(MOO_HELP_DIR)
	cd $(srcdir) && $(INSTALL_DATA) help/*.html $(DESTDIR)$(MOO_HELP_DIR)

uninstall-local:
	rm -f $(DESTDIR)$(MOO_HELP_DIR)/*.html

if MOO_OS_UNIX
dist_man_MANS = medit.1
endif MOO_OS_UNIX
