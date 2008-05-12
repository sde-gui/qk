# -*- makefile -*-

BUILT_SOURCES += marshals.h
CLEANFILES += marshals.h
EXTRA_DIST += marshals.list
marshals.h: marshals.list
	glib-genmarshal --prefix=_moo_marshal --header $(srcdir)/marshals.list > $@.tmp && \
	mv $@.tmp $@
