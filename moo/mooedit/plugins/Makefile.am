#
#   mooedit/plugins/Makefile.am
#

DIST_SUBDIRS = cproject
SUBDIRS =

DEFAULT_INCLUDES =
AM_CFLAGS =                             \
    -I$(top_builddir)                   \
    -I$(top_builddir)/$(MOO_SRC_PREFIX) \
    $(MOO_CFLAGS)                       \
    $(DEBUG_CFLAGS)                     \
    $(CFLAGS)

noinst_LTLIBRARIES = libplugins.la

if !MOO_OS_MINGW
unix_sources =              \
    moofind.c               \
    moofind-glade.h         \
    fileselector.c
SUBDIRS += cproject
if MOO_BUILD_TERM
unix_sources += console.c
endif
endif

libplugins_la_SOURCES =     \
    $(unix_sources)         \
    activestrings.c         \
    mooeditplugins.h

BUILT_SOURCES = moofind-glade.h
EXTRA_DIST = moofind.glade moofind-glade.h

moofind-glade.h: moofind.glade
	sh $(srcdir)/../../mooutils/xml2h.sh MOO_FIND_GLADE_XML $(srcdir)/moofind.glade > \
	   moofind-glade.h
