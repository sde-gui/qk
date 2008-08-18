# begin ugly-post.mk
# -%- lang: makefile; indent-width: 8; use-tabs: true -%-

$(srcdir)/Makefile.am: $(srcdir)/Makefile.ug $(UGLY_DEPS)
	@top_srcdir=`cd $(top_srcdir) && pwd`; \
	echo 'cd $(srcdir) && $$top_srcdir/ugly/ugly $$top_srcdir $(subdir)'; \
	cd $(srcdir) && $$top_srcdir/ugly/ugly $$top_srcdir $(subdir) && exit 0; \
	exit 1;

# end ugly-post.mk
