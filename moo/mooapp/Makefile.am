SUBDIRS = smclient

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

noinst_LTLIBRARIES = libmooapp.la
libmooapp_la_SOURCES = $(mooapp_sources)
libmooapp_la_LIBADD = smclient/libsmclient.la

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
