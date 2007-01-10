EXTRA_DIST =				\
	moofileselector.glade		\
	moofileselector-prefs.glade

fileselector_sources =			\
	moofileselector-prefs.c		\
	moofileselector.c		\
	moofileselector.h		\
	moofileselector-glade.h		\
	moofileselector-prefs-glade.h

BUILT_SOURCES =				\
	moofileselector-glade.h		\
	moofileselector-prefs-glade.h

XML2H = $(srcdir)/../../../mooutils/xml2h.sh
moofileselector-glade.h: moofileselector.glade $(XML2H)
	$(SHELL) $(XML2H) MOO_FILE_SELECTOR_GLADE_XML $(srcdir)/moofileselector.glade > $@.tmp && \
	mv $@.tmp $@
moofileselector-prefs-glade.h: moofileselector-prefs.glade $(XML2H)
	$(SHELL) $(XML2H) MOO_FILE_SELECTOR_PREFS_GLADE_XML $(srcdir)/moofileselector-prefs.glade > $@.tmp && \
	mv $@.tmp $@

noinst_LTLIBRARIES = libfileselector.la
libfileselector_la_SOURCES = $(fileselector_sources)

AM_CFLAGS =			\
	-I../../..		\
	-I$(top_builddir)	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)
