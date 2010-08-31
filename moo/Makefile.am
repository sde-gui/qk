noinst_LTLIBRARIES = libmoo.la

AM_CPPFLAGS = $(MOO_CPPFLAGS)
AM_CFLAGS = $(MOO_CFLAGS)
AM_CXXFLAGS = $(MOO_CXXFLAGS)

libmoo_la_LIBADD =				\
	mooapp/libmooapp.la			\
	eggsmclient/libeggsmclient.la		\
	mooedit/libmooedit.la			\
	gtksourceview/libgtksourceview.la	\
	moofileview/libmoofileview.la		\
	moolua/libmoolua.la			\
	mooutils/libmooutils.la			\
	xdgmime/libxdgmime.la

libmoo_la_SOURCES =		\
	marshals.c		\
	marshals.h		\
	moo-config.h

EXTRA_DIST = marshals.list

BUILT_SOURCES =			\
	marshals.stamp		\
	marshals.c		\
	marshals.h

marshals.stamp: marshals.list
	echo '#include "marshals.h"' > marshals.c.tmp
	$(GLIB_GENMARSHAL) --prefix=_moo_marshal --body $(srcdir)/marshals.list >> marshals.c.tmp
	cmp -s marshals.c.tmp marshals.c || mv marshals.c.tmp marshals.c
	$(GLIB_GENMARSHAL) --prefix=_moo_marshal --header $(srcdir)/marshals.list > marshals.h.tmp
	cmp -s marshals.h.tmp marshals.h || mv marshals.h.tmp marshals.h
	rm -f marshals.c.tmp marshals.h.tmp
	echo stamp > marshals.stamp

SUBDIRS =			\
	xdgmime			\
	mooutils		\
	moolua			\
	moofileview		\
	gtksourceview		\
	mooedit			\
	eggsmclient		\
	mooapp
