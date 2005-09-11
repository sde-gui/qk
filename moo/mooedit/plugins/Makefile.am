#
#   mooedit/plugins/Makefile.am
#

SUBDIRS = cproject

DEFAULT_INCLUDES =
AM_CFLAGS =                             \
    -DXDG_PREFIX=_moo_edit_xdg          \
    -I$(top_builddir)                   \
    -I$(top_builddir)/$(MOO_SRC_PREFIX) \
    $(MOO_CFLAGS)                       \
    $(DEBUG_CFLAGS)                     \
    $(CFLAGS)                           \
    -DG_LOG_DOMAIN=\"Moo\"              \
    -D__MOO__                           \
    -DMOO_MARSHALS_H=\"mooutils/moomarshals.h\"

if MINGW_BUILD
mingw_flags = -no-undefined
endif MINGW_BUILD

pluginsdir = ${MOO_PLUGINS_DIR}
plugins_LTLIBRARIES = dummy.la

dummy_la_SOURCES = dummy.c
dummy_la_LDFLAGS = -module -avoid-version -export-symbols-regex dummy_init $(mingw_flags)

noinst_LTLIBRARIES = libplugins.la

libplugins_la_SOURCES = \
    mooeditplugins.h    \
    fileselector.c      \
    moofind.c           \
    moofind-glade.h

BUILT_SOURCES = moofind-glade.h
EXTRA_DIST = moofind.glade

moofind-glade.h: moofind.glade
	sh $(srcdir)/../../mooutils/xml2h.sh MOO_FIND_GLADE_XML $(srcdir)/moofind.glade > \
	   moofind-glade.h
