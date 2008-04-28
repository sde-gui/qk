SUBDIRS = smclient

BUILT_SOURCES =			\
	mooappabout-glade.h
CLEANFILES =			\
	mooappabout-glade.h

mooapp_include_headers =	\
	mooapp.h

if MOO_INSTALL_HEADERS
mooapp_includedir = $(moo_includedir)/mooapp
mooapp_include_HEADERS = $(mooapp_include_headers)
endif

mooapp_sources =			\
	$(mooapp_include_headers)	\
	mooappabout.c			\
	mooappabout.h			\
	mooapp.c			\
	mooapp.h			\
	mooapp-private.h		\
	mooapp-accels.h			\
	moohtml.h			\
	moohtml.c			\
	moolinklabel.h			\
	moolinklabel.c

EXTRA_DIST =				\
	glade/mooappabout.glade

@MOO_GLADE_SUBDIR_RULE@

noinst_LTLIBRARIES = libmooapp.la
libmooapp_la_SOURCES = $(mooapp_sources)
nodist_libmooapp_la_SOURCES = mooappabout-glade.h
libmooapp_la_LIBADD = smclient/libsmclient.la

AM_CFLAGS =			\
	-I..			\
	-I$(top_builddir)	\
	-I$(top_srcdir)/doc	\
	-I$(top_builddir)/doc	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)

BUILT_SOURCES += marshals.h
nodist_libmooapp_la_SOURCES += marshals.h
CLEANFILES += marshals.h
EXTRA_DIST += marshals.list
marshals.h: marshals.list
	glib-genmarshal --prefix=_moo_marshal --header	\
	       $(srcdir)/marshals.list > $@.tmp &&	\
	mv $@.tmp $@
