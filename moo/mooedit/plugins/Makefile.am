DIST_SUBDIRS = fileselector

# subdirs = activestrings completion
# libs = activestrings/libastrings.la completion/libcompletion.la
subdirs =
libs =
if GTK_2_6
subdirs += fileselector
libs += fileselector/libfileselector.la
endif
SUBDIRS = $(subdirs) .

noinst_LTLIBRARIES = libmooeditplugins.la
libmooeditplugins_la_LIBADD = $(libs)

libmooeditplugins_la_SOURCES =	\
	mooeditplugins.h

EXTRA_DIST =			\
	moofind.glade		\
	moofind.c

BUILT_SOURCES = moofind-glade.h
CLEANFILES = moofind-glade.h
@MOO_GLADE_RULE@

if MOO_OS_UNIX
libmooeditplugins_la_SOURCES += moofind.c
nodist_libmooeditplugins_la_SOURCES = moofind-glade.h
endif

AM_CFLAGS =			\
	-I../..			\
	-I$(top_builddir)	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)
