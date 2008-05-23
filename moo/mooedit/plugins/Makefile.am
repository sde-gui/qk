DIST_SUBDIRS = ctags
SUBDIRS =

if MOO_BUILD_CTAGS
SUBDIRS += ctags
endif

noinst_LTLIBRARIES =
EXTRA_DIST =
BUILT_SOURCES =
CLEANFILES =

AM_CFLAGS =			\
	-I..			\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)
AM_OBJCFLAGS = $(AM_CFLAGS)

###########################################################################
# fileselector
#
noinst_LTLIBRARIES += libfileselector.la

glade_files =				\
	moofileselector.glade		\
	moofileselector-prefs.glade

libfileselector_la_SOURCES =		\
	moofileselector-prefs.c		\
	moofileselector.c		\
	moofileselector.h


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
	moofilelist.c		\
	moofind.c

glade_files += moofind.glade

EXTRA_DIST +=			\
	moofind.c

include ../../../mk/glade.mk
