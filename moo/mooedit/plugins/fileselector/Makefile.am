noinst_LTLIBRARIES = libfileselector.la

EXTRA_DIST =				\
	moofileselector.glade		\
	moofileselector-prefs.glade

libfileselector_la_SOURCES =		\
	moofileselector-prefs.c		\
	moofileselector.c		\
	moofileselector.h

nodist_libfileselector_la_SOURCES =	\
	moofileselector-glade.h		\
	moofileselector-prefs-glade.h

BUILT_SOURCES =				\
	moofileselector-glade.h		\
	moofileselector-prefs-glade.h
CLEANFILES =				\
	moofileselector-glade.h		\
	moofileselector-prefs-glade.h

AM_CFLAGS =			\
	-I../../..		\
	-I$(top_builddir)	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)

@MOO_GLADE_RULE@
