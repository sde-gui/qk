BUILT_SOURCES =			\
	mooappabout-glade.h

mooapp_include_headers =	\
	mooapp.h

if MOO_INSTALL_HEADERS
mooapp_includedir = $(moo_includedir)/mooapp
mooapp_include_HEADERS = $(mooapp_include_headers)
endif

mooapp_sources =			\
	$(mooapp_include_headers)	\
	mooappabout-glade.h		\
	mooappabout.c                   \
	mooappabout.h                   \
	mooapp.c                        \
	mooapp.h                        \
	mooapp-private.h                \
	mooappinput.c                   \
	mooappinput.h

EXTRA_DIST =				\
	gpl                             \
	moohtml.c                       \
	moohtml.h			\
	glade/mooappabout.glade

if MOO_USE_XML
mooapp_sources +=                       \
	moohtml.c                       \
	moohtml.h
endif

XML2H = $(srcdir)/../mooutils/xml2h.sh
mooappabout-glade.h: $(srcdir)/glade/mooappabout.glade $(XML2H)
	$(SHELL) $(XML2H) MOO_APP_ABOUT_GLADE_UI $(srcdir)/glade/mooappabout.glade > mooappabout-glade.h

noinst_LTLIBRARIES = libmooapp.la
libmooapp_la_SOURCES = $(mooapp_sources)
AM_CFLAGS =			\
	-I..			\
	-I$(top_builddir)	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)
