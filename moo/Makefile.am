EXTRA_DIST = moo-config.h

EXTRA_DIST += marshals.list
BUILT_SOURCES = marshals.h
CLEANFILES = marshals.h
marshals.h: marshals.list
	$(GLIB_GENMARSHAL) --prefix=_moo_marshal --header $(srcdir)/marshals.list > marshals.h.tmp \
		&& mv marshals.h.tmp marshals.h

# noinst_LTLIBRARIES = libmoo.la
#
# AM_CPPFLAGS = $(MOO_CPPFLAGS)
# AM_CFLAGS = $(MOO_CFLAGS)
# AM_CXXFLAGS = $(MOO_CXXFLAGS)
#
# libmoo_la_LIBADD =				\
# 	mooapp/libmooapp.la			\
# 	eggsmclient/libeggsmclient.la		\
# 	mooedit/libmooedit.la			\
# 	gtksourceview/libgtksourceview.la	\
# 	moofileview/libmoofileview.la		\
# 	moolua/libmoolua.la			\
# 	mooutils/libmooutils.la			\
# 	xdgmime/libxdgmime.la
#
# libmoo_la_SOURCES =		\
# 	marshals.c		\
# 	marshals.h		\
# 	moo-config.h
#
# EXTRA_DIST = marshals.list
#
# BUILT_SOURCES =			\
# 	marshals.c		\
# 	marshals.h
#
# marshals.c: marshals.list
# 	echo '#include "marshals.h"' > marshals.c.tmp
# 	$(GLIB_GENMARSHAL) --prefix=_moo_marshal --body $(srcdir)/marshals.list >> marshals.c.tmp
# 	cmp -s marshals.c.tmp marshals.c || mv marshals.c.tmp marshals.c
# 	rm -f marshals.c.tmp
#
# marshals.h: marshals.list
# 	$(GLIB_GENMARSHAL) --prefix=_moo_marshal --header $(srcdir)/marshals.list > marshals.h.tmp
# 	cmp -s marshals.h.tmp marshals.h || mv marshals.h.tmp marshals.h
# 	rm -f marshals.h.tmp
#
# CLEANFILES =			\
# 	marshals.c		\
# 	marshals.h

SUBDIRS =			\
	xdgmime			\
	mooutils		\
	moolua			\
	moofileview		\
	gtksourceview		\
	mooedit			\
	eggsmclient		\
	mooapp			\
	plugins			\
	medit
