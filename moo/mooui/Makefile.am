SUBDIRS = smclient

mooui_sources =				\
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

noinst_LTLIBRARIES = libmooui.la
libmooui_la_SOURCES = $(mooui_sources)
libmooui_la_LIBADD = smclient/libsmclient.la

BUILT_SOURCES =
CLEANFILES =
EXTRA_DIST =

glade_files = glade/mooappabout.glade
include ../../mk/glade.mk

AM_CFLAGS =			\
	-I..			\
	-I$(top_builddir)	\
	-I$(top_srcdir)/doc	\
	-I$(top_builddir)/doc	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)

include ../../mk/marshals.mk
