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
	mooappinput.c			\
	mooappinput.h			\
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
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)
