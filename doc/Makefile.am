t2t_sources =				\
	license.t2t			\
	medit.t2t			\
	prefs-file.t2t			\
	prefs.t2t			\
	user-tools.t2t

EXTRA_DIST =				\
	$(t2t_sources)			\
	splity/splity.pl		\
	splity/index-template.html	\
	splity/page-template.html	\
	makedocs			\
	stamp-help			\
	help-sections.h

BUILT_SOURCES = stamp-help help-sections.h

help-sections.h: stamp-help
build-docs: stamp-help

stamp-help: $(t2t_sources) makedocs
	rm -fr help && \
	$(mkdir_p) help && \
	$(srcdir)/makedocs $(srcdir) $(srcdir)/medit.t2t && \
	echo stamp > stamp-help

medit.lo: build-docs
prefs.lo: build-docs
prefs-file.lo: build-docs
license.lo: build-docs
user-tools.lo: build-docs

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
	  cp -pR help/ $(distdir)/ ; \
	else \
	  cp -pR $(srcdir)/help/ $(distdir)/ ; \
	fi

distclean-hook:
	rm -fr help stamp-help
