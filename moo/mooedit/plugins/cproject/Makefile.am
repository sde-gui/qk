#
#   mooedit/plugins/cproject/Makefile.am
#

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

pluginsdir = ${MOO_EDIT_PLUGINS_DIR}
plugins_LTLIBRARIES = cproject.la

cproject_la_SOURCES = cproject.c cproject.h

if MINGW_BUILD
mingw_flags = -no-undefined
endif MINGW_BUILD

cproject_la_LDFLAGS = -module -avoid-version -export-symbols-regex cproject_init $(mingw_flags)
