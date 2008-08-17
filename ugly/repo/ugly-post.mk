# begin ugly-post.mk
# -%- lang: makefile; indent-width: 8; use-tabs: true -%-

$(srcdir)/Makefile.am: $(srcdir)/Makefile.ug $(UGLY_DEPS)
	@echo 'cd $(srcdir) && $(UGLY) $(top_srcdir) $(subdir)'; \
	cd $(srcdir) && $(UGLY) $(top_srcdir) $(subdir) && exit 0; \
	exit 1;

# end ugly-post.mk
