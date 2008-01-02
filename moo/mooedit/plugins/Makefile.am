DIST_SUBDIRS = ctags
SUBDIRS =

if MOO_BUILD_CTAGS
SUBDIRS += ctags
endif

noinst_LTLIBRARIES =
EXTRA_DIST =
BUILT_SOURCES =
CLEANFILES =

@MOO_GLADE_RULE@

AM_CFLAGS =			\
	-I../..			\
	-I$(top_builddir)	\
	-I$(top_srcdir)/doc	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)
AM_OBJCFLAGS = $(AM_CFLAGS) $(MOO_OBJCFLAGS)

###########################################################################
# fileselector
#
noinst_LTLIBRARIES += libfileselector.la

EXTRA_DIST +=				\
	moofileselector.glade		\
	moofileselector-prefs.glade

libfileselector_la_SOURCES =		\
	moofileselector-prefs.c		\
	moofileselector.c		\
	moofileselector.h

nodist_libfileselector_la_SOURCES =	\
	moofileselector-glade.h		\
	moofileselector-prefs-glade.h

BUILT_SOURCES +=			\
	moofileselector-glade.h		\
	moofileselector-prefs-glade.h
CLEANFILES +=				\
	moofileselector-glade.h		\
	moofileselector-prefs-glade.h


###########################################################################
# libmooeditplugins
#
noinst_LTLIBRARIES += libmooeditplugins.la
libmooeditplugins_la_LIBADD = libfileselector.la

if MOO_BUILD_CTAGS
libmooeditplugins_la_LIBADD += ctags/libctags.la
endif

libmooeditplugins_la_SOURCES =	\
	mooeditplugins.h	\
	moofilelist.c

EXTRA_DIST +=			\
	moofind.glade		\
	moofind.c

BUILT_SOURCES += moofind-glade.h
CLEANFILES += moofind-glade.h

if MOO_OS_UNIX
libmooeditplugins_la_SOURCES += moofind.c
nodist_libmooeditplugins_la_SOURCES = moofind-glade.h
endif
