# begin ugly-sub.mk
# -%- lang: makefile; indent-width: 8; use-tabs: true -%-

$(top_srcdir)/m4/ugly-stamp.m4: $(UGLY_STAMP_DEPS)
	cd $(top_builddir) && $(MAKE) $(AM_MAKEFLAGS) m4/ugly-stamp.m4

# end ugly-sub.mk
