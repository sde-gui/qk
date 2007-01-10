astringsdir = $(MOO_DATA_DIR)
astrings_DATA =			\
	as.cfg

EXTRA_DIST =			\
	as-plugin.glade		\
	$(astrings_DATA)

astrings_sources =		\
	as-plugin.c		\
	as-plugin.h             \
	as-plugin-prefs.c       \
	as-plugin-script.c      \
	as-plugin-script.h      \
	as-plugin-glade.h

BUILT_SOURCES =			\
	as-plugin-glade.h

XML2H = $(srcdir)/../../../mooutils/xml2h.sh
as-plugin-glade.h: as-plugin.glade $(XML2H)
	$(SHELL) $(XML2H) AS_PLUGIN_GLADE_UI $(srcdir)/as-plugin.glade > $@.tmp && \
	mv $@.tmp $@

noinst_LTLIBRARIES = libastrings.la
libastrings_la_SOURCES = $(astrings_sources)

AM_CFLAGS =			\
	-I../../..		\
	-I$(top_builddir)	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)
