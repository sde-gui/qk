t2t_sources =				\
	license.t2t			\
	medit.t2t			\
	prefs-file.t2t			\
	prefs.t2t			\
	user-tools.t2t			\
	man-medit.t2t

EXTRA_DIST =				\
	$(t2t_sources)			\
	splity/splity.pl		\
	splity/index-template.html	\
	splity/page-template.html	\
	makedocs			\
	stamp-help			\
	help-sections.h

dist_man_MANS = medit.1

BUILT_SOURCES = stamp-help medit.1

help-sections.h: stamp-help
build-docs: stamp-help

medit.1: man-medit.t2t
	$(TXT2TAGS) --target=man --outfile=$(srcdir)/medit.1 $(srcdir)/man-medit.t2t

stamp_help_deps = $(t2t_sources) makedocs
stamp-help: $(stamp_help_deps)
	cd $(srcdir) && \
	rm -fr help && \
	$(mkdir_p) help && \
	TXT2TAGS=$(TXT2TAGS) $(srcdir)/makedocs $(srcdir)/medit.t2t \
		$(srcdir)/stamp-help $(stamp_help_deps)

medit.lo: build-docs
prefs.lo: build-docs
prefs-file.lo: build-docs
license.lo: build-docs
user-tools.lo: build-docs
man-medit.lo: medit.1

install-data-hook: build-docs
	$(mkdir_p) $(DESTDIR)$(htmldir)/help
	if test -f stamp-help; then \
	  $(INSTALL_DATA) help/* $(DESTDIR)$(htmldir)/help/ ; \
	else \
	  $(INSTALL_DATA) $(srcdir)/help/* $(DESTDIR)$(htmldir)/help/ ; \
	fi
uninstall-hook:
	rm -fr $(DESTDIR)$(htmldir)/help

dist-hook:
	if test -f stamp-help; then \
	  cp -pR help $(distdir)/ ; \
	else \
	  cp -pR $(srcdir)/help $(distdir)/ ; \
	fi
