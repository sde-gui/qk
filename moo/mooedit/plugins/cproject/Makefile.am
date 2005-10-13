#
#   mooedit/plugins/cproject/Makefile.am
#

DEFAULT_INCLUDES =
AM_CFLAGS =                             \
    -I$(top_builddir)                   \
    -I$(top_builddir)/$(MOO_SRC_PREFIX) \
    $(MOO_CFLAGS)                       \
    $(DEBUG_CFLAGS)                     \
    $(CFLAGS)

pluginsdir = ${MOO_PLUGINS_DIR}
plugins_LTLIBRARIES = cproject.la

cproject_la_SOURCES =   \
    cproject.c          \
    cproject.h          \
    cproject-plugin.c   \
    cproject-project.c  \
    cproject-project.h

if MOO_OS_MINGW
mingw_flags = -no-undefined
endif MOO_OS_MINGW

cproject_la_LDFLAGS = -module -avoid-version -export-symbols-regex cproject_init $(mingw_flags)
