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

pluginsdir = ${MOO_EDIT_PLUGINS_DIR}
plugins_LTLIBRARIES = dummy.la fileselector.la

dummy_la_SOURCES = dummy.c
dummy_la_LDFLAGS = -module -avoid-version -export-symbols-regex dummy_init $(mingw_flags)

fileselector_la_SOURCES = fileselector.c
fileselector_la_LDFLAGS = -module -avoid-version -export-symbols-regex fileselector_init $(mingw_flags)
